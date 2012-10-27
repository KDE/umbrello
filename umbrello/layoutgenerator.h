/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LAYOUTGENERATOR_H

#include "associationwidget.h"
#include "debug_utils.h"
#include "dotgenerator.h"
#include "floatingtextwidget.h"
#include "optionstate.h"
#include "umlwidget.h"

// app includes
#include <KConfigGroup>
#include <KDesktopFile>
#include <KStandardDirs>

// qt includes
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QRectF>
#include <QRegExp>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>

#define LAYOUTGENERATOR_DEBUG
//#define LAYOUTGENERATOR_DATA_DEBUG

#ifdef LAYOUTGENERATOR_DEBUG

static QString pngViewer()
{
#ifdef Q_WS_WIN
    return "start";
#else
#ifdef Q_WS_MAC
    return "unknown";
#else
    return "okular";
#endif
#endif
}

static QString textViewer()
{
#ifdef Q_WS_WIN
    return "start";
#else
#ifdef Q_WS_MAC
    return "unknown";
#else
    return "mcedit";
#endif
#endif
}
#endif

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
            return list.at(0).absoluteFilePath();
        }
#endif
        return QString();
    }

    /**
     * generate layout and apply it to the given diagram.
     *
     * @return true if generating succeded
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
            QString type = assoc->associationType().toString().toLower();
            QString key = "type::" + type;

            QString id;
            if (m_edgeParameters.contains("id::" + key) && m_edgeParameters["id::" + key] == "swap")
                id = fixID(ID2STR(assoc->widgetIDForRole(Uml::A)) + ID2STR(assoc->widgetIDForRole(Uml::B)));
            else
                id = fixID(ID2STR(assoc->widgetIDForRole(Uml::B)) + ID2STR(assoc->widgetIDForRole(Uml::A)));

            // adjust associations not used in the dot file
            if (!m_edges.contains(id)) {
                // shorten line path
                AssociationLine *path = assoc->associationLine();
                if (path->count() > 2 && assoc->widgetIDForRole(Uml::A) != assoc->widgetIDForRole(Uml::B)) {
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
            path->setEndPoints(QPoint(p[0].x() + m_origin.x(), m_boundingRect.height() - p[0].y() + m_origin.y()), QPoint(p[len-1].x() + m_origin.x(), m_boundingRect.height() - p[len-1].y() + m_origin.y()));

            // set label position
            QPointF &l = m_edgeLabelPosition[id];
            FloatingTextWidget *tw = assoc->nameWidget();
            if (tw) {
                tw->setX((int)(l.x() + m_origin.x()));
                tw->setY(int(m_boundingRect.height() - l.y() + m_origin.y()));
            }
            // FIXME: set remaining association line points
            /*
            for(int i = 1; i < len-1; i++) {
                path->insertPoint(i, QPoint(p[i].x()+ m_origin.x(), m_boundingRect.height() - p[i].y() + m_origin.y()));
            }
            */
            /*
             * here stuff could be added to add more points from informations returned by dot.
            */
        }

        foreach(UMLWidget *widget, scene->widgetList()) {
            QString id = ID2STR(widget->id());
            if (!m_nodes.contains(id))
                continue;
            QPoint p = origin(id);
            widget->setX(p.x());
            widget->setY(p.y()-widget->height());
//:TODO:            widget->adjustAssociations(widget->x(), widget->y());    // adjust assoc lines
        }

        foreach(AssociationWidget *assoc, scene->associationList()) {
            assoc->resetTextPositions();
//:TODO:            assoc->calculateEndingPoints();
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
        QString diagramType = scene->type().toString().toLower();
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

#else
    typedef QMap<QString,QStringList> ParameterList;

    bool splitParameters(QMap<QString,QStringList> &map, const QString &s)
    {
        // FIXME: add shape=box without '"'
        static QRegExp rx("([a-zA-Z_]+)=\"([a-zA-Z0-9.- #]+)\"");
        static QRegExp rx2("([a-zA-Z_]+)=([a-zA-Z0-9.- #]+)");
        int pos = 0;
        int count = 0;
        /*
        *        while ((pos = rx2.indexIn(s, pos)) != -1) {
        *            QString key = rx2.cap(1);
        *            QString value = rx2.cap(2);
        *            ++count;
        *            pos += rx2.matchedLength();
        *            //qDebug() << key << value;
        *            if (map.contains(key))
        *                map[key] << value;
        *            else
        *                map[key] = QStringList() << value;
        }
        */
        pos = 0;
        while ((pos = rx.indexIn(s, pos)) != -1) {
            QString key = rx.cap(1);
            QString value = rx.cap(2);
            ++count;
            pos += rx.matchedLength();
            //qDebug() << key << value;

            QStringList data;
            if (key == "pos") {
                value.remove("e,");
                data = value.split(' ');
        } else if (key.startsWith('_')) {
            data = value.split(' ');
        }
        else if (key == "label")
            data = QStringList() << value;
        else
            data = value.split(',');

        if (map.contains(key))
            map[key] << data;
        else
            map[key] = data;
        }
        return true;
    }

/**
     *
    digraph G {
        graph [splines=polyline, rankdir=BT, outputorder=nodesfirst, ranksep="0.5", nodesep="0.5"];
        node [label="\N"];
        graph [bb="0,0,2893,638",
        _draw_="c 9 -#ffffffff C 9 -#ffffffff P 4 0 -1 0 638 2894 638 2894 -1 ",
        xdotversion="1.2"];
        XC0weWhArzOJ [label=note, shape=box, width="2.5833", height="0.86111", pos="93,31", _draw_="c 9 -#000000ff p 4 186 62 0 62 0 0 186 0 ", _ldraw_="F 14.000000 11 -Times-Roman c 9 -#000000ff T 93 27 0 24 4 -note "];
        sL4cKPpHnJkU -> ITfDmJvJE00m [arrowhead=normal, weight="1.0", label=" ", pos="e,2326.3,600.47 2299.7,543.57 2306.1,557.22 2314.9,575.99 2322.1,591.39", lp="2319,572", _draw_="c 9 -#000000ff B 4 2300 544 2306 557 2315 576 2322 591 ", _hdraw_="S 5 -solid c 9 -#000000ff C 9 -#000000ff P 3 2319 593 2326 600 2325 590 ", _ldraw_="F 14.000000 11 -Times-Roman c 9 -#000000ff T 2319 568 0 4 1 -  "];
        sL4cKPpHnJkU -> sL4cKPpHnJkU [label=" ", arrowtail=odiamond, dir=back, constraint=false, pos="s,2339.3,516.43 2351.5,516.59 2365.1,517.35 2375,520.16 2375,525 2375,531.2 2358.7,534.06 2339.3,533.57", lp="2377,525", _draw_="c 9 -#000000ff B 7 2351 517 2365 517 2375 520 2375 525 2375 531 2359 534 2339 534 ", _tdraw_="S 5 -solid c 9 -#000000ff p 4 2351 517 2345 521 2339 516 2345 513 ", _ldraw_="F 14.000000 11 -Times-Roman c 9 -#000000ff T 2377 521 0 4 1 -  "];
    */

    bool parseLine(const QString &line)
    {
        static QRegExp m_cols("^[\t ]*(.*)[\t ]*\\[(.*)\\]");
        static int m_level = -1;

        if (line.contains('{')) {
            m_level++;
            return true;
        }
        else if (line.contains('}')) {
            m_level--;
            return true;
        }
        int pos = 0;
        if (m_cols.indexIn(line, pos) == -1)
            return false;

        QString keyword = m_cols.cap(1).trimmed();
        QString attributeString = m_cols.cap(2);
        uDebug() << keyword << attributeString;
        ParameterList attributes;
        splitParameters(attributes, attributeString);
        uDebug() << attributes;

        if (keyword == "graph") {
            if (attributes.contains("bb")) {
                QStringList &a = attributes["bb"];
                m_boundingRect.setLeft(a[0].toDouble());
                m_boundingRect.setTop(a[1].toDouble());
                m_boundingRect.setRight(a[2].toDouble());
                m_boundingRect.setBottom(a[3].toDouble());
            }
        } else if (keyword == "node") {
            return true;
        } else if (keyword == "edge") {
            return true;
        // transistion
        } else if (line.contains("->")) {
            QStringList k = keyword.split(" ");
            if (k.size() < 3)
                return false;
            QString key = fixID(k[0]+k[2]);

            if (attributes.contains("pos")) {
                QStringList &a = attributes["pos"];
                EdgePoints points;

                for(int i = 1; i < a.size(); i++) {
                    QStringList b = a[i].split(',');
                    QPointF p(b[0].toDouble(), b[1].toDouble());
                    points.append(p);
                }
                QStringList b = a[0].split(',');
                QPointF p(b[0].toDouble(), b[1].toDouble());
                points.append(p);

                m_edges[key] = points;
            }
            if (0 && attributes.contains("_draw_")) {
                QStringList &a = attributes["_draw_"];
                if (a.size() < 5 || (a[3] != "L" && a[3] != "p"))
                    return false;
                int size = a[4].toInt();
                EdgePoints points;

                for(int i = 0; i < size; i++) {
                    QPointF p(a[i*2+5].toDouble(), a[i*2+6].toDouble());
                    points.append(p);
                }
                m_edges[key] = points;
            }
            return true;
        // single node
        } else {
            double scale = 72.0;
            QRectF f(0, 0, 0, 0);
            QString id = fixID(keyword);
            if (attributes.contains("pos")) {
                QStringList &a = attributes["pos"];
                QStringList b = a[0].split(",");
                f.setLeft(b[0].toDouble());
                f.setTop(b[1].toDouble());
            }
            if (attributes.contains("height")) {
                QStringList &a = attributes["height"];
                f.setHeight(a[0].toDouble()*scale);
            }

            if (attributes.contains("width")) {
                QStringList &a = attributes["width"];
                f.setWidth(a[0].toDouble()*scale);
            }
            uDebug() << "adding" << id << f;
            m_nodes[id] = f;
        }
    return true;
    }
#endif

    QRectF m_boundingRect;
    NodeType m_nodes;      ///< list of nodes found in parsed dot file
    EdgeType m_edges;      ///< list of edges found in parsed dot file
    QHash<QString, QPointF> m_edgeLabelPosition; ///< contains global node parameters
    QString m_dotPath;     ///< contains path to dot executable

    friend QDebug operator<<(QDebug out, LayoutGenerator &c);
};

#if 0
static QDebug operator<<(QDebug out, LayoutGenerator &c)
{
    out << "LayoutGenerator:"
        << "m_boundingRect:" << c.m_boundingRect
        << "m_nodes:" << c.m_nodes
        << "m_edges:" << c.m_edges
        << "m_scale:" << c.m_scale
        << "m_executable:" << c.m_executable;
    return out;
}
#endif
#endif
