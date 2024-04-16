/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "layoutgenerator.h"

#include "associationline.h"
#include "associationwidget.h"
#include "cmds.h"
#define DBG_SRC QStringLiteral("LayoutGenerator")
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "uml.h"
#include "umlwidget.h"

// kde includes
#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>

// qt includes
#include <QDir>
#include <QFile>
#include <QHash>
#include <QProcess>
#include <QRegExp>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryFile>
//#include <QTextStream>

//#define USE_XDOT

//#define START_PNGVIEWER

#define LAYOUTGENERATOR_DEBUG
//#define LAYOUTGENERATOR_DATA_DEBUG

//#define SHOW_CONTROLPOINTS

#ifdef LAYOUTGENERATOR_DEBUG

static QString pngViewer()
{
#ifdef Q_OS_WIN
    return QStringLiteral("start");
#else
#ifdef Q_OS_MAC
    return QStringLiteral("unknown");
#else
    return QStringLiteral("okular");
#endif
#endif
}

static QString textViewer()
{
#ifdef Q_OS_WIN
    return QStringLiteral("start");
#else
#ifdef Q_OS_MAC
    return QStringLiteral("unknown");
#else
    return QStringLiteral("mcedit");
#endif
#endif
}
#endif

#ifdef SHOW_CONTROLPOINTS
static QGraphicsPathItem *s_debugItems;
static QPainterPath s_path;
#endif

// Currently this file is not using debug statements. Activate this line when inserting them:
//DEBUG_REGISTER(LayoutGenerator)

/**
 * constructor
*/
LayoutGenerator::LayoutGenerator()
{
    setUseFullNodeLabels(false);
}

/**
 * Return state if layout generator is enabled.
 * It is enabled when the dot application has been found.
 *
 * @return true if enabled
*/
bool LayoutGenerator::isEnabled() const
{
    return !m_dotPath.isEmpty();
}

/**
 * generate layout and apply it to the given diagram.
 *
 * @return true if generating succeeded
*/
bool LayoutGenerator::generate(UMLScene *scene, const QString &variant)
{
    QTemporaryFile in;
    QTemporaryFile out;
    QTemporaryFile xdotOut;
    if (!isEnabled()) {
        logWarn0("LayoutGenerator::generate: Could not apply autolayout because graphviz not found.");
        return false;
    }

#ifdef SHOW_CONTROLPOINTS
    if (!s_debugItems) {
        s_debugItems = new QGraphicsPathItem;
        scene->addItem(s_debugItems);
    }
    s_path = QPainterPath();
    s_debugItems->setPath(s_path);
#endif
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

    QString executable = generatorFullPath();

    QProcess p;
    QStringList args;
    args << QStringLiteral("-o") << out.fileName() << QStringLiteral("-Tplain-ext") << in.fileName();
    p.start(executable, args);
    p.waitForFinished();

    args.clear();
    args << QStringLiteral("-o") << xdotOut.fileName() << QStringLiteral("-Txdot") << in.fileName();
    p.start(executable, args);
    p.waitForFinished();

#ifdef LAYOUTGENERATOR_DEBUG
    QTemporaryFile pngFile;
    pngFile.setAutoRemove(false);
    pngFile.setFileTemplate(QDir::tempPath() + QStringLiteral("/umbrello-layoutgenerator-XXXXXX.png"));
    pngFile.open();
    pngFile.close();
    args.clear();
    args << QStringLiteral("-o") << pngFile.fileName() << QStringLiteral("-Tpng") << in.fileName();
    p.start(executable, args);
    p.waitForFinished();
    qDebug() << pngViewer() << pngFile.fileName();
#ifdef START_PNGVIEWER
    args.clear();
    args << pngFile.fileName();
    p.startDetached(pngViewer(), args);
#endif
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
bool LayoutGenerator::apply(UMLScene *scene)
{
    Q_FOREACH(AssociationWidget *assoc, scene->associationList()) {
        AssociationLine& path = assoc->associationLine();
        QString type = Uml::AssociationType::toString(assoc->associationType()).toLower();
        QString key = QStringLiteral("type::") + type;

        QString id;
        if (m_edgeParameters.contains(QStringLiteral("id::") + key) && m_edgeParameters[QStringLiteral("id::") + key] == QStringLiteral("swap"))
            id = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::A)) + Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::B)));
        else
            id = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::B)) + Uml::ID::toString(assoc->widgetLocalIDForRole(Uml::RoleType::A)));

        // adjust associations not used in the dot file
        if (!m_edges.contains(id)) {
            // shorten line path
            if (path.count() > 2 && assoc->widgetLocalIDForRole(Uml::RoleType::A) != assoc->widgetLocalIDForRole(Uml::RoleType::B)) {
                while (path.count() > 2)
                    path.removePoint(1);
            }
            continue;
        }

        // set label position
        QPointF &l = m_edgeLabelPosition[id];
        FloatingTextWidget *tw = assoc->nameWidget();
        if (tw) {
            tw->setPos(mapToScene(l));
        }

        // setup line points
        EdgePoints &p = m_edges[id];
        int len = p.size();
#ifdef SHOW_CONTROLPOINTS
        QPolygonF pf;
        QFont f;
        for (int i=0; i < len; i++) {
            pf << mapToScene(p[i]);
            s_path.addText(mapToScene(p[i] + QPointF(5,0)), f, QString::number(i));
        }

        s_path.addPolygon(pf);
        s_path.addEllipse(mapToScene(l), 5, 5);
        s_debugItems->setPath(s_path);
#endif
        if (m_version <= 20130928) {
            path.setLayout(Uml::LayoutType::Direct);
            path.cleanup();
            path.setEndPoints(mapToScene(p[0]), mapToScene(p[len-1]));
        } else {
            path.setLayout(Settings::optionState().generalState.layoutType);
            path.cleanup();

            if (Settings::optionState().generalState.layoutType == Uml::LayoutType::Polyline) {
                for (int i = 0; i < len; i++) {
                    if (i > 0 && p[i] == p[i-1])
                        continue;
                    path.addPoint(mapToScene(p[i]));
                }
            } else if(Settings::optionState().generalState.layoutType == Uml::LayoutType::Spline) {
                for (int i = 0; i < len; i++) {
                    path.addPoint(mapToScene(p[i]));
                }
            } else if (Settings::optionState().generalState.layoutType == Uml::LayoutType::Orthogonal) {
                for (int i = 0; i < len; i++) {
                    path.addPoint(mapToScene(p[i]));
                }
            } else
                path.setEndPoints(mapToScene(p[0]), mapToScene(p[len-1]));
        }
    }

    UMLApp::app()->beginMacro(i18n("Apply layout"));

    Q_FOREACH(UMLWidget *widget, scene->widgetList()) {
        QString id = Uml::ID::toString(widget->localID());
        if (!m_nodes.contains(id))
            continue;
        if (widget->isPortWidget() || widget->isPinWidget())
            continue;

#ifdef SHOW_CONTROLPOINTS
        s_path.addRect(QRectF(mapToScene(m_nodes[id].bottomLeft()), m_nodes[id].size()));
        s_path.addRect(QRectF(origin(id), m_nodes[id].size()));
        s_debugItems->setPath(s_path);
#endif
        QPointF p = origin(id);
        widget->setStartMovePosition(widget->pos());
        widget->setX(p.x());
        widget->setY(p.y()-widget->height());
        widget->adjustAssocs(widget->x(), widget->y());    // adjust assoc lines

        UMLApp::app()->executeCommand(new Uml::CmdMoveWidget(widget));
    }
    UMLApp::app()->endMacro();

    Q_FOREACH(AssociationWidget *assoc, scene->associationList()) {
        assoc->calculateEndingPoints();
        assoc->associationLine().update();
        assoc->resetTextPositions();
        assoc->saveIdealTextPositions();
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
bool LayoutGenerator::availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles)
{
    QString diagramType = Uml::DiagramType::toString(scene->type()).toLower();
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("umbrello5/layouts"), QStandardPaths::LocateDirectory);
    QStringList fileNames;
    Q_FOREACH(const QString& dir, dirs) {
        const QStringList entries = QDir(dir).entryList(QStringList() << QString::fromLatin1("%1*.desktop").arg(diagramType));
        Q_FOREACH(const QString& file, entries) {
            fileNames.append(dir + QLatin1Char('/') + file);
        }
    }
    Q_FOREACH(const QString &fileName, fileNames) {
        QFileInfo fi(fileName);
        QString baseName;
        if (fi.baseName().contains(QStringLiteral("-")))
            baseName = fi.baseName().remove(diagramType + QLatin1Char('-'));
        else if (fi.baseName() == diagramType)
            baseName = fi.baseName();
        else
            baseName = QStringLiteral("default");
        KDesktopFile desktopFile(fileName);
        configFiles[baseName] = desktopFile.readName();
    }
    return true;
}

/**
 * Return the origin of node based on the bottom/left corner
 *
 * @param id The widget id to fetch the origin from
 * @return QPoint instance with the coordinates
 */
QPointF LayoutGenerator::origin(const QString &id)
{
    QString key = fixID(id);
    if (!m_nodes.contains(key)) {
#ifdef LAYOUTGENERATOR_DATA_DEBUG
        DEBUG() << "LayoutGenerator::origin(" << id << "): " << key;
#endif
        return QPoint(0,0);
    }
    QRectF &r = m_nodes[key];
    QPointF p(m_origin.x() + r.x() - r.width()/2, m_boundingRect.height() - r.y() + r.height()/2 + m_origin.y());
#ifdef LAYOUTGENERATOR_DATA_DEBUG
    DEBUG() << r << p;
#endif
    return p;
}

/**
 * Read generated dot file and extract positions
 * of the contained widgets.
 *
 * @return true if extracting succeeded
*/
bool LayoutGenerator::readGeneratedDotFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        while(line.endsWith(QLatin1Char(',')))
            line += in.readLine();
        parseLine(line);
    }
    return true;
}

#ifndef USE_XDOT
/**
 * Parse line from dot generated plain-ext output format
 *
 *  The format is documented at https://graphviz.gitlab.io/_pages/doc/info/output.html and looks like:
 *
 *   graph 1 28.083 10.222
 *   node ITfDmJvJE00m 8.0833 8.7361 0.86111 0.45833 QObject solid box black lightgrey
 *   edge sL4cKPpHnJkU sL4cKPpHnJkU 7 8.1253 7.2568 8.2695 7.2687 8.375 7.3127 8.375 7.3889 8.375 7.4377 8.3317 7.4733 8.2627 7.4957 Aggregation 8.8472 7.3889 solid black
 *
 * @param line line in dot plain-ext output format
 * @return true if line could be parsed successfully
*/
bool LayoutGenerator::parseLine(const QString &line)
{
    QStringList a = line.split(QLatin1Char(' '));
    if (a[0] == QStringLiteral("graph")) {
        m_boundingRect = QRectF(0, 0, a[2].toDouble()*m_scale, a[3].toDouble()*m_scale);
        return true;
    } else if (a[0] == QStringLiteral("node")) {
        QString key = fixID(a[1]);
        m_nodes[key] = QRectF(a[2].toDouble()*m_scale, a[3].toDouble()*m_scale,
                              a[4].toDouble()*m_scale, a[5].toDouble()*m_scale);
        return true;
    } else if (a[0] == QStringLiteral("edge")) {
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
    } else if (a[0] == QStringLiteral("stop")) {
        return true;
    }
    return false;
}

#else
typedef QMap<QString,QStringList> ParameterList;

bool LayoutGenerator::splitParameters(QMap<QString,QStringList> &map, const QString &s)
{
    // FIXME: add shape=box without '"'
    static QRegExp rx(QStringLiteral("([a-zA-Z_]+)=\"([a-zA-Z0-9.- #]+)\""));
    static QRegExp rx2(QStringLiteral("([a-zA-Z_]+)=([a-zA-Z0-9.- #]+)"));
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
        if (key == QStringLiteral("pos")) {
            value.remove(QStringLiteral("e,"));
            data = value.split(QLatin1Char(' '));
        } else if (key.startsWith(QLatin1Char('_'))) {
            data = value.split(QLatin1Char(' '));
        }
        else if (key == QStringLiteral("label"))
            data = QStringList() << value;
        else
            data = value.split(QLatin1Char(','));

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

bool LayoutGenerator::parseLine(const QString &line)
{
    static QRegExp m_cols(QStringLiteral("^[\t ]*(.*)[\t ]*\\[(.*)\\]"));
    static int m_level = -1;

    if (line.contains(QLatin1Char('{'))) {
        m_level++;
        return true;
    }
    else if (line.contains(QLatin1Char('}'))) {
        m_level--;
        return true;
    }
    int pos = 0;
    if (m_cols.indexIn(line, pos) == -1)
        return false;

    QString keyword = m_cols.cap(1).trimmed();
    QString attributeString = m_cols.cap(2);
    DEBUG() << "LayoutGenerator::parseLine " <<  keyword << attributeString;
    ParameterList attributes;
    splitParameters(attributes, attributeString);
    DEBUG() << attributes;

    if (keyword == QStringLiteral("graph")) {
        if (attributes.contains(QStringLiteral("bb"))) {
            QStringList &a = attributes[QStringLiteral("bb")];
            m_boundingRect.setLeft(a[0].toDouble());
            m_boundingRect.setTop(a[1].toDouble());
            m_boundingRect.setRight(a[2].toDouble());
            m_boundingRect.setBottom(a[3].toDouble());
        }
    } else if (keyword == QStringLiteral("node")) {
        return true;
    } else if (keyword == QStringLiteral("edge")) {
        return true;
    // transition
    } else if (line.contains(QStringLiteral("->"))) {
        QStringList k = keyword.split(QLatin1Char(' '));
        if (k.size() < 3)
            return false;
        QString key = fixID(k[0]+k[2]);

        if (attributes.contains(QStringLiteral("pos"))) {
            QStringList &a = attributes[QStringLiteral("pos")];
            EdgePoints points;

            for(int i = 1; i < a.size(); i++) {
                QStringList b = a[i].split(QLatin1Char(','));
                QPointF p(b[0].toDouble(), b[1].toDouble());
                points.append(p);
            }
            QStringList b = a[0].split(QLatin1Char(','));
            QPointF p(b[0].toDouble(), b[1].toDouble());
            points.append(p);

            m_edges[key] = points;
        }
        if (0 && attributes.contains(QStringLiteral("_draw_"))) {
            QStringList &a = attributes[QStringLiteral("_draw_")];
            if (a.size() < 5 || (a[3] != QStringLiteral("L") && a[3] != QStringLiteral("p")))
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
        if (attributes.contains(QStringLiteral("pos"))) {
            QStringList &a = attributes[QStringLiteral("pos")];
            QStringList b = a[0].split(QLatin1Char(','));
            f.setLeft(b[0].toDouble());
            f.setTop(b[1].toDouble());
        }
        if (attributes.contains(QStringLiteral("height"))) {
            QStringList &a = attributes[QStringLiteral("height")];
            f.setHeight(a[0].toDouble()*scale);
        }

        if (attributes.contains(QStringLiteral("width"))) {
            QStringList &a = attributes[QStringLiteral("width")];
            f.setWidth(a[0].toDouble()*scale);
        }
        DEBUG() << "LayoutGenerator::parseLine adding " << id << f;
        m_nodes[id] = f;
    }
return true;
}
#endif

/**
 * map dot coordinate to scene coordinate
 * @param p dot point to map
 * @return uml scene coordinate
 */
QPointF LayoutGenerator::mapToScene(const QPointF &p)
{
    return QPointF(p.x()+ m_origin.x(), m_boundingRect.height() - p.y() + m_origin.y());
}

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
