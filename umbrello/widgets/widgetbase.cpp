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

#include <QGraphicsSceneContextMenuEvent>
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
struct WidgetInterfaceData
{
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
WidgetBase::WidgetBase(UMLObject *object) :
    m_umlObject(object),
    m_lineColor(awesomeLineColor()),
    m_lineWidth(0),
    m_brush(awesomeBrush()),
    m_activated(false),
    m_widgetInterfaceData(0),
    m_isSceneSetBefore(false),
    firstTime(true)
{
    if(!object) {
        m_widgetInterfaceData = new WidgetInterfaceData;
    }
    setFlags(ItemIsSelectable | ItemIsMovable);
    hide(); // Show up in slotInit

    QTimer::singleShot(10, this, SLOT(slotInit()));

    // DEPRECATED INITIALIZATION
    {
        for(int i= FT_NORMAL; i < FT_INVALID; ++i) {
            m_pFontMetrics[i] = new QFontMetrics(font());
        }
    }
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
 * @todo Either remove this method, or allow it to only allow specific
 *       users as making this method public violates encapsulation.
 */
void WidgetBase::setUMLObject(UMLObject *obj)
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
    QTimer::singleShot(10, this, SLOT(slotUMLObjectDataChanged()));
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
        return m_umlObject->getID();
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
    if(m_umlObject) {

        if(m_umlObject->getID() != Uml::id_None) {
            uWarning() << "changing old UMLObject " << ID2STR(m_umlObject->getID())
                       << " to " << ID2STR(id);
        }

        m_umlObject->setID(id);
    }
    else {
        m_widgetInterfaceData->id = id;
    }

    attributeChange(IDHasChanged, ID2STR(id));
}

/**
 * @return The type used for rtti.
 */
Uml::Widget_Type WidgetBase::baseType() const
{
    return m_baseType;
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
 * This is shortcut method for UMLApp::app()->getDocument()
 *
 * @return Pointer to the UMLDoc object.
 */
UMLDoc* WidgetBase::umlDoc() const
{
    return UMLApp::app()->getDocument();
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
        return m_umlObject->getDoc();
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
        return m_umlObject->getName();
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
QColor WidgetBase::fontColor() const
{
    return m_fontColor;
}

/**
 * Sets the color of the font to \a color.
 * If \a color is invalid, line color is used for font color.
 *
 * This method issues @ref FontColorHasChanged notification after
 * setting the new font color.
 * @see WidgetBase::attributeChange
 */
void WidgetBase::setFontColor(const QColor& color)
{
    const QColor oldColor = fontColor();
    m_fontColor = color;
    if(!m_fontColor.isValid()) {
        m_fontColor = m_lineColor;
    }

    attributeChange(FontColorHasChanged, oldColor);
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
}

/**
 * This virtual method should be reimplementd by subclasses to attempt complete
 * construction, with assumption that sub components have been constructed already.
 *
 * The flag, m_activated should be set to true/false on success/failure.
 *
 * By default, this method just sets the flag to true.
 *
 * @see WidgetBase::m_activated to understand what activation means.
 * @sa WidgetBase::setActivatedFlag
 */
void WidgetBase::activate()
{
    m_activated = true;
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
    // NOTE:
    // The "none" is used by kde3 version of umbrello. The current
    // technique to determine whether a property is being used from
    // the diagram or not is just to compare the same, rather than
    // having flags for them.
    //
    // This way, there is no burden to update the flags and the code
    // is more robust.
    const QLatin1String none("none");

    // Load the line color first

    // Workaround for old "linecolour" usage.
    QString lineColor = qElement.attribute("linecolour");
    lineColor = qElement.attribute("linecolor", lineColor);
    if(!lineColor.isEmpty() && lineColor != none) {
        setLineColor(QColor(lineColor));
    }
    else if(umlScene()) {
        setLineColor(umlScene()->getLineColor());
    }

    // Load the line width.
    QString lineWidth = qElement.attribute("linewidth");
    if(!lineWidth.isEmpty() && lineWidth != none) {
        setLineWidth(lineWidth.toInt());
    }
    else if(umlScene()) {
        setLineWidth(umlScene()->getLineWidth());
    }

    // Load the font color, if invalid line color is automatically used.
    QString fontColor = qElement.attribute("fontcolor");
    setFontColor(QColor(fontColor));

    // Load the brush.
    QBrush newBrush;
    bool brushSet = Widget_Utils::loadBrushFromXMI(qElement, newBrush);

    // If this fails, we try to load fillColor attribute which is used in kde3 version of umbrello.
    if(!brushSet) {
        // Workaround for old "fillcolour" usage.
        QString fillColor = qElement.attribute("fillcolour");
        fillColor = qElement.attribute("fillcolor");

        if(!fillColor.isEmpty() && fillColor != none) {
            setBrush(QColor(fillColor));
            brushSet = true;
        }
    }
    else {
        setBrush(newBrush);
    }
    // Set the diagram's brush if it is not yet set.
    if(!brushSet && umlScene()) {
        setBrush(umlScene()->brush());
    }

    // Load the font.
    QString font = qElement.attribute(QLatin1String("font"));
    bool fontSet = false;
    if(!font.isEmpty()) {
        fontSet = m_font.fromString(font);
        if(fontSet) {
            setFont(m_font);
        }
        else {
            uWarning() << "Loading font attribute->" << font << " failed";
        }
    }
    // Set diagram's default font if font is not yet set.
    if(!fontSet && umlScene()) {
        setFont(umlScene()->font());
    }

    QString id = qElement.attribute("xmi.id", "-1");
    // Assert for the correctness of id loaded and the created object.
    if(m_umlObject) {
        if(id != ID2STR(this->id())) {
            uWarning() << "ID mismatch between UMLWidget and its UMLObject"
                       << "So the id read will be ignored.";
        }
    }
    else {
        setID(STR2ID(id));
    }

    qreal x = qElement.attribute("x", "0").toDouble();
    qreal y = qElement.attribute("y", "0").toDouble();
    setPos(x, y);

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
    qElement.setAttribute("linecolor", m_lineColor.name());
    qElement.setAttribute("linewidth", m_lineWidth);
    qElement.setAttribute("fontcolor", m_fontColor.name());
    qElement.setAttribute("font", m_font.toString());

    Widget_Utils::saveBrushToXMI(qDoc, qElement, m_brush);

    qElement.setAttribute("xmi.id", ID2STR(id()));
    qElement.setAttribute("x", pos().x());
    qElement.setAttribute("y", pos().y());
}

/**
 * @return Whether the widget type has an associated UMLObject
 */
bool WidgetBase::widgetHasUMLObject(Uml::Widget_Type type)
{
    switch(type)
    {
    case Uml::wt_Actor:
    case Uml::wt_UseCase:
    case Uml::wt_Class:
    case Uml::wt_Interface:
    case Uml::wt_Enum:
    case Uml::wt_Datatype:
    case Uml::wt_Package:
    case Uml::wt_Component:
    case Uml::wt_Node:
    case Uml::wt_Artifact:
    case Uml::wt_Object:
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
    if(!trigger) {
        return;
    }
    QColor newColour;
    WidgetBase* widget = 0; // use for select the first object properties (fill, line color)

    const Uml::Widget_Type wt = m_baseType; // short hand name

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(trigger);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    ListPopupMenu::Menu_Type sel = menu->getMenuType(trigger);
    switch (sel) {
    case ListPopupMenu::mt_Rename:
        umlDoc()->renameUMLObject(umlObject());
        break;

    case ListPopupMenu::mt_Delete:
        // umlScene()->removeWidget(this);
        break;

    //     //UMLWidgetController::doMouseDoubleClick relies on this implementation
    case ListPopupMenu::mt_Properties:
        if (wt == Uml::wt_Actor || wt == Uml::wt_UseCase ||
            wt == Uml::wt_Package || wt == Uml::wt_Interface || wt == Uml::wt_Datatype ||
            wt == Uml::wt_Component || wt == Uml::wt_Artifact ||
            wt == Uml::wt_Node || wt == Uml::wt_Enum || wt == Uml::wt_Entity ||
            (wt == Uml::wt_Class && umlScene()->getType() == Uml::dt_Class)) {

            showPropertiesDialog();

        } else if (wt == Uml::wt_Object) {
            m_umlObject->showProperties();
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
            newColour = widget->getFillColor();
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
        if (KFontDialog::getFont(newFont, false, 0) == KFontDialog::Accepted) {
            setFont(newFont);
            //UMLApp::app()->executeCommand(new CmdChangeFontSelection(m_pDoc, m_pView, font));
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
        uDebug() << "Menu_Type " << sel << " not implemented";
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
 * This slot is used to intialize the widget, also allowing virtual
 * methods to be called.
 */
void WidgetBase::slotInit()
{
    // Call this virtual method to ensure the superclasses initialize themselves with
    // various properties of m_umlObject.
    setUMLObject(m_umlObject);

    // Now call attributeChange with various geometry parameters to ensure that proper
    // initialization of sub objects' properties take place.
    QVariant v;
    attributeChange(LineColorHasChanged, v);
    attributeChange(LineWidthHasChanged, v);
    attributeChange(FontHasChanged, v);
    attributeChange(FontColorHasChanged, v);
    attributeChange(BrushHasChanged, v);

    // Now show the item (Ugly hack to prevent floatingtext widget from showing up
    // until sane text is set on it)
    if (baseType() != Uml::wt_Text) {
        show();
    }
    // Now just update the geometry for the first ever time after it is shown.
    updateGeometry();
    // Now invoke the virtual delayedInitialize() for subclasses to do more initialization.
    delayedInitialize();
}

/**
 * Reimplemented to show appropriate context menu.
 */
void WidgetBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();

    QPointer<ListPopupMenu> menu = new ListPopupMenu(0, this, false, false);
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
    case FontColorHasChanged:
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
    if (change == ItemSceneHasChanged) {
        UMLScene *uScene = umlScene();
        if (uScene && !m_isSceneSetBefore) {
            m_isSceneSetBefore = true;
            // call this virtual method now.
            sceneSetFirstTime();
        }
    }
    return QGraphicsItem::itemChange(change, value);
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
 * This virtual method is called very shortly after the object is constructed completely.
 * A timer with small timeout is used for this purpose.
 * @see WidgetBase::slotInit()
 */
void WidgetBase::delayedInitialize()
{
}

/**
 * This virtual method is called after this widget has been associated with a UMLScene
 * for the first time.
 * Initalization from UMLScene's properties like line color etc. can be done by
 * reimplementing this method.
 */
void WidgetBase::sceneSetFirstTime()
{
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



///////////////////////////////SOME DEPRECATED STUFF///////////////

// WidgetBase::WidgetBase(UMLScene *scene, UMLObject *object) :
//     QObject(),
//     QGraphicsItem(0),
// 
//     m_umlObject(object),
//     m_lineColor(Qt::red),
//     m_lineWidth(0),
//     m_brush(awesomeBrush()),
//     m_widgetInterfaceData(0),
//     m_isSceneSetBefore(false),
//     firstTime(true)
// {
//     for(int i= FT_NORMAL; i < FT_INVALID; ++i) {
//         m_pFontMetrics[i] = new QFontMetrics(font());
//     }
//     if(!object) {
//         m_widgetInterfaceData = new WidgetInterfaceData;
//     }
//     setFlags(ItemIsSelectable | ItemIsMovable);
//     hide();
//     if(scene) {
//         scene->addItem(this);
//     }
// }

// WidgetBase::WidgetBase(UMLScene *scene, const Uml::IDType &_id) :
//     m_umlObject(0),
//     m_lineColor(Qt::red),
//     m_lineWidth(0),
//     m_brush(awesomeBrush()),
//     m_isSceneSetBefore(false),
//     firstTime(true)
// {
//     for(int i= FT_NORMAL; i < FT_INVALID; ++i) {
//         m_pFontMetrics[i] = new QFontMetrics(font());
//     }
//     m_widgetInterfaceData = new WidgetInterfaceData;
//     if(_id == Uml::id_None) {
//         m_widgetInterfaceData->id = UniqueID::gen();
//     }
//     else {
//         m_widgetInterfaceData->id = _id;
//     }
//     if(scene) {
//         scene->addItem(this);
//     }
//     hide();
//     setFlags(ItemIsSelectable | ItemIsMovable);
// }

/**
 * Template Method, override this to set the default
 *  font metric.
 */
void WidgetBase::setDefaultFontMetrics(WidgetBase::FontType fontType)
{
    setupFontType(m_font, fontType);
    setFontMetrics(fontType, QFontMetrics(m_font));
}

void WidgetBase::setupFontType(QFont &font, WidgetBase::FontType fontType)
{
    switch (fontType) {
    case FT_NORMAL:
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
        break;
    case FT_BOLD:
        font.setBold(true);
        font.setItalic(false);
        font.setUnderline(false);
        break;
    case FT_ITALIC:
        font.setBold(false);
        font.setItalic(true);
        font.setUnderline(false);
        break;
    case FT_UNDERLINE:
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(true);
        break;
    case FT_BOLD_ITALIC:
        font.setBold(true);
        font.setItalic(true);
        font.setUnderline(false);
        break;
    case FT_BOLD_UNDERLINE:
        font.setBold(true);
        font.setItalic(false);
        font.setUnderline(true);
        break;
    case FT_ITALIC_UNDERLINE:
        font.setBold(false);
        font.setItalic(true);
        font.setUnderline(true);
        break;
    case FT_BOLD_ITALIC_UNDERLINE:
        font.setBold(true);
        font.setItalic(true);
        font.setUnderline(true);
        break;
    default: return;
    }
}

/**
 * Template Method, override this to set the default
 *  font metric.
 */
void WidgetBase::setDefaultFontMetrics(WidgetBase::FontType fontType, QPainter &painter)
{
    setupFontType(m_font, fontType);
    painter.setFont(m_font);
    setFontMetrics(fontType, painter.fontMetrics());
}

//FIXME this is probably the source of problems with widgets not being wide enough

/** Returns the font metric used by this object for Text which uses bold/italic fonts*/
QFontMetrics &WidgetBase::getFontMetrics(WidgetBase::FontType fontType)
{
    if (m_pFontMetrics[fontType] == 0) {
        setDefaultFontMetrics(fontType);
    }
    return *m_pFontMetrics[fontType];
}

/** set the font metric to use */
void WidgetBase::setFontMetrics(WidgetBase::FontType fontType, QFontMetrics fm)
{
    delete m_pFontMetrics[fontType];
    m_pFontMetrics[fontType] = new QFontMetrics(fm);
}

void WidgetBase::forceUpdateFontMetrics(QPainter *painter)
{
    if (painter == 0) {
        for (int i = 0; i < (int)WidgetBase::FT_INVALID; ++i) {
            if (m_pFontMetrics[(WidgetBase::FontType)i] != 0)
                setDefaultFontMetrics((WidgetBase::FontType)i);
        }
    } else {
        for (int i2 = 0; i2 < (int)WidgetBase::FT_INVALID; ++i2) {
            if (m_pFontMetrics[(WidgetBase::FontType)i2] != 0)
                setDefaultFontMetrics((WidgetBase::FontType)i2, *painter);
        }
    }
    // calculate the size, based on the new font metric
    updateComponentSize();
}

void WidgetBase::updateComponentSize()
{
    if(firstTime) {
        firstTime = false;
    }
    else {
        UMLWidget *rect = dynamic_cast<UMLWidget*>(this);
        if(rect) {
            slotUMLObjectDataChanged();
        }
        else {
            updateGeometry();
        }
    }
}

#include "widgetbase.moc"
