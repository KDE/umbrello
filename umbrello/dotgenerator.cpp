/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// self includes
#include "dotgenerator.h"

// app includes
#include "activitywidget.h"
#include "associationwidget.h"
#include "classifierwidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "debug_utils.h"
#include "uml.h"  // only needed for log{Warn,Error}

// kde includes
#include <KConfigGroup>
#include <KDesktopFile>
#if QT_VERSION < 0x050000
#include <KStandardDirs>
#endif

// qt includes
#include <QFile>
#include <QPaintEngine>
#include <QProcess>
#include <QRectF>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>

QString dotType(WidgetBase *widget)
{
    const QString rawType = widget->baseTypeStr();
    return rawType.toLower().remove(QLatin1String("wt_"));
}

/**
 * dot specific paint engine
 */
class DotPaintEngine : public QPaintEngine
{
public:
    DotPaintEngine(PaintEngineFeatures caps = 0) { Q_UNUSED(caps) }
    virtual ~DotPaintEngine() {}
    virtual bool begin (QPaintDevice * pdev)
    {
        Q_UNUSED(pdev)
        return true;
    }
    virtual void drawEllipse(const QRectF & rect) { Q_UNUSED(rect) }
    virtual void drawEllipse(const QRect & rect) { Q_UNUSED(rect) }
    virtual void drawImage(const QRectF & rectangle, const QImage & image, const QRectF & sr, Qt::ImageConversionFlags flags = Qt::AutoColor) { Q_UNUSED(rectangle) Q_UNUSED(image) Q_UNUSED(sr) Q_UNUSED(flags) }
    virtual void drawLines(const QLineF * lines, int lineCount) { Q_UNUSED(lines) Q_UNUSED(lineCount) }
    virtual void drawLines(const QLine * lines, int lineCount) { Q_UNUSED(lines) Q_UNUSED(lineCount) }
    virtual void drawPath(const QPainterPath & path) { Q_UNUSED(path) }
    virtual void drawPixmap(const QRectF & r, const QPixmap & pm, const QRectF & sr) { Q_UNUSED(r) Q_UNUSED(pm) Q_UNUSED(sr) }
    virtual void drawPoints(const QPointF * points, int pointCount) { Q_UNUSED(points) Q_UNUSED(pointCount) }
    virtual void drawPoints(const QPoint * points, int pointCount) { Q_UNUSED(points) Q_UNUSED(pointCount) }
    virtual void drawPolygon(const QPointF * points, int pointCount, PolygonDrawMode mode) { Q_UNUSED(points) Q_UNUSED(pointCount) Q_UNUSED(mode) }
    virtual void drawPolygon(const QPoint * points, int pointCount, PolygonDrawMode mode) { Q_UNUSED(points) Q_UNUSED(pointCount) Q_UNUSED(mode) }
    virtual void drawRects(const QRectF * rects, int rectCount) { Q_UNUSED(rects) Q_UNUSED(rectCount) }
    virtual void drawRects(const QRect * rects, int rectCount) { Q_UNUSED(rects) Q_UNUSED(rectCount) }
    virtual void drawTextItem(const QPointF & p, const QTextItem & textItem)
    {
        Q_UNUSED(p)
        m_data << textItem.text();
    }
    virtual void drawTiledPixmap(const QRectF & rect, const QPixmap & pixmap, const QPointF & p) { Q_UNUSED(rect) Q_UNUSED(pixmap) Q_UNUSED(p) }
    virtual bool end()
    {
        return true;
    }
    virtual Type type() const
    {
        return QPaintEngine::User;
    }
    virtual void updateState(const QPaintEngineState & state) { Q_UNUSED(state) }

    QStringList m_data;
};

/**
 * dot specific paint device
 */
class DotPaintDevice : public QPaintDevice
{
public:
    DotPaintDevice() : m_engine(new DotPaintEngine)
    {
    }

    ~DotPaintDevice()
    {
        delete m_engine;
    }

    virtual QPaintEngine* paintEngine() const
    {
        return m_engine;
    }

    QStringList &data()
    {
        return m_engine->m_data;
    }

protected:
    virtual int metric(PaintDeviceMetric metric) const
    {
        switch(metric) {
            case QPaintDevice::PdmDpiX: return 1;
            case QPaintDevice::PdmDpiY: return 1;
            case QPaintDevice::PdmWidth: return 100;
            case QPaintDevice::PdmHeight: return 100;
            default: return 0;
        }
        return 0;
    }

    DotPaintEngine *m_engine;
};

#define DOTGENERATOR_DEBUG
/**
 * constructor
 */
DotGenerator::DotGenerator()
  : m_scale(72),
    m_generator(QLatin1String("dot")),
    m_usePosition(false),
    m_useFullNodeLabels(true)
{
    Settings::OptionState& optionState = Settings::optionState();
    if (optionState.autoLayoutState.autoDotPath) {
        m_dotPath = currentDotPath();
    }
    else if (!optionState.autoLayoutState.dotPath.isEmpty()) {
        m_dotPath = optionState.autoLayoutState.dotPath;
    }
}

/**
 * Return the path where dot is installed.
 *
 * @return string with dot path
 */
QString DotGenerator::currentDotPath()
{
#if QT_VERSION >= 0x050000
    QString executable = QStandardPaths::findExecutable(QLatin1String("dot"));
#else
    QString executable = KStandardDirs::findExe(QLatin1String("dot"));
#endif
    if (!executable.isEmpty()) {
        QFileInfo fi(executable);
        return fi.absolutePath();
    }
#ifdef Q_OS_WIN
    // search for dot installation
    QString appDir(QLatin1String(qgetenv("ProgramFiles").constData()));
    QDir dir(appDir);
    dir.setFilter(QDir::Dirs);
    dir.setNameFilters(QStringList() << QLatin1String("Graphviz*"));
    dir.setSorting(QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
    if (list.size() > 0) {
        QString dotPath = list.at(0).absoluteFilePath();
        QString exePath = QFile::exists(dotPath + QLatin1String("\\bin")) ? dotPath + QLatin1String("\\bin") : dotPath;
        return QFile::exists(exePath + QLatin1String("\\dot.exe")) ? exePath : QString();
    }
#endif
    return QString();
}

void DotGenerator::setGeneratorName(const QString &name)
{
    m_generator = name;
    m_version = generatorVersion();
    uDebug() << "found graphviz generator at " << generatorFullPath() << " with version " << m_version;
}

QString DotGenerator::generatorFullPath() const
{
    return m_dotPath + QLatin1Char('/') + m_generator;
}

/**
 * return usage of position attribute
 *
 * @return true if position are used
 */
bool DotGenerator::usePosition() const
{
    return m_usePosition;
}

/**
 * set usage of position attribute in dot file
 *
 * @param state The new state
 */
void DotGenerator::setUsePosition(bool state)
{
    m_usePosition = state;
}

/**
 * return usage of full node labels
 *
 * @return true if position are used
 */
bool DotGenerator::useFullNodeLabels() const
{
    return m_useFullNodeLabels;
}

/**
 * Set usage of full node labels.
 * When set to true labels are extracted from the
 * text output generated by the widget's paint method.
 *
 * @param state The new state
 */
void DotGenerator::setUseFullNodeLabels(bool state)
{
    m_useFullNodeLabels = state;
}

/**
 * Return a list of available templates for a given scene type
 *
 * @param scene The diagram
 * @param configFiles will contain the collected list of config files
 * @return true if collecting succeeds
 */
bool DotGenerator::availableConfigFiles(UMLScene *scene, QHash<QString, QString> &configFiles)
{
    QString diagramType = Uml::DiagramType::toString(scene->type()).toLower();
#if QT_VERSION >= 0x050000
    QStringList fileNames = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QString::fromLatin1("umbrello5/layouts/%1*.desktop").arg(diagramType));
#else
    KStandardDirs dirs;

    QStringList fileNames = dirs.findAllResources("data", QString::fromLatin1("umbrello/layouts/%1*.desktop").arg(diagramType));
#endif
    foreach(const QString &fileName, fileNames) {
        QFileInfo fi(fileName);
        QString baseName;
        if (fi.baseName().contains(QLatin1String("-")))
            baseName = fi.baseName().remove(diagramType + QLatin1Char('-'));
        else if (fi.baseName() == diagramType)
            baseName = fi.baseName();
        else
            baseName = QLatin1String("default");
        KDesktopFile desktopFile(fileName);
        configFiles[baseName] = desktopFile.readName();
    }
    return true;
}

/**
 * Read a layout config file
 *
 * @param diagramType String identifying the diagram
 * @param variant String identifying the variant
 * @return true on success
 */
bool DotGenerator::readConfigFile(QString diagramType, const QString &variant)
{
    QStringList fileNames;

    if (!variant.isEmpty())
        fileNames << QString::fromLatin1("%1-%2.desktop").arg(diagramType).arg(variant);
    fileNames << QString::fromLatin1("%1-default.desktop").arg(diagramType);
    fileNames << QLatin1String("default.desktop");

    QString configFileName;
    foreach(const QString &fileName, fileNames) {
#if QT_VERSION >= 0x050000
        configFileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("umbrello5/layouts/%1").arg(fileName));
#else
        configFileName = KStandardDirs::locate("data", QString::fromLatin1("umbrello/layouts/%1").arg(fileName));
#endif
        if (!configFileName.isEmpty())
            break;
    }

    if (configFileName.isEmpty()) {
        if (variant.isEmpty()) {
            logError1("DotGenerator::readConfigFile could not find layout config file name "
                      "for diagram type %1 and empty variant", diagramType);
        } else {
            logError2("DotGenerator::readConfigFile could not find layout config file name "
                      "for diagram type %1 and variant %2", diagramType, variant);
        }
        return false;
    }
    uDebug() << "reading config file" << configFileName;
    m_configFileName = configFileName;
    KDesktopFile desktopFile(configFileName);
    KConfigGroup edgesAttributes(&desktopFile,"X-UMBRELLO-Dot-Edges");
    KConfigGroup nodesAttributes(&desktopFile,"X-UMBRELLO-Dot-Nodes");
    KConfigGroup attributes(&desktopFile,"X-UMBRELLO-Dot-Attributes");
    QString layoutType = Uml::LayoutType::toString(Settings::optionState().generalState.layoutType);
    KConfigGroup layoutAttributes(&desktopFile,QString(QLatin1String("X-UMBRELLO-Dot-Attributes-%1")).arg(layoutType));
    // settings are not needed by dotgenerator
    KConfigGroup settings(&desktopFile,"X-UMBRELLO-Dot-Settings");

    m_edgeParameters.clear();
    m_nodeParameters.clear();
    m_dotParameters.clear();

    foreach(const QString &key, attributes.keyList()) {
        QString value = attributes.readEntry(key);
        if (!value.isEmpty())
            m_dotParameters[key] = value;
    }

    foreach(const QString &key, layoutAttributes.keyList()) {
        QString value = layoutAttributes.readEntry(key);
        if (!value.isEmpty()) {
            if (!m_dotParameters.contains(key))
                m_dotParameters[key] = value;
            else
                m_dotParameters[key].append(QLatin1String(",") + value);
        }
    }

    foreach(const QString &key, nodesAttributes.keyList()) {
        QString value = nodesAttributes.readEntry(key);
        m_nodeParameters[key] = value;
    }

    foreach(const QString &key, edgesAttributes.keyList()) {
        QString value = edgesAttributes.readEntry(key);
        if (m_edgeParameters.contains(key)) {
            m_edgeParameters[key] += QLatin1Char(',') + value;
        } else {
            m_edgeParameters[key] = value;
        }
    }

    QString value = settings.readEntry(QLatin1String("origin"));
    QStringList a = value.split(QLatin1Char(','));
    if (a.size() == 2)
        m_origin = QPointF(a[0].toDouble(), a[1].toDouble());
    else
        logError1("DotGenerator::readConfigFile illegal format of entry 'origin' value %1",
                  value);

    setGeneratorName(settings.readEntry("generator", "dot"));

#ifdef LAYOUTGENERATOR_DATA_DEBUG
    uDebug() << m_edgeParameters;
    uDebug() << m_nodeParameters;
    uDebug() << m_dotParameters;
#endif
    return true;
}

/**
 * Create dot file using displayed widgets
 * and associations of the provided scene
 * @note This method could also be used as a base to export diagrams as dot file
 *
 * @param scene The diagram from which the widget information is fetched
 * @param fileName Filename where to create the dot file
 * @param variant  Variant string passed to readConfigFile()
 *
 * @return true if generating finished successfully
 */
bool DotGenerator::createDotFile(UMLScene *scene, const QString &fileName, const QString &variant)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QString diagramType = Uml::DiagramType::toString(scene->type()).toLower();
    if (!readConfigFile(diagramType, variant))
        return false;

    QString data;
    QTextStream out(&data);

    foreach(UMLWidget *widget, scene->widgetList()) {
        QStringList params;

        if (m_nodeParameters.contains(QLatin1String("all")))
            params << m_nodeParameters[QLatin1String("all")].split(QLatin1Char(','));

        if (usePosition())
            params  << QString::fromLatin1("pos=\"%1, %2\"").arg(widget->x()+widget->width()/2).arg(widget->y()+widget->height()/2);

        QString type = dotType(widget);

        if (type == QLatin1String("state")) {
            StateWidget *w = static_cast<StateWidget *>(widget);
            type = w->stateTypeStr().toLower();
        }
        else if (type == QLatin1String("activity")) {
            ActivityWidget *w = static_cast<ActivityWidget *>(widget);
            type = w->activityTypeStr().toLower();
        }
        else if (type == QLatin1String("signal")) {
            SignalWidget *w = static_cast<SignalWidget *>(widget);
            type = w->signalTypeStr().toLower();
        }

        QString key = QLatin1String("type::") + type;

        QString label;

        if (!useFullNodeLabels())
            label = widget->name() + QLatin1String("\\n") + type;
        else {
            DotPaintDevice d;
            QPainter p(&d);
            QStyleOptionGraphicsItem options;
            widget->paint(&p, &options);
            label = d.data().join(QLatin1String("\\n"));
        }

        if (label.contains(QLatin1String("\""))) {
            label = label.replace(QLatin1Char('"'), QLatin1String("\\\""));
            uDebug() << "replaced \" in" << label;
        }

        if (m_nodeParameters.contains(key))
            params << m_nodeParameters[key].split(QLatin1Char(','));
        else if (m_nodeParameters.contains(QLatin1String("type::default")))
            params << m_nodeParameters[QLatin1String("type::default")].split(QLatin1Char(','));

        if (!findItem(params, QLatin1String("label=")))
            params << QString::fromLatin1("label=\"%1\"").arg(label);

        if (!findItem(params, QLatin1String("width=")))
            params << QString::fromLatin1("width=\"%1\"").arg(widget->width()/m_scale);

        if (!findItem(params, QLatin1String("height=")))
            params << QString::fromLatin1("height=\"%1\"").arg(widget->height()/m_scale);

#ifdef DOTGENERATOR_DATA_DEBUG
        uDebug() << type << params;
#endif
        QString id = fixID(Uml::ID::toString(widget->localID()));
        if (!widget->isTextWidget())
            out << "\"" << id << "\""
                << " [" << params.join(QLatin1String(",")) << "];\n";
        // add associations for child items
        foreach(QGraphicsItem *item, widget->childItems()) {
            UMLWidget *w2 = dynamic_cast<UMLWidget *>(item);
            if (!w2) {
                logWarn1("DotGenerator::createDotFile: child item of widget %1 is null", key);
                continue;
            }
            QString type2 = dotType(w2);
            QString id2 = fixID(Uml::ID::toString(w2->localID()));
            QStringList params2;
            QString vkey = QString(QLatin1String("visual::type::%1::%2")).arg(type).arg(type2);
            if (m_edgeParameters.contains(vkey)) {
                params2 << m_edgeParameters[vkey];
            } else {
                uDebug() << "key" << vkey << "not found; skipping association";
                continue;
            }
            vkey = QString(QLatin1String("ranking::type::%1::%2")).arg(type).arg(type2);
            if (m_edgeParameters.contains(vkey)) {
                params2 << m_edgeParameters[vkey];
            } else {
                uDebug() << "key" << vkey << "not found";
            }
            out << "\"" << id << "\" -> \"" << id2 << "\""
                << " [" << params2.join(QLatin1String(",")) << "];\n";
        }
    }

    foreach(AssociationWidget *assoc, scene->associationList()) {
        QString type = Uml::AssociationType::toString(assoc->associationType()).toLower();
        QString key = QLatin1String("type::") + type;
        bool swapId = false;

        if (m_edgeParameters.contains(QLatin1String("id::") + key))
            swapId = m_edgeParameters[QLatin1String("id::") + key] == QLatin1String("swap");
        else if (m_edgeParameters.contains(QLatin1String("id::type::default")))
            swapId = m_edgeParameters[QLatin1String("id::type::default")] == QLatin1String("swap");

        QString label;
        if (!useFullNodeLabels())
            label = assoc->name() + QLatin1String("\\n") + type;
        else
            label = assoc->name();

        QString headLabel = assoc->roleName(swapId ? Uml::RoleType::B : Uml::RoleType::A);
        QString tailLabel = assoc->roleName(swapId ? Uml::RoleType::A : Uml::RoleType::B);

        if (!headLabel.isEmpty())
            headLabel.prepend(QLatin1String("+"));
        if (!tailLabel.isEmpty())
            tailLabel.prepend(QLatin1String("+"));

        headLabel += QLatin1String(QLatin1String("  ")) + assoc->multiplicity(swapId ? Uml::RoleType::B : Uml::RoleType::A);
        tailLabel += QLatin1String(QLatin1String("  ")) + assoc->multiplicity(swapId ? Uml::RoleType::A : Uml::RoleType::B);

        QString edgeParameters;
        QStringList params;
        QString rkey = QLatin1String(QLatin1String("ranking::")) + key;
        if (m_edgeParameters.contains(rkey))
            edgeParameters = m_edgeParameters[rkey];
        else if (m_edgeParameters.contains(QLatin1String("ranking::type::default"))) {
            edgeParameters = m_edgeParameters[QLatin1String("ranking::type::default")];
        }
        params << edgeParameters.split(QLatin1Char(','));

        QString vkey = QLatin1String(QLatin1String("visual::")) + key;
        if (m_edgeParameters.contains(vkey))
            edgeParameters = m_edgeParameters[vkey];
        else if (m_edgeParameters.contains(QLatin1String("visual::type::default"))) {
            edgeParameters = m_edgeParameters[QLatin1String("visual::type::default")];
        }
        params << edgeParameters.split(QLatin1Char(','));

        if (!findItem(params, QLatin1String("label=")))
            params << QString::fromLatin1("label=\"%1\"").arg(label);

        if (!findItem(params, QLatin1String("headlabel=")))
            params << QString::fromLatin1("headlabel=\"%1\"").arg(headLabel);

        if (!findItem(params, QLatin1String("taillabel=")))
            params << QString::fromLatin1("taillabel=\"%1\"").arg(tailLabel);

#ifdef DOTGENERATOR_DATA_DEBUG
        uDebug() << type << params;
#endif
        QString aID = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(swapId ? Uml::RoleType::A : Uml::RoleType::B)));
        QString bID = fixID(Uml::ID::toString(assoc->widgetLocalIDForRole(swapId ? Uml::RoleType::B : Uml::RoleType::A)));

        out << "\"" << aID << "\" -> \"" << bID << "\"" << " [" << params.join(QLatin1String(",")) << "];\n";
    }

    QTextStream o(&file);
    o << "# generated from " << m_configFileName << "\n";
    o << "digraph G {\n";

    foreach(const QString &key, m_dotParameters.keys()) {
        o << "\t" << key << " [" << m_dotParameters[key] << "];\n";
    }

    o << data << "\n";
    o << "}\n";

    return true;
}

/**
 * Find string starting with the search string in string list
 * @param  params string list to search in
 * @param  search string
 * @return true when search string has been found
 */
bool DotGenerator::findItem(QStringList &params, const QString &search)
{
    foreach(const QString &s, params) {
        if (s.startsWith(search))
            return true;
    }
    return false;
}

/**
 * There are id wrapped with '"', remove it.
 */
QString DotGenerator::fixID(const QString &_id)
{
    // FIXME: some widget's ids returned from the list are wrapped with "\"", find and fix them
    QString id(_id);
    id.remove(QLatin1Char('"'));
    return id;
}

/**
 * get generator version
 * @return version for example 20130928
 */
int DotGenerator::generatorVersion() const
{
    QProcess p;
    QStringList args;
    args << QLatin1String("-V");
    p.start(generatorFullPath(), args);
    p.waitForFinished();
    QString out(QLatin1String(p.readAllStandardError()));
    QRegExp rx(QLatin1String("\\((.*)\\."));
    QString version = rx.indexIn(out) != -1 ? rx.cap(1) : QString();
    return version.toInt(0);
}

#if 0
static QDebug operator<<(QDebug out, LayoutGenerator &c)
{
    out << "DotGenerator:"
        << "m_boundingRect:" << c.m_boundingRect
        << "m_nodes:" << c.m_nodes
        << "m_edges:" << c.m_edges
        << "m_scale:" << c.m_scale
    return out;
}
#endif
