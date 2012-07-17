/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "widgetbase.h"

#include "classifier.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "listpopupmenu.h"
#include "umlwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"
#include "uniqueid.h"
#include "widget_utils.h"

#include <kfontdialog.h>
#include <kcolordialog.h>

#include <QPointer>
#include <QTimer>

////////////////////////////////////////////////
static void setupAwesomeBrush(QBrush &brush)
{
    QLinearGradient grad;
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, Qt::white);
    // grad.setColorAt(1, QColor("#ffda0c"));
    const int grayLevel = 220;
    const int alpha = 255;
    QColor gray(grayLevel, grayLevel, grayLevel, alpha);
    grad.setColorAt(1, gray);

    grad.setStart(0, 0);
    grad.setFinalStop(1, 1);

    brush = QBrush(grad);
}

static QBrush awesomeBrush()
{
    static QBrush brush;
    if (!brush.gradient()) {
        setupAwesomeBrush(brush);
    }
    return brush;
}

static QColor awesomeLineColor()
{
    return QColor(80, 80, 0);
}
////////////////////////////////////////////////

/**
 * @short A class to encapsulate some properties to be stored as a
 * part of Lazy initialization.
 *
 * The properties encapsulated here are used only if the widget does
 * not have UMLObject representation.
 */
class WidgetInterfaceData
{
public:
    WidgetInterfaceData() : id(Uml::id_None)
    {
    }

    Uml::IDType id;
    QString documentation;
    QString name;
};

/**
 * Constructs a WidgetBase object with the associated UMLObject to
 * \a object and the parent item of the widget being null.  Also sets
 * the ItemIsMovable and ItemIsSelectable flag.
 *
 * @param object UMLObject that is represented by this widget. Pass
 *               null if this widget does not have UMLObject
 *               representation.
 *
 * @note The widget's visual properties are best to be set by the
 *       Widget_Factory::createWidget method.
 */
WidgetBase::WidgetBase(WidgetType type, UMLObject *object)
  : QGraphicsObject(),
    m_baseType(type),
    m_umlObject(object),
    m_lineColor(awesomeLineColor()),
    m_lineWidth(0),
    m_brush(awesomeBrush()),
    m_activated(false),
    m_widgetInterfaceData(0),
    m_usesDiagramLineColor(true),
    m_usesDiagramLineWidth(true),
    m_usesDiagramBrush(true),
    m_usesDiagramFont(true),
    m_usesDiagramTextColor(true)
{
    if(!object) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }
    setFlags(ItemIsSelectable | ItemIsMovable |ItemSendsGeometryChanges);
    hide(); // Show up in activate
}

/**
 * Destructor
 */
WidgetBase::~WidgetBase()
{
    delete m_widgetInterfaceData;
}

/**
 * @retval The UMLObject represented by this widget
 * @retval null if no UMLObject representation.
 */
UMLObject* WidgetBase::umlObject() const
{
    return m_umlObject;
}

/**
 * Set the UMLObject for this widget to represent.
 *
 * @param notifyAsSlot If true, slotUMLObjectDataChanged() is invoked as a slot
 *                     with the aid of QTimer::singleShot.
 *                     This helps in virtual function calls to function appropriately
 *                     in case this method was called from constructor.
 *
 * @todo Either remove this method, or allow it to only allow specific
 *       users as making this method public violates encapsulation.
 */
void WidgetBase::setUMLObject(UMLObject *obj, bool notifyAsSlot)
{
    UMLObject *oldObj = m_umlObject;
    m_umlObject = obj;

    if(oldObj) {
        oldObj->disconnect(this);
    }

    if(m_umlObject) {
        delete m_widgetInterfaceData;
        m_widgetInterfaceData = 0;
        connect(umlObject(), SIGNAL(modified()), this,
                SLOT(slotUMLObjectDataChanged()));
    }
    else if(!m_widgetInterfaceData) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }

    umlObjectChanged(oldObj);
    if (notifyAsSlot) {
        QTimer::singleShot(0, this, SLOT(slotUMLObjectDataChanged()));
    } else {
        slotUMLObjectDataChanged();
    }
}

/**
 * If this widget represents a UMLObject, the id of that object is
 * returned. Else the id stored in this widget is returned.
 *
 * @return The identifier of this widget.
 */
Uml::IDType WidgetBase::id() const
{
    if(m_umlObject) {
        return m_umlObject->id();
    }
    return m_widgetInterfaceData->id;
}

/**
 * If this widget represents a UMLObject, the id of that object is set
 * to \a id. Else the id is stored in this widget.
 *
 * This method issues a @ref IDHasChanged notification after setting
 * the id.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setID(Uml::IDType id)
{
    const Uml::IDType oldId = this->id();

    if (id == Uml::id_None) {
        // generate unique id in case of None.
        id = UniqueID::gen();
    }

    if(m_umlObject) {
        if(m_umlObject->id() != Uml::id_None) {
            uWarning() << "changing old UMLObject " << ID2STR(m_umlObject->id())
                       << " to " << ID2STR(id);
        }

        m_umlObject->setID(id);
    }
    else {
        m_widgetInterfaceData->id = id;
    }

    attributeChange(IDHasChanged, ID2STR(oldId));
}

/**
 * @return The type used for rtti.
 */
WidgetBase::WidgetType WidgetBase::baseType() const
{
    return m_baseType;
}

/**
 * @return The type used for rtti as string.
 */
QLatin1String WidgetBase::baseTypeStr() const
{
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, m_baseType));
}

/**
 * @return The UMLScene for this widget is returned, or 0 if the
 *         widget is not stored in a scene.
 *
 * @note To add or remove widgets to scene, use UMLScene::addItem
 */
UMLScene* WidgetBase::umlScene() const
{
    return qobject_cast<UMLScene*>(this->scene());
}

/**
 * This is shortcut method for UMLApp::app()->document()
 *
 * @return Pointer to the UMLDoc object.
 */
UMLDoc* WidgetBase::umlDoc() const
{
    return UMLApp::app()->document();
}

/**
 * If this widget represents a UMLObject, the documentation string
 * stored in the object is returned. Else the documentation string
 * stored in the widget is returned.
 *
 * @return A string representing the documentation for this widget
 *         which is usually set by the user.
 */
QString WidgetBase::documentation() const
{
    if(m_umlObject) {
        return m_umlObject->doc();
    }
    return m_widgetInterfaceData->documentation;
}

/**
 * If this widget represents a UMLObject, the documentation string
 * of that object is set to \a doc. Else the documentation string
 * stored in the widget is set to \a doc.
 *
 * This method issues a @ref DocumentationHasChanged notification
 * after setting the new documentation.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setDocumentation(const QString& doc)
{
    const QString oldDoc = documentation();

    if(m_umlObject) {
        m_umlObject->setDoc(doc);
    }
    else {
        m_widgetInterfaceData->documentation = doc;
    }

    attributeChange(DocumentationHasChanged, oldDoc);
}

/**
 * If this widget represents a UMLObject, the name string stored
 * in the object is returned. Else the name string stored in the
 * widget is returned.
 *
 * @return A string representing the name for this widget
 *         which is usually set by the user.
 */
QString WidgetBase::name() const
{
    if(m_umlObject) {
        return m_umlObject->name();
    }
    return m_widgetInterfaceData->name;
}

/**
 * If this widget represents a UMLObject, the name string of that
 * object is set to \a name. Else the name string stored in the
 * widget is set to \a name.
 *
 * This method issues a @ref NameHasChanged notification after setting
 * the new name.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setName(const QString& name)
{
    const QString oldName = this->name();
    if(m_umlObject) {
        m_umlObject->setName(name);
    }
    else {
        m_widgetInterfaceData->name = name;
    }

    attributeChange(NameHasChanged, oldName);
}

/// @return The color used to draw lines of the widget.
QColor WidgetBase::lineColor() const
{
    return m_lineColor;
}

/**
 * Set the linecolor to \a color and updates the widget.
 * @param color The color to be set
 *
 * This method issues a @ref LineColorHasChanged notification after
 * setting the new line color.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setLineColor(const QColor& color)
{
    const QColor oldColor = lineColor();
    m_lineColor = color;
    if(!m_lineColor.isValid()) {
        uWarning() << "Invalid color";
        m_lineColor = Qt::black;
    }

    attributeChange(LineColorHasChanged, oldColor);
}

/// @return The width of the lines drawn in the widget.
uint WidgetBase::lineWidth() const
{
    return m_lineWidth;
}

/**
 * Sets the line width of widget lines to \a lw and calls
 * updateGeometry() method to let object calculate new bound rect
 * based on the new line width.
 *
 * @param lw  The width of line to be set.
 *
 * This method issues @ref LineWidthHasChanged notification after
 * setting new line width.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setLineWidth(uint lw)
{
    const qreal oldWidth = lineWidth();
    m_lineWidth = lw;

    attributeChange(LineWidthHasChanged, oldWidth);
}

/// @return Font color used to draw font.
QColor WidgetBase::textColor() const
{
    return m_textColor;
}

/**
 * Sets the color of the font to \a color.
 * If \a color is invalid, black is used instead.
 *
 * This method issues @ref TextColorHasChanged notification after
 * setting the new font color.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setTextColor(const QColor& color)
{
    const QColor oldColor = textColor();
    m_textColor = color;
    if(!m_textColor.isValid()) {
        m_textColor = Qt::black;
    }

    attributeChange(TextColorHasChanged, oldColor);
}

/// @return The QBrush object used to fill this widget.
QBrush WidgetBase::brush() const
{
    return m_brush;
}

/**
 * Sets the QBrush object of this widget to \a brush which is used to
 * fill this widget.
 *
 * This method issues @ref BrushHasChanged notification after setting
 * the new brush.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setBrush(const QBrush& brush)
{
    const QBrush oldBrush = this->brush();
    m_brush = brush;

    attributeChange(BrushHasChanged, oldBrush);
}

bool WidgetBase::usesDiagramLineColor() const
{
    return m_usesDiagramLineColor;
}

void WidgetBase::setUsesDiagramLineColor(bool state)
{
    if (m_usesDiagramLineColor == state) {
        return;
    }
    m_usesDiagramLineColor = state;
    if (state && umlScene()) {
        QColor color = umlScene()->lineColor();
        if (color != lineColor()) {
            setLineColor(color);
        }
    }
}

bool WidgetBase::usesDiagramLineWidth() const
{
    return m_usesDiagramLineWidth;
}

void WidgetBase::setUsesDiagramLineWidth(bool state)
{
    if (m_usesDiagramLineWidth == state) {
        return;
    }
    m_usesDiagramLineWidth = state;
    if (state && umlScene()) {
        uint lw = umlScene()->lineWidth();
        if (lw != lineWidth()) {
            setLineWidth(lw);
        }
    }
}

bool WidgetBase::usesDiagramBrush() const
{
    return m_usesDiagramBrush;
}

void WidgetBase::setUsesDiagramBrush(bool state)
{
    if (state == m_usesDiagramBrush) {
        return;
    }
    m_usesDiagramBrush = state;
    if (state && umlScene()) {
        QBrush b = umlScene()->brush();
        if (b != brush()) {
            setBrush(b);
        }
    }
}

bool WidgetBase::usesDiagramFont() const
{
    return m_usesDiagramFont;
}

void WidgetBase::setUsesDiagramFont(bool state)
{
    if (state == m_usesDiagramFont) {
        return;
    }
    m_usesDiagramFont = state;
    if (state && umlScene()) {
        QFont f = umlScene()->font();
        if (f != font()) {
            setFont(f);
        }
    }
}

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
    if (state && umlScene()) {
        QColor color = umlScene()->textColor();
        if (color != textColor()) {
            setTextColor(color);
        }
    }
}

/// @return The font used for displaying any text
QFont WidgetBase::font() const
{
    return m_font;
}

/**
 * Set the font used to display text inside this widget.
 *
 * This method issues @ref FontHasChanged notification after setting
 * the new font.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setFont(const QFont& font)
{
    const QFont oldFont = this->font();
    m_font = font;

    attributeChange(FontHasChanged, oldFont);
}

/**
 * @return Activation status.
 * @see WidgetBase::m_activated to understand what activation means.
 */
bool WidgetBase::isActivated() const
{
    return m_activated;
}

/**
 * Sets only the activation flag, without invoking activate() method.
 * @see WidgetBase::m_activated to understand what activation means.
 */
void WidgetBase::setActivatedFlag(bool b)
{
    m_activated = b;
    if (b) {
        m_loadData.clear();
    }
}

/**
 * This virtual method should be reimplementd by subclasses to attempt complete
 * construction, with assumption that sub components have been constructed already.
 *
 * The flag, m_activated should be set to true/false on success/failure.
 *
 * By default, this method just sets the flag to true.
 *
 * @return The new activation status.
 *
 * @see WidgetBase::m_activated to understand what activation means.
 * @sa WidgetBase::setActivatedFlag
 */
bool WidgetBase::activate()
{
    UMLScene *scene = umlScene();
    Q_ASSERT(scene);
    // Reset flag, because you might be "reactivating" this widget in which case
    // the new activation status needs to be held.
    setActivatedFlag(false);

    if (!m_loadData.isEmpty()) {
        m_usesDiagramLineColor =
            m_loadData.value("usesDiagramLineColor", true).toBool();
        QColor lineColor = scene->lineColor();
        if (!m_usesDiagramLineColor) {
            lineColor = m_loadData.value("lineColor").value<QColor>();
        }
        setLineColor(lineColor);

        m_usesDiagramLineWidth =
            m_loadData.value("usesDiagramLineWidth", true).toBool();
        uint lineWidth = scene->lineWidth();
        if (!m_usesDiagramLineWidth) {
            lineWidth = m_loadData.value("lineWidth").toUInt();
        }
        setLineWidth(lineWidth);

        m_usesDiagramTextColor =
            m_loadData.value("usesDiagramTextColor", true).toBool();
        QColor textColor = scene->textColor();
        if (!m_usesDiagramTextColor) {
            textColor = m_loadData.value("textColor").value<QColor>();
        }
        setTextColor(textColor);

        m_usesDiagramBrush = m_loadData.value("usesDiagramBrush", true).toBool();
        QBrush brush = scene->brush();
        if (!m_usesDiagramBrush) {
            brush = m_loadData.value("brush").value<QBrush>();
        }
        setBrush(brush);

        m_usesDiagramFont = m_loadData.value("usesDiagramFont", true).toBool();
        QFont font = scene->font();
        if (!m_usesDiagramFont) {
            font = m_loadData.value("font").value<QFont>();
        }
        setFont(font);

        QPointF pos = m_loadData.value("pos").toPointF();
        setPos(mapToParent(mapFromScene(pos)));
    } else {
        // To ensure even the children of Widgets if any are appropriately
        // initialized.
        QVariant v;
        attributeChange(LineColorHasChanged, v);
        attributeChange(LineWidthHasChanged, v);
        attributeChange(FontHasChanged, v);
        attributeChange(TextColorHasChanged, v);
        attributeChange(BrushHasChanged, v);
    }

    setVisible(true);
    setUMLObject(m_umlObject);

    setActivatedFlag(true);
    return isActivated();
}

bool WidgetBase::userChange(UserChangeType c) const
{
    return m_userChange.testFlag(c);
}

WidgetBase::UserChange WidgetBase::userChanges() const
{
    return m_userChange;
}

void WidgetBase::setUserChange(UserChangeType c, bool value)
{
    if (value) {
        m_userChange = m_userChange & c;
    } else {
        m_userChange = m_userChange & ~c;
    }
}

/**
 * A virtual method for the widget to display a property dialog box.
 * Subclasses should reimplment this appropriately.
 */
void WidgetBase::showPropertiesDialog()
{
}

/**
 * A virtual method to load the properties of this widget from a
 * QDomElement into this widget.
 *
 * Subclasses should reimplement this to load addtional properties
 * required, calling this base method to load the basic properties of
 * the widget.
 *
 * @param qElement A QDomElement which contains xml info for this
 *                 widget.
 *
 * @todo Add support to load older version.
 */
bool WidgetBase::loadFromXMI(QDomElement &qElement)
{
    const QLatin1String none("none");
    // Start filling m_loadData afresh
    m_loadData.clear();

    // Load the line color first

    // Workaround for old "linecolour" usage.
    QString lineColor = qElement.attribute("linecolour", none);
    lineColor = qElement.attribute("linecolor", lineColor);
    if (lineColor != none) {
        m_loadData.insert("usesDiagramLineColor", false);
        m_loadData.insert("lineColor", QColor(lineColor));
    } else {
        m_loadData.insert("usesDiagramLineColor", true);
    }

    // Load the line width.
    QString lineWidth = qElement.attribute("linewidth", none);
    if(lineWidth != none) {
        m_loadData.insert("usesDiagramLineWidth", false);
        m_loadData.insert("lineWidth", uint(lineWidth.toUInt()));
    }
    else {
        m_loadData.insert("usesDiagramLineWidth", true);
    }

    // Load the font color, if invalid black is used.
    QString textColor = qElement.attribute("textcolor", none);
    if (textColor != none) {
        m_loadData.insert("usesDiagramTextColor", false);
        m_loadData.insert("textColor", QColor(textColor));
    } else {
        m_loadData.insert("usesDiagramTextColor", true);
    }

    // Load the brush.
    QDomElement brushElement = qElement.firstChildElement("brush");
    int usesDiagramBrush = qElement.attribute("usesdiagrambrush", "-1").toInt();
    if (!brushElement.isNull() || usesDiagramBrush != -1) {
        if (usesDiagramBrush == 0 || brushElement.isNull()) {
            m_loadData.insert("usesDiagramBrush", true);
        } else {
            QBrush newBrush;
            bool brushSet = Widget_Utils::loadBrushFromXMI(brushElement, newBrush);
            if (!brushSet) {
                uError() << "Could not load brush from XMI file";
                m_loadData.insert("usesDiagramBrush", true);
            } else {
                m_loadData.insert("usesDiagramBrush", false);
                m_loadData.insert("brush", newBrush);
            }
        }
    } else {
        // If control is here,
        // we are loading a file saved using older version mostly.

        bool useFillColor = false;
        QString usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolour", "1");
        usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolor", usesDiagramUseFillColor);
        if (usesDiagramUseFillColor.toInt()) {
            QBrush brsh;  //:TODO: = QBrush(umlScene()->brush()); 
            useFillColor = (brsh.style() != Qt::NoBrush);
        } else {
            useFillColor = (qElement.attribute("usefillcolor", "1").toInt() != 0);
        }

        if (useFillColor == false) {
            m_loadData.insert("usesDiagramBrush", false);
            m_loadData.insert("brush", QBrush(Qt::NoBrush));
        } else {
            QString fillColor = qElement.attribute("fillcolour", "none");
            fillColor = qElement.attribute("fillcolor", fillColor);
            if (fillColor != none) {
                m_loadData.insert("usesDiagramBrush", false);
                m_loadData.insert("brush", QBrush(QColor(fillColor)));
            } else {
                m_loadData.insert("usesDiagramBrush", true);
            }
        }
    }

    // Load the font.
    QString font = qElement.attribute(QLatin1String("font"));
    bool fontSet = false;
    if(!font.isEmpty()) {
        QFont fnt;
        fontSet = fnt.fromString(font);
        if(fontSet) {
            m_loadData.insert("usesDiagramFont", false);
            m_loadData.insert("font", fnt);
        } else {
            uWarning() << "Loading font attribute->" << font << " failed";
        }
    }
    // Set diagram's default font if font is not yet set.
    if(!fontSet) {
        m_loadData.insert("usesDiagramFont", true);
    }

    QPointF pos;
    pos.setX(qElement.attribute("x", "0").toDouble());
    pos.setY(qElement.attribute("y", "0").toDouble());
    m_loadData.insert("pos", pos);

    return true;
}

/**
 * A virtual method to save the properties of this widget into a
 * QDomElement i.e xml.
 *
 * Subclasses should first create a new dedicated element as the child
 * of \a qElement parameter passed.  Then this base method should be
 * called to save basic widget properties.
 *
 * @param qDoc A QDomDocument object representing the xml document.
 * @oaram qElement A QDomElement representing xml element data.
 */
void WidgetBase::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    Q_ASSERT(umlScene());
    if (m_usesDiagramLineColor) {
        qElement.setAttribute("linecolor", "none");
    } else {
        qElement.setAttribute("linecolor", m_lineColor.name());
    }

    if (m_usesDiagramLineWidth) {
        qElement.setAttribute("linewidth", m_lineWidth);
    } else {
        qElement.setAttribute("linewidth", m_lineWidth);
    }

    if (m_usesDiagramTextColor) {
        qElement.setAttribute("textcolor", "none");
    } else {
        qElement.setAttribute("textcolor", m_textColor.name());
    }

    if (m_usesDiagramFont) {
        // do not save font attribute, which would result in default font
        // being set while loading.
    } else {
        qElement.setAttribute("font", m_font.toString());
    }

    if (m_usesDiagramBrush) {
        qElement.setAttribute("usesdiagrambrush", true);
    } else {
        qElement.setAttribute("usesdiagrambrush", false);
        Widget_Utils::saveBrushToXMI(qDoc, qElement, m_brush);
    }

    const QPointF pos = scenePos();
    qElement.setAttribute("x", qRound(pos.x()));
    qElement.setAttribute("y", qRound(pos.y()));
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
 * @return Whether the widget type has an associated UMLObject
 */
bool WidgetBase::widgetHasUMLObject(WidgetType type)
{
    switch(type)
    {
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
    case WidgetBase::wt_Class:
    case WidgetBase::wt_Interface:
    case WidgetBase::wt_Enum:
    case WidgetBase::wt_Datatype:
    case WidgetBase::wt_Package:
    case WidgetBase::wt_Component:
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
    case WidgetBase::wt_Object:
        return true;
    default:
        return false;
    }
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
    QColor newColour;
    WidgetBase* widget = 0; // use for select the first object properties (fill, line color)

    const WidgetType wt = m_baseType; // short hand name

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(trigger);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    ListPopupMenu::MenuType sel = menu->getMenuType(trigger);
    switch (sel) {
    case ListPopupMenu::mt_Rename:
        umlDoc()->renameUMLObject(umlObject());
        break;

    //case ListPopupMenu::mt_Delete:  // is done in UMLWidget
    //    umlScene()->removeWidget(this);
    //    break;

    case ListPopupMenu::mt_Properties:
        if (wt == WidgetBase::wt_Actor     || wt == WidgetBase::wt_UseCase   ||
            wt == WidgetBase::wt_Package   || wt == WidgetBase::wt_Interface ||
            wt == WidgetBase::wt_Datatype  || wt == WidgetBase::wt_Node      ||
            wt == WidgetBase::wt_Component || wt == WidgetBase::wt_Artifact  ||
            wt == WidgetBase::wt_Enum      || wt == WidgetBase::wt_Entity    ||
            (wt == WidgetBase::wt_Class && umlScene()->type() == Uml::DiagramType::Class)) {

            showPropertiesDialog();

        } else if (wt == WidgetBase::wt_Object) {
            m_umlObject->showPropertiesPagedDialog();
        } else {
            uWarning() << "making properties dialog for unknown widget type";
        }
        break;

    case ListPopupMenu::mt_Line_Color:
        widget = umlScene()->getFirstMultiSelectedWidget();
        if (widget) {
            newColour = widget->lineColor();
        }
        if (KColorDialog::getColor(newColour)) {
            umlScene()->selectionSetLineColor(newColour);
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Fill_Color:
        widget = umlScene()->getFirstMultiSelectedWidget();
        if (widget) {
            newColour = widget->brush().color();
        }
        if (KColorDialog::getColor(newColour)) {
            umlScene()->selectionSetFillColor(newColour);
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Use_Fill_Color:
        // m_bUseFillColour = !m_bUseFillColour;
        // m_bUsesDiagramUseFillColour = false;
        // m_pView->selectionUseFillColor(m_bUseFillColour);
        break;

    case ListPopupMenu::mt_Show_Attributes_Selection:
    case ListPopupMenu::mt_Show_Operations_Selection:
    case ListPopupMenu::mt_Visibility_Selection:
    case ListPopupMenu::mt_DrawAsCircle_Selection:
    case ListPopupMenu::mt_Show_Operation_Signature_Selection:
    case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
    case ListPopupMenu::mt_Show_Packages_Selection:
    case ListPopupMenu::mt_Show_Stereotypes_Selection:
    case ListPopupMenu::mt_Show_Public_Only_Selection:
        umlScene()->selectionToggleShow(sel);
        umlDoc()->setModified(true);
        break;

    case ListPopupMenu::mt_ViewCode: {
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(umlObject());
        if (c) {
            UMLApp::app()->viewCodeDocument(c);
        }
        break;
    }

    case ListPopupMenu::mt_Delete_Selection:
        umlScene()->deleteSelection();
        break;

    case ListPopupMenu::mt_Change_Font:
    case ListPopupMenu::mt_Change_Font_Selection: {
        QFont newFont = font();
        if (KFontDialog::getFont(newFont, KFontChooser::NoDisplayFlags, 0) == KFontDialog::Accepted) {
            setFont(newFont);
            //UMLApp::app()->executeCommand(new CmdChangeFontSelection(m_doc, m_pView, font));
        }
    }
        break;

    // case ListPopupMenu::mt_Cut:
    //     m_pView -> setStartedCut();
    //     UMLApp::app() -> slotEditCut();
    //     break;

    // case ListPopupMenu::mt_Copy:
    //     UMLApp::app() -> slotEditCopy();
    //     break;

    // case ListPopupMenu::mt_Paste:
    //     UMLApp::app() -> slotEditPaste();
    //     break;

    case ListPopupMenu::mt_Refactoring:
        //check if we are operating on a classifier, or some other kind of UMLObject
        if (dynamic_cast<UMLClassifier*>(umlObject())) {
            UMLApp::app()->refactor(static_cast<UMLClassifier*>(umlObject()));
        }
        break;

    // case ListPopupMenu::mt_Clone:
    //     // In principle we clone all the uml objects.
    // {
    //     UMLObject *pClone = m_pObject->clone();
    //     m_pView->addObject(pClone);
    // }
    // break;

    case ListPopupMenu::mt_Rename_MultiA:
    case ListPopupMenu::mt_Rename_MultiB:
    case ListPopupMenu::mt_Rename_Name:
    case ListPopupMenu::mt_Rename_RoleAName:
    case ListPopupMenu::mt_Rename_RoleBName: {
        FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(this);
        ft->handleRename();
        break;
    }

    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
        break;
    }
}

/**
 * This slot is connected to UMLObject::modified() signal to sync the
 * required changes as well as to update visual aspects corresponding
 * to the change.
 *
 * @note Subclasses can *reimplement* this slot to fine tune its own
 *       updations.
 */
void WidgetBase::slotUMLObjectDataChanged()
{
    updateGeometry();
}

/**
 * Reimplemented to show appropriate context menu.
 */
void WidgetBase::contextMenuEvent(UMLSceneContextMenuEvent *event)
{
    event->accept();

    UMLScene *scene = umlScene();
    if (!isSelected() && scene && !scene->selectedItems().isEmpty()) {
        Qt::KeyboardModifiers forSelection = (Qt::ControlModifier | Qt::ShiftModifier);
        if ((event->modifiers() & forSelection) == 0) {
            scene->clearSelection();
        }
    }
    setSelected(true);
    QPointer<ListPopupMenu> menu = new ListPopupMenu(0, this, false, false);
    setupContextMenuActions(*(menu.data()));
    QAction *triggered = menu->exec(event->screenPos());
    ListPopupMenu *parentMenu = ListPopupMenu::menuFromAction(triggered);

    if (!parentMenu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    WidgetBase *ownerWidget = parentMenu->ownerWidget();
    // assert because logic is based on only WidgetBase being the owner of 
    // ListPopupMenu actions executed in this context menu.
    Q_ASSERT_X(ownerWidget != 0, "WidgetBase::contextMenuEvent",
            "ownerWidget is null which means action belonging to UMLView, UMLScene"
            " or UMLObject is the one triggered in ListPopupMenu");

    ownerWidget->slotMenuSelection(triggered);

    delete menu.data();
}

void WidgetBase::setupContextMenuActions(ListPopupMenu &menu)
{
    Q_UNUSED(menu);
}

/**
 * This virtual method is provided to notify self and subclasses about
 * change in some attribute.
 * This is modelled on similar lines to that of QGraphicsItem::itemChange.
 *
 * @param change The attribute which changed.
 * @param oldValue The oldValue if needed can be used in subclasses.
 *
 * @return Currently only QVariant() (provided now for futuristic approach)
 */
QVariant WidgetBase::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    Q_UNUSED(oldValue);

    switch(change) {
    case FontHasChanged:
    case NameHasChanged:
    case LineWidthHasChanged:
        updateGeometry();
        break;

    case LineColorHasChanged:
    case TextColorHasChanged:
    case BrushHasChanged:
        update();
        break;

    default:
        break;
    }
    return QVariant();
}

/**
 * Do some initialization on first scene change.
 * @note The virtual function call is legitimate here since scene->addItem() is always
 *       invoked after item has been fully constructed.
 */
QVariant WidgetBase::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange) {
        m_itemPositionChangePos = pos();
        if (userChange(PositionChange)) {
            QPointF pt = value.toPointF();
            pt = QPointF(qMax(pt.x(), qreal(0)), qMax(pt.y(), qreal(0)));
            return QGraphicsObject::itemChange(change, pt);
        }
    }

    if (change == ItemPositionHasChanged) {
        if (userChange(PositionChange)) {
            // TODO: Push move undo command

        }
    }

    return QGraphicsObject::itemChange(change, value);
}

/**
 * This virtual method is called by this WidgetBase base class to
 * notify subclasses about the need to change its boundingRect
 * Example. When a new pen is set on a widget which paints
 *
 * The default implementation just calls update()
 */
void WidgetBase::updateGeometry()
{
    update();
}

/**
 * This virtual method is called when the underlying uml object of
 * this widget changes. Disconnections to signals wrt to old object,
 * connections to new signals of new object are to be made and any
 * other book keeping work can also be done.
 *
 * @param obj The old UMLObject. The new one is accessible through
 *            umlObject()
 *
 * @note obj can be null!
 *
 * @note The reimplemented method should call this base method to
 *       connect/disconnect signals done at UMLObject level.
 */
void WidgetBase::umlObjectChanged(UMLObject *oldObj)
{
    Q_UNUSED(oldObj);
}

/**
 * This method sets the bounding rectangle of this widget to \a
 * rect. The bounding rect being set is cached locally for performance
 * reasons.
 *
 * Also the prepareGeometryChange() method is implicitly called to
 * update QGraphicsScene indexes.
 *
 * @param rect The bounding rectangle for this widget.
 *
 * @note The bounding rect being set should also be compensated with
 *       half pen width if the widget is painting an outline/stroke.
 *
 * @note Also note that, subclasses reimplementing @ref boundingRect()
 *       virtual method will not be affected by this method unless the
 *       subclassed widget explicitly uses it.
 *
 * @see Widget_Utils::adjustedBoundingRect
 */
void WidgetBase::setBoundingRect(const QRectF &rect)
{
    prepareGeometryChange();
    m_boundingRect = rect;
}

/**
 * This method sets the shape of the widget to \a path. The shape of
 * the widget is cached for performance reasons.
 *
 * @param path The shape of this widget. If empty, boundingRect will
 *             be used as widget shape.
 *
 * @see WidgetBase::setBoundingRect
 *
 * @todo Check the accuracy of this method for non rectangular widgets
 *       as this doesn't call prepareGeometryChange.
 *
 * @note Also note that, subclasses reimplementing @ref shape()
 *       virtual method will not be affected by this method unless the
 *       subclassed widget explicitly uses it.
 */
void WidgetBase::setShape(const QPainterPath& path)
{
    m_shape = path;
    if(m_shape.isEmpty()) {
        m_shape.addRect(boundingRect());
    }
    update();
}

#include "widgetbase.moc"
