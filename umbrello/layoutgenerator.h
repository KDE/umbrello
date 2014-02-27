/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef LAYOUTGENERATOR_H

#include "dotgenerator.h"
#include "optionstate.h"

//// qt includes
#include <QRectF>

/**
 * The class LayoutGenerator provides calculated layouts of diagrams.
 *
 * It uses the dot executable from the graphviz package for calculation
 * of widget positions.
 *
 * The implementation calls dot with information from the displayed
 * widgets and associations by creating a temporary dot file based on a
 * layout configure file, which is located in the umbrello/layouts subdir of
 * the "data" resource type. The config file is determined from the
 * type of the currently displayed diagram and the layout chosen by the user.
 *
 * Dot creates a file containing the calculated widget positions.
 * The widget positions are retrieved from this file and used to move
 * widgets on the provided diagram. Additional points in association lines
 * are removed.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class LayoutGenerator : public DotGenerator
{
public:
    typedef QHash<QString,QRectF> NodeType;
    typedef QList<QPointF> EdgePoints;
    typedef QHash<QString,EdgePoints> EdgeType;

    /**
     * constructor
    */
    LayoutGenerator()
    {
        setUseFullNodeLabels(false);
    }

    /**
     * Return state if layout generator is enabled.
     * It is enabled when the dot application has been found.
     *
     * @return true if enabled
    */
    bool isEnabled()
    {
        Settings::OptionState& optionState = Settings::optionState();
        if (optionState.autoLayoutState.autoDotPath) {
            m_dotPath = currentDotPath();
        }
        else if (!optionState.autoLayoutState.dotPath.isEmpty()) {
            m_dotPath = optionState.autoLayoutState.dotPath;
        }
        return !m_dotPath.isEmpty();
    }

    /**
     * Return the path where dot is installed.
     *
     * @return string with dot path
     */
    static QString currentDotPath()
    {
        QString executable = KStandardDirs::findExe("dot");
        if (!executable.isEmpty()) {
            QFileInfo fi(executable);
            return fi.absolutePath();
        }
#ifdef Q_OS_WIN
        // search for dot installation
        QString appDir(qgetenv("ProgramFiles"));
        QDir dir(appDir);
        dir.setFilter(QDir::Dirs);
        dir.setNameFilters(QStringList() << "Graphviz*");
        dir.setSorting(QDir::Reversed);
        QFileInfoList list = dir.entryInfoList();
        if (list.size() > 0) {
            QString dotPath = list.at(0).absoluteFilePath();
            QString exePath = QFile::exists(dotPath + "\\bin") ? dotPath + "\\bin" : dotPath;
            return QFile::exists(exePath + "\\dot.exe") ? exePath : "";
        }
#endif
        return QString();
    }

    /**
     * generate layout and apply it to the given diagram.
     *
     * @return true if generating succeeded
    */
    bool generate(UMLScene *scene, const QString &variant = QString())
    {
        QTemporaryFile in;
        QTemporaryFile out;
        QTemporaryFile xdotOut;
        if (!isEnabled()) {
            uWarning() << "Could not apply autolayout because graphviz installation has not been found.";
            return false;
        }

#ifdef LAYOUTGENERATOR_DEBUG
        in.setAutoRemove(false);
        out.setAutoRemove(false);
        xdotOut.setAutoRemove(false);
#endif

        // generate filenames
        in.open();
        in.close();
        out.open();
        out.close();
        xdotOut.open();
        xdotOut.close();

#ifdef LAYOUTGENERATOR_DEBUG
        qDebug() << textViewer() << in.fileName();
        qDebug() << textViewer() << out.fileName();
        qDebug() << textViewer() << xdotOut.fileName();
#endif

        if (!createDotFile(scene, in.fileName(), variant))
            return false;

        QString executable = m_dotPath + "/" + m_generator;

        QProcess p;
        QStringList args;
        args << "-o" << out.fileName() << "-Tplain-ext" << in.fileName();
        p.start(executable, args);
        p.waitForFinished();

        args.clear();
        args << "-o" << xdotOut.fileName() << "-Txdot" << in.fileName();
        p.start(executable, args);
        p.waitForFinished();

#ifdef LAYOUTGENERATOR_DEBUG
        QTemporaryFile pngFile;
        pngFile.setAutoRemove(false);
        pngFile.setFileTemplate(QDir::tempPath() + "/umbrello-layoutgenerator-XXXXXX.png");
        pngFile.open();
        pngFile.close();
        qDebug() << pngViewer() << pngFile.fileName();
        args.clear();
        args << "-o" << pngFile.fileName() << "-Tpng" << in.fileName();
        p.start(executable, args);
        p.waitForFinished();
#endif
#ifndef USE_XDOT
        if (!readGeneratedDotFile(out.fileName()))
#else
        if (!readGeneratedDotFile(xdotOut.fileName()))
#endif
            return false;

        return true;
    }

    /**
     * apply auto layout to the given scene
     * @param scene
     * @return true if autolayout has been applied
     */
    bool apply(UMLScene *scene)
    {
        foreach(AssociationWidget *assoc, scene->associationList()) {
            AssociationLine *path = assoc->associationLine();
            QString type = Uml::AssociationType::toString(assoc->associationType()).toLower();
            QString key = "type::" + type;

            QString id;
            if (m_edgeParameters.contains("id::" + key) && m_edgeParameters["id::" + key] == "swap")
                id = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::A)) + Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::B)));
            else
                id = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::B)) + Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::A)));

            // adjust associations not used in the dot file
            if (!m_edges.contains(id)) {
                // shorten line path
                AssociationLine *path = assoc->associationLine();
                if (path->count() > 2 && assoc->widgetLocalIDForRole(Uml::RoleType::A) != assoc->widgetLocalIDForRole(Uml::RoleType::B)) {
                    while(path->count() > 2)
                        path->removePoint(1);
                }
                continue;
            }

            EdgePoints &p = m_edges[id];
            int len = p.size();

            while(path->count() > 1) {
                path->removePoint(0);
            }
            path->setEndPoints(mapToScene(p[0]), mapToScene(p[len-1]));

            // set label position
            QPointF &l = m_edgeLabelPosition[id];
            FloatingTextWidget *tw = assoc->nameWidget();
            if (tw) {
                tw->setPos(mapToScene(l));
            }
            // FIXME: set remaining association line points
            /*
            for(int i = 1; i < len-1; i++) {
                path->insertPoint(i, mapToScene((p[i]));
            }
            */
            /*
             * here stuff could be added to add more points from information returned by dot.
            */
        }

        foreach(UMLWidget *widget, scene->widgetList()) {
            QString id = Uml::ID::toString(widget->localID());
            if (!m_nodes.contains(id))
                continue;
            QPoint p = origin(id);
            widget->setX(p.x());
            widget->setY(p.y()-widget->height());
            widget->adjustAssocs(widget->x(), widget->y());    // adjust assoc lines
        }

        foreach(AssociationWidget *assoc, scene->associationList()) {
            assoc->calculateEndingPoints();
            if (assoc->associationLine())
                assoc->associationLine()->update();
            assoc->resetTextPositions();
        }
        return true;
    }

    /**
     * Return a list of available templates for a given scene type
     *
     * @param scene The diagram
     * @param configFiles will contain the collected list of config files
     * @return true if collecting succeeds
     */
    static bool availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles)
    {
        QString diagramType = Uml::DiagramType::toString(scene->type()).toLower();
        KStandardDirs dirs;

        QStringList fileNames = dirs.findAllResources("data", QString("umbrello/layouts/%1*.desktop").arg(diagramType));
        foreach(const QString &fileName, fileNames) {
            QFileInfo fi(fileName);
            QString baseName;
            if (fi.baseName().contains("-"))
                baseName = fi.baseName().remove(diagramType + "-");
            else if (fi.baseName() == diagramType)
                baseName = fi.baseName();
            else
                baseName = "default";
            KDesktopFile desktopFile(fileName);
            configFiles[baseName] = desktopFile.readName();
        }
        return true;
    }

protected:
    /**
     * Return the origin of node based on the bottom/left corner
     *
     * @param id The widget id to fetch the origin from
     * @return QPoint instance with the coordinates
     */
    QPoint origin(const QString &id)
    {
        QString key = fixID(id);
        if (!m_nodes.contains(key)) {
#ifdef LAYOUTGENERATOR_DATA_DEBUG
            uDebug() << key;
#endif
            return QPoint(0,0);
        }
        QRectF &r = m_nodes[key];
        QPoint p(m_origin.x() + r.x() - r.width()/2, m_boundingRect.height() - r.y() + r.height()/2 + m_origin.y());
#ifdef LAYOUTGENERATOR_DATA_DEBUG
        uDebug() << r << p;
#endif
        return p;
    }

    /**
     * Read generated dot file and extract positions
     * of the contained widgets.
     *
     * @return true if extracting succeeded
    */
    bool readGeneratedDotFile(const QString &fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            while(line.endsWith(','))
                line += in.readLine();
            parseLine(line);
        }
        return true;
    }

#ifndef USE_XDOT
    /**
     * Parse line from dot generated plain-ext output format
     *
     *  The format is documented at http://graphviz.org/content/output-formats#dplain-ext and looks like:
     *
     *   graph 1 28.083 10.222
     *   node ITfDmJvJE00m 8.0833 8.7361 0.86111 0.45833 QObject solid box black lightgrey
     *   edge sL4cKPpHnJkU sL4cKPpHnJkU 7 8.1253 7.2568 8.2695 7.2687 8.375 7.3127 8.375 7.3889 8.375 7.4377 8.3317 7.4733 8.2627 7.4957 Aggregation 8.8472 7.3889 solid black
     *
     * @param line line in dot plain-ext output format
     * @return true if line could be parsed successfully
    */
    bool parseLine(const QString &line)
    {
        QStringList a = line.split(' ');
        if (a[0] == "graph") {
            m_boundingRect = QRectF(0, 0, a[2].toDouble()*m_scale, a[3].toDouble()*m_scale);
            return true;
        } else if (a[0] == "node") {
            QString key = fixID(a[1]);
            m_nodes[key] = QRectF(a[2].toDouble()*m_scale, a[3].toDouble()*m_scale, a[4].toDouble()*m_scale, a[5].toDouble()*m_scale);
            return true;
        } else if (a[0] == "edge") {
            QString key = fixID(a[1]+a[2]);
            EdgePoints p;
            int len = a[3].toInt();
            for(int i = 0; i < len; i++)
                p.append(QPointF(a[i*2+4].toDouble()*m_scale, a[i*2+5].toDouble()*m_scale));
            m_edges[key] = p;

            int b = len*2 + 4;
            bool ok;
            double x = a[b+1].toDouble(&ok);
            if (!ok)
                return true;
            double y = a[b+2].toDouble(&ok);
            if (!ok)
                return true;
            m_edgeLabelPosition[key] = QPointF(x*m_scale, y*m_scale);

            return true;
        } else if (a[0] == "stop") {
            return true;
        }
        return false;
    }

    typedef QMap<QString,QStringList> ParameterList;

    LayoutGenerator();

    bool isEnabled();
    static QString currentDotPath();
    bool generate(UMLScene *scene, const QString &variant = QString());
    bool apply(UMLScene *scene);
    static bool availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles);
    QPoint origin(const QString &id);
    bool readGeneratedDotFile(const QString &fileName);
    bool parseLine(const QString &line);
    bool splitParameters(QMap<QString,QStringList> &map, const QString &s);
    QPointF mapToScene(const QPointF &p);

protected:
    QRectF m_boundingRect;
    NodeType m_nodes;      ///< list of nodes found in parsed dot file
    EdgeType m_edges;      ///< list of edges found in parsed dot file
    QHash<QString, QPointF> m_edgeLabelPosition; ///< contains global node parameters
    QString m_dotPath;     ///< contains path to dot executable

    friend QDebug operator<<(QDebug out, LayoutGenerator &c);
};

#if 0
static QDebug operator<<(QDebug out, LayoutGenerator &c);
#endif

#endif
