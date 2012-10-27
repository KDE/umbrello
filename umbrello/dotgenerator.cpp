/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// self includes
#include "dotgenerator.h"

// app includes
#include "activitywidget.h"
#include "associationwidget.h"
#include "classifierwidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "debug_utils.h"
#include "umlwidget.h"

// kde includes
#include <KConfigGroup>
#include <KDesktopFile>
#include <KStandardDirs>

// qt includes
#include <QFile>
#include <QPaintEngine>
#include <QProcess>
#include <QRectF>
#include <QRegExp>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>

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
    virtual void drawwPath(const QPainterPath & path) { Q_UNUSED(path) }
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
    m_generator("dot"),
    m_usePosition(false),
    m_useFullNodeLabels(true)
{
}

/**
 * return usage of position attribute
 *
 * @return true if position are used
 */
bool DotGenerator::usePosition()
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
bool DotGenerator::useFullNodeLabels()
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
bool DotGenerator::availableConfigFiles(UMLScene *scene, QHash<QString,QString> &configFiles)
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

/**
 * Read a layout config file
 *
 * @param diagramType String identifing the diagram
 * @param variant String identifing the variant
 * @return true on success
 */
bool DotGenerator::readConfigFile(QString diagramType, const QString &variant)
{
    QStringList fileNames;

    if (!variant.isEmpty())
        fileNames << QString("%1-%2.desktop").arg(diagramType).arg(variant);
    fileNames << QString("%1-default.desktop").arg(diagramType);
    fileNames << "default.desktop";

    QString configFileName;
    foreach(const QString &fileName, fileNames) {
        configFileName = KStandardDirs::locate("data", QString("umbrello/layouts/%1").arg(fileName));
        if (!configFileName.isEmpty())
            break;
    }

    if (configFileName.isEmpty()) {
        uError() << "could not find layout config file name for diagram type" << diagramType << "and variant" << variant;
        return false;
    }
    uDebug() << "reading config file" << configFileName;
    m_configFileName = configFileName;
    KDesktopFile desktopFile(configFileName);
    KConfigGroup edgesAttributes(&desktopFile,"X-UMBRELLO-Dot-Edges");
    KConfigGroup nodesAttributes(&desktopFile,"X-UMBRELLO-Dot-Nodes");
    KConfigGroup attributes(&desktopFile,"X-UMBRELLO-Dot-Attributes");
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

    foreach(const QString &key, nodesAttributes.keyList()) {
        QString value = nodesAttributes.readEntry(key);
        m_nodeParameters[key] = value;
    }

    foreach(const QString &key, edgesAttributes.keyList()) {
        QString value = edgesAttributes.readEntry(key);
        if (m_edgeParameters.contains(key)) {
            m_edgeParameters[key] += ',' + value;
        } else {
            m_edgeParameters[key] = value;
        }
    }

    QString value = settings.readEntry("origin");
    QStringList a = value.split(",");
    if (a.size() == 2)
        m_origin = QPointF(a[0].toDouble(), a[1].toDouble());
    else
        uError() << "illegal format of entry 'origin'" << value;

    m_generator = settings.readEntry("generator","dot");

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
 * @param fileName Filename where to create the dot file
 * @param scene The diagram from which the widget informations are fetched
 *
 * @return true if generating finished successfully
 */
bool DotGenerator::createDotFile(UMLScene *scene, const QString &fileName, const QString &variant)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QString diagramType = scene->type().toString().toLower();
    if (!readConfigFile(diagramType, variant))
        return false;

    QString data;
    QTextStream out(&data);

    foreach(UMLWidget *widget, scene->widgetList()) {
        QStringList params;

        if (m_nodeParameters.contains("all"))
            params << m_nodeParameters["all"].split(',');

        if (usePosition())
            params  << QString("pos=\"%1,%2\"").arg(widget->pos().x()+widget->width()/2).arg(widget->pos().y()+widget->height()/2);

        QString type = QString(widget->baseTypeStr()).toLower().remove("wt_");

        if (type == "state") {
            StateWidget *w = static_cast<StateWidget *>(widget);
            type = w->stateTypeStr().toLower();
        }
        else if (type == "activity") {
            ActivityWidget *w = static_cast<ActivityWidget *>(widget);
            type = w->activityTypeStr().toLower();
        }
        else if (type == "signal") {
            SignalWidget *w = static_cast<SignalWidget *>(widget);
            type = w->signalTypeStr().toLower();
        }

        QString key = "type::" + type;

        QString label;

        if (!useFullNodeLabels())
            label = widget->name() + "\\n" + type;
        else {
            DotPaintDevice d;
            QPainter p(&d);
            widget->paint(&p, 0, 0);
            label = d.data().join("\\n");
        }
    
        if (m_nodeParameters.contains(key))
            params << m_nodeParameters[key].split(',');
        else if (m_nodeParameters.contains("type::default"))
            params << m_nodeParameters["type::default"].split(',');

        if (!findItem(params,"label="))
            params << QString("label=\"%1\"").arg(label);

        if (!findItem(params,"width="))
            params << QString("width=\"%1\"").arg(widget->width()/m_scale);

        if (!findItem(params,"height="))
            params << QString("height=\"%1\"").arg(widget->height()/m_scale);

#ifdef DOTGENERATOR_DATA_DEBUG
        uDebug() << type << params;
#endif
        QString id = fixID(ID2STR(widget->id()));
        if (widget->baseType() != WidgetBase::wt_Text)
            out << "\"" << id << "\""
                << " [" << params.join(",") << "];\n";
    }

    foreach(AssociationWidget *assoc, scene->associationList()) {
        QString type = assoc->associationType().toString().toLower();
        QString key = "type::" + type;
        bool swapId = false;

        if (m_edgeParameters.contains("id::" + key))
            swapId = m_edgeParameters["id::" + key] == "swap";
        else if (m_edgeParameters.contains("id::type::default"))
            swapId = m_edgeParameters["id::type::default"] == "swap";

        QString label;
        if (!useFullNodeLabels())
            label = assoc->name() + "\\n" + type;
        else
            label = assoc->name();

        QString headLabel = assoc->roleName(swapId ? Uml::B : Uml::A);
        QString tailLabel = assoc->roleName(swapId ? Uml::A : Uml::B);

        if (!headLabel.isEmpty())
            headLabel.prepend("+");
        if (!tailLabel.isEmpty())
            tailLabel.prepend("+");

        headLabel += QLatin1String("  ") + assoc->multiplicity(swapId ? Uml::B : Uml::A);
        tailLabel += QLatin1String("  ") + assoc->multiplicity(swapId ? Uml::A : Uml::B);

        QString edgeParameters;
        QStringList params;
        QString rkey = QLatin1String("ranking::") + key;
        if (m_edgeParameters.contains(rkey))
            edgeParameters = m_edgeParameters[rkey];
        else if (m_edgeParameters.contains("ranking::type::default")) {
            edgeParameters = m_edgeParameters["ranking::type::default"];
        }
        params << edgeParameters.split(',');

        QString vkey = QLatin1String("visual::") + key;
        if (m_edgeParameters.contains(vkey))
            edgeParameters = m_edgeParameters[vkey];
        else if (m_edgeParameters.contains("visual::type::default")) {
            edgeParameters = m_edgeParameters["visual::type::default"];
        }
        params << edgeParameters.split(',');

        if (!findItem(params,"label="))
            params << QString("label=\"%1\"").arg(label);

        if (!findItem(params,"headlabel="))
            params << QString("headlabel=\"%1\"").arg(headLabel);

        if (!findItem(params,"taillabel="))
            params << QString("taillabel=\"%1\"").arg(tailLabel);

#ifdef DOTGENERATOR_DATA_DEBUG
        uDebug() << type << params;
#endif
        QString aID = fixID(ID2STR(assoc->widgetIDForRole(swapId ? Uml::A : Uml::B)));
        QString bID = fixID(ID2STR(assoc->widgetIDForRole(swapId ? Uml::B : Uml::A)));

        out << "\"" << aID << "\" -> \"" << bID << "\"" << " [" << params.join(",") << "];\n";
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
 * @params params string list to search in
 * @params search string
 * @return true, when search string has been found
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
    id.remove("\"");
    return id;
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
