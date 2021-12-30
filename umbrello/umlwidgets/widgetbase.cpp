/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "widgetbase.h"

#include "classifier.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "floatingtextwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobject.h"
#include "umlscene.h"
#include "widgetbasepopupmenu.h"
#include "uniqueid.h"

#if QT_VERSION < 0x050000
#include <kcolordialog.h>
#include <kfontdialog.h>
#endif
#include <KLocalizedString>

#include <QAction>
#if QT_VERSION >= 0x050000
#include <QColorDialog>
#include <QFontDialog>
#endif
#include <QPointer>
#include <QXmlStreamWriter>

static unsigned eventCnt = 0;

void QGraphicsObjectWrapper::setSelected(bool state)
{
    if (!m_calledFromItemChange)
        QGraphicsObject::setSelected(state);
    QString info;
    WidgetBase *wb = dynamic_cast<WidgetBase*>(this);
    if (wb)
        info = wb->name();
    if (info.isEmpty()) {
        DEBUG(DBG_SRC) << ++eventCnt << " new state=" << state << ", fromItemChange=" << m_calledFromItemChange << " " << this;
    } else {
        DEBUG(DBG_SRC) << ++eventCnt << " new state=" << state << ", fromItemChange=" << m_calledFromItemChange << " " << info;
    }
    m_calledFromItemChange = false;
}

QVariant QGraphicsObjectWrapper::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        bool state = value.toBool();
        m_calledFromItemChange = true;
        setSelected(state);
    }
    return QGraphicsItem::itemChange(change, value);
}

/**
 * Creates a WidgetBase object.
 *
 * @param scene   The view to be displayed on.
 * @param type    The WidgetType to construct.  This must be set to the appropriate
 *                value by the constructors of inheriting classes.
 */
WidgetBase::WidgetBase(UMLScene *scene, WidgetType type, Uml::ID::Type id)
  : QGraphicsObjectWrapper(),
    m_baseType(type),
    m_scene(scene),
    m_umlObject(0),
    m_nId(id == Uml::ID::None ? UniqueID::gen() : id),
    m_nLocalID(UniqueID::gen()),
    m_textColor(QColor("black")),
    m_fillColor(QColor("yellow")),
    m_brush(m_fillColor),
    m_lineWidth(0), // initialize with 0 to have valid start condition
    m_useFillColor(true),
    m_usesDiagramFillColor(true),
    m_usesDiagramLineColor(true),
    m_usesDiagramLineWidth(true),
    m_usesDiagramTextColor(true),
    m_usesDiagramUseFillColor(true),
    m_autoResize(true),
    m_changesShape(false)
{
    Q_ASSERT(m_baseType > wt_Min && m_baseType < wt_Max);
    setFlags(ItemIsSelectable);
    //setFlags(ItemIsSelectable | ItemIsMovable |ItemSendsGeometryChanges);

    // Note: no virtual methods from derived classes available,
    // this operation need to be finished in derived class constructor.
    setLineColor(QColor("black"));
    setSelected(false);

    if (m_scene) {
        m_usesDiagramLineColor = true;
        m_usesDiagramLineWidth  = true;
        m_usesDiagramTextColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_textColor = optionState.uiState.textColor;
        setLineColor(optionState.uiState.lineColor);
        setLineWidth(optionState.uiState.lineWidth);
        m_font = optionState.uiState.font;
    } else {
        uError() << "WidgetBase constructor: SERIOUS PROBLEM - m_scene is NULL";
    }
}

/**
 * Destructor.
 */
WidgetBase::~WidgetBase()
{
}

/**
 * Read property of m_baseType.
 */
WidgetBase::WidgetType WidgetBase::baseType() const
{
    Q_ASSERT(m_baseType > wt_Min && m_baseType < wt_Max);
    return m_baseType;
}

/**
 * Set property m_baseType. Used for types changing their types during runtime.
 */
void WidgetBase::setBaseType(const WidgetType& baseType)
{
    Q_ASSERT(baseType > wt_Min && baseType < wt_Max);
    m_baseType = baseType;
}

/**
 * @return The type used for rtti as string.
 */
QLatin1String WidgetBase::baseTypeStr() const
{
    Q_ASSERT(m_baseType > wt_Min && m_baseType < wt_Max);
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, m_baseType));
}

/**
 * @return The type as string without 'wt_' prefix.
 */
QString WidgetBase::baseTypeStrWithoutPrefix() const
{
    QString rawType = baseTypeStr();
    return rawType.remove(QLatin1String("wt_"));
}

/*
 * Sets the state of whether the widget is selected.
 *
 * @param select   The state of whether the widget is selected.
 */
void WidgetBase::setSelected(bool select)
{
    QGraphicsObjectWrapper::setSelected(select);
}

/**
 * Deliver a pointer to the connected UMLView
 * (needed esp. by event handling of AssociationLine).
 */
UMLScene* WidgetBase::umlScene() const
{
    return m_scene;
}

/**
 * This is shortcut method for UMLApp::app()->document().
 *
 * @return Pointer to the UMLDoc object.
 */
UMLDoc* WidgetBase::umlDoc() const
{
    return UMLApp::app()->document();
}

/**
 * Returns the @ref UMLObject set to represent.
 *
 * @return the UMLObject to represent.
 */
UMLObject* WidgetBase::umlObject() const
{
    return m_umlObject;
}

/**
 * Sets the @ref UMLObject to represent.
 *
 * @param obj  The object to represent.
 */
void WidgetBase::setUMLObject(UMLObject *obj)
{
    m_umlObject = obj;
}

/**
 * Write property of m_nId.
 */
void WidgetBase::setID(Uml::ID::Type id)
{
    if (m_umlObject) {
        if (m_umlObject->id() != Uml::ID::None)
            uWarning() << "changing old UMLObject " << Uml::ID::toString(m_umlObject->id())
                << " to " << Uml::ID::toString(id);
        m_umlObject->setID(id);
    }
    m_nId = id;
}

/**
 * Read property of m_nId.
 */
Uml::ID::Type WidgetBase::id() const
{
    if (m_umlObject)
        return m_umlObject->id();
    return m_nId;
}

/**
 * Sets the local id of the object.
 *
 * @param id   The local id of the object.
 */
void WidgetBase::setLocalID(Uml::ID::Type id)
{
    m_nLocalID = id;
}

/**
 * Returns the local ID for this object.  This ID is used so that
 * many objects of the same @ref UMLObject instance can be on the
 * same diagram.
 *
 * @return  The local ID.
 */
Uml::ID::Type WidgetBase::localID() const
{
    return m_nLocalID;
}

/**
 * Returns the widget with the given ID.
 * The default implementation tests the following IDs:
 * - m_nLocalID
 * - if m_umlObject is non NULL: m_umlObject->id()
 * - m_nID
 * Composite widgets override this function to test further owned widgets.
 *
 * @param id  The ID to test this widget against.
 * @return  'this' if id is either of m_nLocalID, m_umlObject->id(), or m_nId;
 *           else NULL.
 */
UMLWidget* WidgetBase::widgetWithID(Uml::ID::Type id)
{
    if (id == m_nLocalID ||
            (m_umlObject != nullptr && id == m_umlObject->id()) ||
            id == m_nId)
        return this->asUMLWidget();
    return nullptr;
}

/**
 * Used by some child classes to get documentation.
 *
 * @return  The documentation from the UMLObject (if m_umlObject is set.)
 */
QString WidgetBase::documentation() const
{
    if (m_umlObject)
        return m_umlObject->doc();
    return m_Doc;
}

/**
 * Returns state of documentation for the widget.
 *
 * @return false if documentation is empty
 */
bool WidgetBase::hasDocumentation()
{
    if (m_umlObject)
        return m_umlObject->hasDoc();
    return !m_Doc.isEmpty();
}

/**
 * Used by some child classes to set documentation.
 *
 * @param doc   The documentation to be set in the UMLObject
 *              (if m_umlObject is set.)
 */
void WidgetBase::setDocumentation(const QString& doc)
{
    if (m_umlObject)
        m_umlObject->setDoc(doc);
    else
        m_Doc = doc;
}

/**
 * Gets the name from the corresponding UMLObject if this widget has an
 * underlying UMLObject; if it does not then it returns the local
 * m_Text (notably the case for FloatingTextWidget.)
 *
 * @return the currently set name
 */
QString WidgetBase::name() const
{
    if (m_umlObject)
        return m_umlObject->name();
    return m_Text;
}

/**
 * Sets the name in the corresponding UMLObject.
 * Sets the local m_Text if m_umlObject is NULL.
 *
 * @param strName The name to be set.
 */
void WidgetBase::setName(const QString &strName)
{
    if (m_umlObject)
        m_umlObject->setName(strName);
    else
        m_Text = strName;
}

/**
 * Returns text color
 *
 * @return currently used text color
 */
QColor WidgetBase::textColor() const
{
    return m_textColor;
}

/**
 * Sets the text color
 *
 * @param color the new text color
 */
void WidgetBase::setTextColor(const QColor &color)
{
    m_textColor = color;
    m_usesDiagramTextColor = false;
}

/**
 * Returns line color
 *
 * @return currently used line color
 */
QColor WidgetBase::lineColor() const
{
    return m_lineColor;
}

/**
 * Sets the line color
 *
 * @param color   The new line color
 */
void WidgetBase::setLineColor(const QColor &color)
{
    m_lineColor = color;
    m_usesDiagramLineColor = false;
}

/**
 * Returns fill color
 *
 * @return currently used fill color
 */
QColor WidgetBase::fillColor() const
{
    return m_fillColor;
}

/**
 * Sets the fill color
 *
 * @param color   The new fill color
 */
void WidgetBase::setFillColor(const QColor &color)
{
    m_fillColor = color;
    m_usesDiagramFillColor = false;
}

/**
 * Returns line width
 *
 * @return currently used line with
 */
uint WidgetBase::lineWidth() const
{
    return m_lineWidth;
}

/**
 * Sets the line width
 *
 * @param width  The new line width
 */
void WidgetBase::setLineWidth(uint width)
{
    m_lineWidth = width;
    m_usesDiagramLineWidth = false;
}

/**
 * Return state of fill color usage
 *
 * @return True if fill color is used
 */
bool WidgetBase::useFillColor()
{
    return m_useFillColor;
}

/**
 * Set state if fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUseFillColor(bool state)
{
    m_useFillColor = state;
    m_usesDiagramUseFillColor = false;
}

/**
 * Returns state if diagram text color is used
 *
 * @return True means diagram text color is used
 */
bool WidgetBase::usesDiagramTextColor() const
{
    return m_usesDiagramTextColor;
}

/**
 * Set state if diagram text color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramTextColor(bool state)
{
    if (m_usesDiagramTextColor == state) {
        return;
    }
    m_usesDiagramTextColor = state;
    setTextColor(m_textColor);
}

/**
 * Returns state of diagram line color is used
 *
 * @return True means diagrams line color is used
*/
bool WidgetBase::usesDiagramLineColor() const
{
    return m_usesDiagramLineColor;
}

/**
 * Set state of diagram line color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramLineColor(bool state)
{
    m_usesDiagramLineColor = state;
}

/**
 * Returns state of diagram fill color is used
 *
 * @return True means diagrams fill color is used
*/
bool WidgetBase::usesDiagramFillColor() const
{
    return m_usesDiagramFillColor;
}

/**
 * Set state if diagram fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramFillColor(bool state)
{
    m_usesDiagramFillColor = state;
}

/**
 * Returns state of diagram use fill color is used
 *
 * @return True means diagrams fill color is used
*/
bool WidgetBase::usesDiagramUseFillColor() const
{
    return m_usesDiagramUseFillColor;
}

/**
 * Set state of diagram use fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramUseFillColor(bool state)
{
    m_usesDiagramUseFillColor = state;
}

/**
 * Returns state of diagram line width is used
 *
 * @return True means diagrams line width is used
 */
bool WidgetBase::usesDiagramLineWidth() const
{
    return m_usesDiagramLineWidth;
}

/**
 * Set state of diagram line width is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramLineWidth(bool state)
{
    m_usesDiagramLineWidth = state;
}

/**
 * Returns the font used for displaying any text.
 * @return the font
 */
QFont WidgetBase::font() const
{
    return m_font;
}

/**
 * Set the font used to display text inside this widget.
 */
void WidgetBase::setFont(const QFont& font)
{
    m_font = font;
}

/**
 * Return state of auto resize property
 * @return the auto resize state
 */
bool WidgetBase::autoResize()
{
    return m_autoResize;
}

/**
 * set auto resize state
 * @param state
 */
void WidgetBase::setAutoResize(bool state)
{
    m_autoResize = state;
}

/**
 * Return changes state property
 * @return the changes shape state
 */
bool WidgetBase::changesShape()
{
    return m_changesShape;
}

/**
 * set changes shape property
 * @param state
 */
void WidgetBase::setChangesShape(bool state)
{
    m_changesShape = state;
}

/**
 * A virtual method for the widget to display a property dialog box.
 * Subclasses should reimplement this appropriately.
 * In case the user cancels the dialog or there are some requirements
 * not fulfilled the method returns false; true otherwise.
 *
 * @return true - properties has been applyed
 * @return false - properties has not been applied
 *
 */
bool WidgetBase::showPropertiesDialog()
{
    return false;
}

/**
 * A virtual method to save the properties of this widget into a
 * QXmlStreamWriter i.e. XML.
 *
 * Subclasses should first create a new dedicated element as the child
 * of \a qElement parameter passed.  Then this base method should be
 * called to save basic widget properties.
 *
 * @param writer The QXmlStreamWriter to write to.
 */
void WidgetBase::saveToXMI(QXmlStreamWriter& writer)
 {
    writer.writeAttribute(QLatin1String("xmi.id"), Uml::ID::toString(id()));
    // Unique identifier for widget (todo: id() should be unique, new attribute
    // should indicate the UMLObject's ID it belongs to)
    writer.writeAttribute(QLatin1String("localid"), Uml::ID::toString(m_nLocalID));

    writer.writeAttribute(QLatin1String("textcolor"), m_usesDiagramTextColor ? QLatin1String("none")
                                                                             : m_textColor.name());
    if (m_usesDiagramLineColor) {
        writer.writeAttribute(QLatin1String("linecolor"), QLatin1String("none"));
    } else {
        writer.writeAttribute(QLatin1String("linecolor"), m_lineColor.name());
    }
    if (m_usesDiagramLineWidth) {
        writer.writeAttribute(QLatin1String("linewidth"), QLatin1String("none"));
    } else {
        writer.writeAttribute(QLatin1String("linewidth"), QString::number(m_lineWidth));
    }
    writer.writeAttribute(QLatin1String("usefillcolor"), QString::number(m_useFillColor));
    // for consistency the following attributes now use american spelling for "color"
    writer.writeAttribute(QLatin1String("usesdiagramfillcolor"), QString::number(m_usesDiagramFillColor));
    writer.writeAttribute(QLatin1String("usesdiagramusefillcolor"), QString::number(m_usesDiagramUseFillColor));
    if (m_usesDiagramFillColor) {
        writer.writeAttribute(QLatin1String("fillcolor"), QLatin1String("none"));
    } else {
        writer.writeAttribute(QLatin1String("fillcolor"), m_fillColor.name());
    }
    writer.writeAttribute(QLatin1String("font"), m_font.toString());
    writer.writeAttribute(QLatin1String("autoresize"), QString::number(m_autoResize ? 1 : 0));
}


/**
 * Returns whether the widget type has an associated UMLObject
 */
bool WidgetBase::widgetHasUMLObject(WidgetBase::WidgetType type)
{
    if (type == WidgetBase::wt_Actor         ||
            type == WidgetBase::wt_UseCase   ||
            type == WidgetBase::wt_Class     ||
            type == WidgetBase::wt_Interface ||
            type == WidgetBase::wt_Enum      ||
            type == WidgetBase::wt_Datatype  ||
            type == WidgetBase::wt_Package   ||
            type == WidgetBase::wt_Component ||
            type == WidgetBase::wt_Port ||
            type == WidgetBase::wt_Node      ||
            type == WidgetBase::wt_Artifact  ||
            type == WidgetBase::wt_Object) {
        return true;
    } else {
        return false;
    }
}

/**
 * Activate the object after deserializing it from XMI
 *
 * @param ChangeLog
 * @return  true for success
 */
bool WidgetBase::activate(IDChangeLog* changeLog)
{
    Q_UNUSED(changeLog);

    if (widgetHasUMLObject(baseType()) && m_umlObject == nullptr) {
        m_umlObject =  UMLApp::app()->document()->findObjectById(m_nId);
        if (m_umlObject == nullptr) {
            uError() << "cannot find UMLObject with id=" << Uml::ID::toString(m_nId);
            return false;
        }
    }
    return true;
}

/**
 * Adds an already created association to the list of
 * associations that include this UMLWidget
 */
void WidgetBase::addAssoc(AssociationWidget *pAssoc)
{
    Q_UNUSED(pAssoc);
}

/**
 * Removes an already created association from the list of
 * associations that include this UMLWidget
 */
void WidgetBase::removeAssoc(AssociationWidget *pAssoc)
{
    Q_UNUSED(pAssoc);
}

/**
 * A virtual method to load the properties of this widget from a
 * QDomElement into this widget.
 *
 * Subclasses should reimplement this to load additional properties
 * required, calling this base method to load the basic properties of
 * the widget.
 *
 * @param qElement A QDomElement which contains xml info for this widget.
 *
 * @todo Add support to load older version.
 */
bool WidgetBase::loadFromXMI(QDomElement& qElement)
{
    QString id = qElement.attribute(QLatin1String("xmi.id"), QLatin1String("-1"));
    m_nId = Uml::ID::fromString(id);

    QString localid = qElement.attribute(QLatin1String("localid"), QLatin1String("0"));
    if (localid != QLatin1String("0")) {
        m_nLocalID = Uml::ID::fromString(localid);
    }

    // first load from "linecolour" and then overwrite with the "linecolor"
    // attribute if that one is present. The "linecolour" name was a "typo" in
    // earlier versions of Umbrello
    QString lineColor = qElement.attribute(QLatin1String("linecolour"), QLatin1String("none"));
    lineColor = qElement.attribute(QLatin1String("linecolor"), lineColor);
    if (lineColor != QLatin1String("none")) {
        setLineColor(QColor(lineColor));
        m_usesDiagramLineColor = false;
    } else if (m_baseType != WidgetBase::wt_Box && m_scene != 0) {
        setLineColor(m_scene->lineColor());
        m_usesDiagramLineColor = true;
    }
    QString lineWidth = qElement.attribute(QLatin1String("linewidth"), QLatin1String("none"));
    if (lineWidth != QLatin1String("none")) {
        setLineWidth(lineWidth.toInt());
        m_usesDiagramLineWidth = false;
    } else if (m_scene) {
        setLineWidth(m_scene->lineWidth());
        m_usesDiagramLineWidth = true;
    }
    QString textColor = qElement.attribute(QLatin1String("textcolor"), QLatin1String("none"));
    if (textColor != QLatin1String("none")) {
        m_textColor = QColor(textColor);
        m_usesDiagramTextColor = false;
    } else if (m_scene) {
        m_textColor = m_scene->textColor();
        m_usesDiagramTextColor = true;
    }
    QString usefillcolor = qElement.attribute(QLatin1String("usefillcolor"), QLatin1String("1"));
    m_useFillColor = (bool)usefillcolor.toInt();
    /*
      For the next three *color attributes, there was a mixup of american and english spelling for "color".
      So first we need to keep backward compatibility and try to retrieve the *colour attribute.
      Next we overwrite this value if we find a *color, otherwise the former *colour is kept.
    */
    QString fillColor = qElement.attribute(QLatin1String("fillcolour"), QLatin1String("none"));
    fillColor = qElement.attribute(QLatin1String("fillcolor"), fillColor);
    if (fillColor != QLatin1String("none")) {
        m_fillColor = QColor(fillColor);
    }

    QString usesDiagramFillColor = qElement.attribute(QLatin1String("usesdiagramfillcolour"), QLatin1String("1"));
    usesDiagramFillColor = qElement.attribute(QLatin1String("usesdiagramfillcolor"), usesDiagramFillColor);
    m_usesDiagramFillColor = (bool)usesDiagramFillColor.toInt();

    QString usesDiagramUseFillColor = qElement.attribute(QLatin1String("usesdiagramusefillcolour"), QLatin1String("1"));
    usesDiagramUseFillColor = qElement.attribute(QLatin1String("usesdiagramusefillcolor"), usesDiagramUseFillColor);
    m_usesDiagramUseFillColor = (bool)usesDiagramUseFillColor.toInt();

    QString font = qElement.attribute(QLatin1String("font"));
    if (!font.isEmpty()) {
        QFont newFont;
        newFont.fromString(font);
        m_font = newFont;
    } else {
        uWarning() << "Using default font " << m_font.toString()
                   << " for widget with xmi.id " << Uml::ID::toString(m_nId);
    }
    QString autoResize = qElement.attribute(QLatin1String("autoresize"), QLatin1String("1"));
    m_autoResize = (bool)autoResize.toInt();

    return true;
}

/**
 * Assignment operator
 */
WidgetBase& WidgetBase::operator=(const WidgetBase& other)
{
    m_baseType = other.m_baseType;
    m_scene = other.m_scene;
    m_umlObject = other.m_umlObject;
    m_Doc = other.m_Doc;
    m_Text = other.m_Text;
    m_nId = other.m_nId;
    m_nLocalID = other.m_nLocalID;
    m_textColor = other.m_textColor;
    setLineColor(other.lineColor());
    m_fillColor = other.m_fillColor;
    m_brush = other.m_brush;
    m_font = other.m_font;
    m_lineWidth  = other.m_lineWidth;
    m_useFillColor = other.m_useFillColor;
    m_usesDiagramTextColor = other.m_usesDiagramTextColor;
    m_usesDiagramLineColor = other.m_usesDiagramLineColor;
    m_usesDiagramFillColor = other.m_usesDiagramFillColor;
    m_usesDiagramLineWidth  = other.m_usesDiagramLineWidth;
    setSelected(other.isSelected());

    return *this;
}

/**
 * return drawing rectangle of widget in local coordinates
 */
QRectF WidgetBase::rect() const
{
    return m_rect;
}

/**
 * set widget rectangle in item coordinates
 */
void WidgetBase::setRect(const QRectF& rect)
{
    if (m_rect == rect)
        return;
    uDebug() << "WidgetBase::setRect(" << name() << ") : x = " << rect.x() << " , y = " << rect.y();
    prepareGeometryChange();
    m_rect = rect;
    update();
}

/**
 * set widget rectangle in item coordinates
 */
void WidgetBase::setRect(qreal x, qreal y, qreal width, qreal height)
{
    setRect(QRectF(x, y, width, height));
}

/**
 * @return The bounding rectangle for this widget.
 * @see setRect
 */
QRectF WidgetBase::boundingRect() const
{
    int halfWidth = lineWidth()/2;
    return m_rect.adjusted(-halfWidth, -halfWidth, halfWidth, halfWidth);
}

/**
 * Test if point is inside the bounding rectangle of the widget.
 * Inheriting classes may reimplement this to test possible child widgets.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if the given point is in the boundaries of the widget;
 *         else NULL.
 */
UMLWidget* WidgetBase::onWidget(const QPointF &p)
{
    UMLWidget *uw = this->asUMLWidget();
    if (uw == 0)
        return 0;
    const qreal w = m_rect.width();
    const qreal h = m_rect.height();
    const qreal left = x();  // don't use m_rect.x() for this, it is always 0
    const qreal right = left + w;
    const qreal top = y();   // don't use m_rect.y() for this, it is always 0
    const qreal bottom = top + h;
    // uDebug() << "p=(" << p.x() << "," << p.y()
    //          << "), x=" << left << ", y=" << top << ", w=" << w << ", h=" << h
    //          << "; right=" << right << ", bottom=" << bottom;
    if (p.x() < left || p.x() > right ||
            p.y() < top || p.y() > bottom) { // Qt coord.sys. origin in top left corner
        // uDebug() << "returning NULL";
        return 0;
    }
    // uDebug() << "returning this";
    return uw;
}

/**
 * Draws the UMLWidget on the given paint device
 *
 * @param painter The painter for the drawing device
 * @param option  Painting related options
 * @param widget  Background widget on which to paint (optional)
 *
 */
void WidgetBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter); Q_UNUSED(option); Q_UNUSED(widget);
}

/**
 * Reimplemented to show appropriate context menu.
 */
void WidgetBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();
    uDebug() << "widget = " << name() << " / type = " << baseTypeStr();

    UMLScene *scene = umlScene();

    // If right-click was done on a widget that was not selected, clear the
    // current selection and select the new widget. The context menu is shown
    // with actions for that single widget.
    // If a keyboard modifier was used, add the widget to the current selection
    // and show the menu with actions for the whole selection.
    if (!isSelected()) {
        Qt::KeyboardModifiers forSelection = (Qt::ControlModifier | Qt::ShiftModifier);
        if ((event->modifiers() & forSelection) == 0) {
            scene->clearSelected();
        }

        if (umlObject() != 0) {
            scene->selectWidget(this->asUMLWidget());
        } else {
            setSelected(true);
        }
    }

    int count = scene->selectedCount(true);

    // Determine multi state
    bool multi = (isSelected() && count > 1);

    WidgetBasePopupMenu popup(0, this, multi, scene->getUniqueSelectionType());

    // Disable the "view code" menu for simple code generators
    if (UMLApp::app()->isSimpleCodeGeneratorActive()) {
        popup.setActionEnabled(ListPopupMenu::mt_ViewCode, false);
    }

    QAction *triggered = popup.exec(event->screenPos());
    slotMenuSelection(triggered);
}

/**
 * This is usually called synchronously after menu.exec() and \a
 * trigger's parent is always the ListPopupMenu which can be used to
 * get the type of action of \a trigger.
 *
 * @note Subclasses can reimplement to handle specific actions and
 *       leave the rest to WidgetBase::slotMenuSelection.
 */
void WidgetBase::slotMenuSelection(QAction *trigger)
{
    if (!trigger) {
        return;
    }
    QColor newColor;

    const WidgetType wt = m_baseType; // short hand name

    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(trigger);
    switch (sel) {
    case ListPopupMenu::mt_Rename:
        umlDoc()->renameUMLObject(umlObject());
        break;

    case ListPopupMenu::mt_Properties:
        if (wt == WidgetBase::wt_Actor     || wt == WidgetBase::wt_UseCase   ||
            wt == WidgetBase::wt_Package   || wt == WidgetBase::wt_Interface ||
            wt == WidgetBase::wt_Datatype  || wt == WidgetBase::wt_Node      ||
            wt == WidgetBase::wt_Component || wt == WidgetBase::wt_Artifact  ||
            wt == WidgetBase::wt_Enum      || wt == WidgetBase::wt_Entity    ||
            wt == WidgetBase::wt_Port      || wt == WidgetBase::wt_Instance ||
            (wt == WidgetBase::wt_Class && umlScene()->isClassDiagram())) {

            showPropertiesDialog();

        } else if (wt == WidgetBase::wt_Object) {
            m_umlObject->showPropertiesDialog();
        } else {
            uWarning() << "making properties dialog for unknown widget type";
        }
        break;

    case ListPopupMenu::mt_Line_Color:
    case ListPopupMenu::mt_Line_Color_Selection:
#if QT_VERSION >= 0x050000
        newColor = QColorDialog::getColor(lineColor());
        if (newColor.isValid() && newColor != lineColor())
#else
        newColor = lineColor();
        if (KColorDialog::getColor(newColor))
#endif
        {
            if (sel == ListPopupMenu::mt_Line_Color_Selection) {
                umlScene()->selectionSetLineColor(newColor);
            } else {
                setLineColor(newColor);
            }
            setUsesDiagramLineColor(false);
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Fill_Color:
    case ListPopupMenu::mt_Fill_Color_Selection:
#if QT_VERSION >= 0x050000
        newColor = QColorDialog::getColor(fillColor());
        if (newColor.isValid() && newColor != fillColor())
#else
        newColor = fillColor();
        if (KColorDialog::getColor(newColor))
#endif
        {
            if (sel == ListPopupMenu::mt_Fill_Color_Selection) {
                umlScene()->selectionSetFillColor(newColor);
            } else {
                setFillColor(newColor);
            }
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Use_Fill_Color:
        setUseFillColor(!m_useFillColor);
        break;

    case ListPopupMenu::mt_Set_Use_Fill_Color_Selection:
        umlScene()->selectionUseFillColor(true);
        break;

    case ListPopupMenu::mt_Unset_Use_Fill_Color_Selection:
        umlScene()->selectionUseFillColor(false);
        break;

    case ListPopupMenu::mt_Show_Attributes_Selection:
    case ListPopupMenu::mt_Hide_Attributes_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowAttributes, sel != ListPopupMenu::mt_Hide_Attributes_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Operations_Selection:
    case ListPopupMenu::mt_Hide_Operations_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowOperations, sel != ListPopupMenu::mt_Hide_Operations_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Visibility_Selection:
    case ListPopupMenu::mt_Hide_Visibility_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowVisibility, sel != ListPopupMenu::mt_Hide_Visibility_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Operation_Signature_Selection:
    case ListPopupMenu::mt_Hide_Operation_Signature_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowOperationSignature, sel != ListPopupMenu::mt_Hide_Operation_Signature_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
    case ListPopupMenu::mt_Hide_Attribute_Signature_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowAttributeSignature, sel != ListPopupMenu::mt_Hide_Attribute_Signature_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Packages_Selection:
    case ListPopupMenu::mt_Hide_Packages_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowPackage, sel != ListPopupMenu::mt_Hide_Packages_Selection
        );
        break;

    case ListPopupMenu::mt_Show_Stereotypes_Selection:
    case ListPopupMenu::mt_Hide_Stereotypes_Selection:
        // Bug73847 - ClassifierWidget::ShowStereotype boolean value is DEPRECATED
        //     TODO - handle this differently, then delete ClassifierWidget::ShowStereotype
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowStereotype, sel != ListPopupMenu::mt_Hide_Stereotypes_Selection
        );
        break;

    case ListPopupMenu::mt_Hide_NonPublic_Selection:
    case ListPopupMenu::mt_Show_NonPublic_Selection:
        umlScene()->selectionSetVisualProperty(
            ClassifierWidget::ShowPublicOnly, sel != ListPopupMenu::mt_Show_NonPublic_Selection
        );
        break;


    case ListPopupMenu::mt_ViewCode: {
        UMLClassifier *c = umlObject()->asUMLClassifier();
        if (c) {
            UMLApp::app()->viewCodeDocument(c);
        }
        break;
    }

    case ListPopupMenu::mt_Remove:
        umlScene()->deleteSelection();
        break;

    case ListPopupMenu::mt_Delete:
        if (!Dialog_Utils::askDeleteAssociation())
            break;
        umlScene()->deleteSelection();
        break;

    case ListPopupMenu::mt_Change_Font:
    case ListPopupMenu::mt_Change_Font_Selection: {
#if QT_VERSION >= 0x050000
        bool ok = false;
        QFont newFont = QFontDialog::getFont(&ok, font());
        if (ok)
#else
        QFont newFont = font();
        if (KFontDialog::getFont(newFont, KFontChooser::NoDisplayFlags, 0) == KFontDialog::Accepted)
#endif
        {
            if (sel == ListPopupMenu::mt_Change_Font_Selection) {
                m_scene->selectionSetFont(newFont);
            } else {
                setFont(newFont);
            }
        }
    }
        break;

    case ListPopupMenu::mt_Cut:
        umlScene()->setStartedCut();
        UMLApp::app()->slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        UMLApp::app()->slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;

    case ListPopupMenu::mt_Refactoring:
        //check if we are operating on a classifier, or some other kind of UMLObject
        if (umlObject()->asUMLClassifier()) {
            UMLApp::app()->refactor(umlObject()->asUMLClassifier());
        }
        break;

     case ListPopupMenu::mt_Clone:
        {
            foreach (UMLWidget* widget, umlScene()->selectedWidgets()) {
                if (Model_Utils::isCloneable(widget->baseType())) {
                    UMLObject *clone = widget->umlObject()->clone();
                    umlScene()->addObject(clone);
                }
            }
        }
        break;

    case ListPopupMenu::mt_Rename_MultiA:
    case ListPopupMenu::mt_Rename_MultiB:
    case ListPopupMenu::mt_Rename_Name:
    case ListPopupMenu::mt_Rename_RoleAName:
    case ListPopupMenu::mt_Rename_RoleBName: {
        FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(this);
        ft->handleRename();
        break;
    }
    case ListPopupMenu::mt_Align_Right:
        umlScene()->alignRight();
        break;
    case ListPopupMenu::mt_Align_Left:
        umlScene()->alignLeft();
        break;
    case ListPopupMenu::mt_Align_Top:
        umlScene()->alignTop();
        break;
    case ListPopupMenu::mt_Align_Bottom:
        umlScene()->alignBottom();
        break;
    case ListPopupMenu::mt_Align_VerticalMiddle:
        umlScene()->alignVerticalMiddle();
        break;
    case ListPopupMenu::mt_Align_HorizontalMiddle:
        umlScene()->alignHorizontalMiddle();
        break;
    case ListPopupMenu::mt_Align_VerticalDistribute:
        umlScene()->alignVerticalDistribute();
        break;
    case ListPopupMenu::mt_Align_HorizontalDistribute:
        umlScene()->alignHorizontalDistribute();
        break;
    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
        break;
    }
}

/**
 * Helper function for debug output.
 * Returns the given enum value as string.
 * @param wt   WidgetType of which a string representation is wanted
 * @return   the WidgetType as string
 */
QString WidgetBase::toString(WidgetType wt)
{
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, wt));
}

/**
 * Returns the given enum value as localized string.
 * @param wt   WidgetType of which a string representation is wanted
 * @return   the WidgetType as localized string
 */
QString WidgetBase::toI18nString(WidgetType wt)
{
    QString name;

    switch (wt) {
    case wt_Activity:
        name = i18n("Activity");
        break;
    case wt_Actor:
        name = i18n("Actor");
        break;
    case wt_Artifact:
        name = i18n("Artifact");
        break;
    case wt_Association:
        name = i18n("Association");
        break;
    case wt_Box:
        name = i18n("Box");
        break;
    case wt_Category:
        name = i18n("Category");
        break;
    case wt_CombinedFragment:
        name = i18n("CombinedFragment");
        break;
    case wt_Component:
        name = i18n("Component");
        break;
    case wt_Class:
        name = i18n("Class");
        break;
    case wt_Datatype:
        name = i18n("Datatype");
        break;
    case wt_Entity:
        name = i18n("Entity");
        break;
    case wt_Enum:
        name = i18n("Enum");
        break;
    case wt_FloatingDashLine:
        name = i18n("FloatingDashLine");
        break;
    case wt_ForkJoin:
        name = i18n("ForkJoin");
        break;
    case wt_Interface:
        name = i18n("Interface");
        break;
    case wt_Message:
        name = i18n("Message");
        break;
    case wt_Node:
        name = i18n("Node");
        break;
    case wt_Note:
        name = i18n("Note");
        break;
    case wt_Object:
        name = i18n("Object");
        break;
    case wt_ObjectNode:
        name = i18n("ObjectNode");
        break;
    case wt_Package:
        name = i18n("Package");
        break;
    case wt_Pin:
        name = i18n("Pin");
        break;
    case wt_Port:
        name = i18n("Port");
        break;
    case wt_Precondition:
        name = i18n("Precondition");
        break;
    case wt_Region:
        name = i18n("Region");
        break;
    case wt_Signal:
        name = i18n("Signal");
        break;
    case wt_State:
        name = i18n("State");
        break;
    case wt_Text:
        name = i18n("Text");
        break;
    case wt_UseCase:
        name = i18n("UseCase");
        break;
    case wt_Instance:
        name = i18n("Instance");
        break;
    default:
        name = QLatin1String("<unknown> &name:");
        uWarning() << "unknown widget type";
        break;
    }
    return name;
}

/**
 * Returns the given enum value as icon type.
 * @param wt   WidgetType of which an icon type representation is wanted
 * @return   the WidgetType as icon type
 */
Icon_Utils::IconType WidgetBase::toIcon(WidgetBase::WidgetType wt)
{
    Icon_Utils::IconType icon;

    switch (wt) {
    case wt_Activity:
        icon = Icon_Utils::it_Activity;
        break;
    case wt_Actor:
        icon = Icon_Utils::it_Actor;
        break;
    case wt_Artifact:
        icon = Icon_Utils::it_Artifact;
        break;
    case wt_Association:
        icon = Icon_Utils::it_Association;
        break;
    case wt_Box:
        icon = Icon_Utils::it_Box;
        break;
    case wt_Category:
        icon = Icon_Utils::it_Category;
        break;
    case wt_CombinedFragment:
        icon = Icon_Utils::it_Combined_Fragment;
        break;
    case wt_Component:
        icon = Icon_Utils::it_Component;
        break;
    case wt_Class:
        icon = Icon_Utils::it_Class;
        break;
    case wt_Datatype:
        icon = Icon_Utils::it_Datatype;
        break;
    case wt_Entity:
        icon = Icon_Utils::it_Entity;
        break;
    case wt_Enum:
        icon = Icon_Utils::it_Enum;
        break;
    case wt_FloatingDashLine:
        icon = Icon_Utils::it_Association;
        break;
    case wt_ForkJoin:
        icon = Icon_Utils::it_Fork_Join;
        break;
    case wt_Instance:
        icon = Icon_Utils::it_Instance;
        break;
    case wt_Interface:
        icon = Icon_Utils::it_Interface;
        break;
    case wt_Message:
        icon = Icon_Utils::it_Message_Synchronous;
        break;
    case wt_Node:
        icon = Icon_Utils::it_Node;
        break;
    case wt_Note:
        icon = Icon_Utils::it_Note;
        break;
    case wt_Object:
        icon = Icon_Utils::it_Object;
        break;
    case wt_ObjectNode:
        icon = Icon_Utils::it_Object_Node;
        break;
    case wt_Package:
        icon = Icon_Utils::it_Package;
        break;
    case wt_Pin:
        icon = Icon_Utils::it_Pin;
        break;
    case wt_Port:
        icon = Icon_Utils::it_Port;
        break;
    case wt_Precondition:
        icon = Icon_Utils::it_Precondition;
        break;
    case wt_Region:
        icon = Icon_Utils::it_Region;
        break;
    case wt_Signal:
        icon = Icon_Utils::it_Send_Signal;
        break;
    case wt_State:
        icon = Icon_Utils::it_State;
        break;
    case wt_Text:
        icon = Icon_Utils::it_Text;
        break;
    case wt_UseCase:
        icon = Icon_Utils::it_UseCase;
        break;
    default:
        icon = Icon_Utils::it_Home;
        uWarning() << "unknown widget type";
        break;
    }
    return icon;
}

#include  "activitywidget.h"
#include  "actorwidget.h"
#include  "artifactwidget.h"
#include  "associationwidget.h"
#include  "boxwidget.h"
#include  "categorywidget.h"
//#include  "classwidget.h"
#include  "combinedfragmentwidget.h"
#include  "componentwidget.h"
#include  "datatypewidget.h"
#include  "entitywidget.h"
#include  "enumwidget.h"
#include  "floatingdashlinewidget.h"
#include  "forkjoinwidget.h"
#include  "interfacewidget.h"
#include  "messagewidget.h"
#include  "nodewidget.h"
#include  "notewidget.h"
#include  "objectnodewidget.h"
#include  "objectwidget.h"
#include  "packagewidget.h"
#include  "pinwidget.h"
#include  "portwidget.h"
#include  "preconditionwidget.h"
#include  "regionwidget.h"
#include  "signalwidget.h"
#include  "statewidget.h"
#include  "usecasewidget.h"

ActivityWidget* WidgetBase::asActivityWidget() { return dynamic_cast<ActivityWidget* >(this); }
ActorWidget* WidgetBase::asActorWidget() { return dynamic_cast<ActorWidget* >(this); }
ArtifactWidget* WidgetBase::asArtifactWidget() { return dynamic_cast<ArtifactWidget* >(this); }
AssociationWidget* WidgetBase::asAssociationWidget() { return dynamic_cast<AssociationWidget* >(this); }
BoxWidget* WidgetBase::asBoxWidget() { return dynamic_cast<BoxWidget* >(this); }
CategoryWidget* WidgetBase::asCategoryWidget() { return dynamic_cast<CategoryWidget* >(this); }
ClassifierWidget* WidgetBase::asClassifierWidget() { return dynamic_cast<ClassifierWidget* >(this); }
CombinedFragmentWidget* WidgetBase::asCombinedFragmentWidget() { return dynamic_cast<CombinedFragmentWidget*>(this); }
ComponentWidget* WidgetBase::asComponentWidget() { return dynamic_cast<ComponentWidget* >(this); }
DatatypeWidget* WidgetBase::asDatatypeWidget() { return dynamic_cast<DatatypeWidget* >(this); }
EntityWidget* WidgetBase::asEntityWidget() { return dynamic_cast<EntityWidget* >(this); }
EnumWidget* WidgetBase::asEnumWidget() { return dynamic_cast<EnumWidget* >(this); }
FloatingDashLineWidget* WidgetBase::asFloatingDashLineWidget() { return dynamic_cast<FloatingDashLineWidget*>(this); }
ForkJoinWidget* WidgetBase::asForkJoinWidget() { return dynamic_cast<ForkJoinWidget* >(this); }
InterfaceWidget* WidgetBase::asInterfaceWidget() { return dynamic_cast<InterfaceWidget* >(this); }
MessageWidget* WidgetBase::asMessageWidget() { return dynamic_cast<MessageWidget* >(this); }
NodeWidget* WidgetBase::asNodeWidget() { return dynamic_cast<NodeWidget* >(this); }
NoteWidget* WidgetBase::asNoteWidget() { return dynamic_cast<NoteWidget* >(this); }
ObjectNodeWidget* WidgetBase::asObjectNodeWidget() { return dynamic_cast<ObjectNodeWidget* >(this); }
ObjectWidget* WidgetBase::asObjectWidget() { return dynamic_cast<ObjectWidget* >(this); }
PackageWidget* WidgetBase::asPackageWidget() { return dynamic_cast<PackageWidget* >(this); }
PinWidget* WidgetBase::asPinWidget() { return dynamic_cast<PinWidget* >(this); }
PinPortBase *WidgetBase::asPinPortBase() { return dynamic_cast<PinPortBase*>(this); }
PortWidget* WidgetBase::asPortWidget() { return dynamic_cast<PortWidget* >(this); }
PreconditionWidget* WidgetBase::asPreconditionWidget() { return dynamic_cast<PreconditionWidget* >(this); }
RegionWidget* WidgetBase::asRegionWidget() { return dynamic_cast<RegionWidget* >(this); }
SignalWidget* WidgetBase::asSignalWidget() { return dynamic_cast<SignalWidget* >(this); }
StateWidget* WidgetBase::asStateWidget() { return dynamic_cast<StateWidget* >(this); }
FloatingTextWidget* WidgetBase::asFloatingTextWidget() { return dynamic_cast<FloatingTextWidget* >(this); }
//TextWidget* WidgetBase::asTextWidget() { return dynamic_cast<TextWidget* >(this); }
UseCaseWidget* WidgetBase::asUseCaseWidget() { return dynamic_cast<UseCaseWidget* >(this); }
UMLWidget *WidgetBase::asUMLWidget() { return dynamic_cast<UMLWidget*>(this); }
