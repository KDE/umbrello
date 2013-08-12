/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "widgetbase.h"

#include "classifier.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlscene.h"

#include <kcolordialog.h>
#include <kfontdialog.h>

#include <QAction>
#include <QPointer>

/**
 * Creates a WidgetBase object.
 *
 * @param scene   The view to be displayed on.
 */
WidgetBase::WidgetBase(UMLScene *scene, WidgetType type)
  : QGraphicsObject(),
    m_baseType(type),
    m_scene(scene),
    m_umlObject(0),
    m_textColor(QColor("black")),
    m_lineColor(QColor("black")),
    m_fillColor(QColor("yellow")),
    m_brush(m_fillColor),
    m_lineWidth(0), // initialize with 0 to have valid start condition
    m_useFillColor(true),
    m_usesDiagramTextColor(true),
    m_usesDiagramLineColor(true),
    m_usesDiagramLineWidth(true)
{
    scene->addItem(this);

    // TODO 310283
    //setFlags(ItemIsSelectable | ItemIsMovable |ItemSendsGeometryChanges);
    if (m_scene) {
        m_usesDiagramLineColor = true;
        m_usesDiagramLineWidth  = true;
        m_usesDiagramTextColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_textColor = optionState.uiState.textColor;
        m_lineColor = optionState.uiState.lineColor;
        m_lineWidth  = optionState.uiState.lineWidth;
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
 * Deliver a pointer to the connected UMLView
 * ( needed esp. by event handling of AssociationLine ).
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
 * @param o The object to represent.
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
 * Used by some child classes to set documentation.
 *
 * @param doc   The documentation to be set in the UMLObject
 *              (if m_umlObject is set.)
 */
void WidgetBase::setDocumentation(const QString& doc)
{
    if (m_umlObject)
        m_umlObject->setDoc( doc );
    else
        m_Doc = doc;
}

/**
 * Gets the name from the corresponding UMLObject if this widget has an
 * underlying UMLObject; if it does not, then it returns the local
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
 * @param colour   The new fill color
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
 * Returns the font used for diaplaying any text.
 * @return the font
 */
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
    m_font = font;
}

/**
 * A virtual method for the widget to display a property dialog box.
 * Subclasses should reimplment this appropriately.
 */
void WidgetBase::showPropertiesDialog()
{
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
void WidgetBase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
 {
    Q_UNUSED(qDoc)

    qElement.setAttribute( "textcolor", m_usesDiagramTextColor ? "none" : m_textColor.name() );

    if (m_usesDiagramLineColor) {
        qElement.setAttribute( "linecolor", "none" );
    } else {
        qElement.setAttribute( "linecolor", m_lineColor.name() );
    }
    if (m_usesDiagramLineWidth) {
        qElement.setAttribute( "linewidth", "none" );
    } else {
        qElement.setAttribute( "linewidth", m_lineWidth );
    }
    qElement.setAttribute("usefillcolor", m_useFillColor);
    // for consistency the following attributes now use american spelling for "color"
    qElement.setAttribute("usesdiagramfillcolor", m_usesDiagramFillColor);
    qElement.setAttribute("usesdiagramusefillcolor", m_usesDiagramUseFillColor);
    if (m_usesDiagramFillColor) {
        qElement.setAttribute("fillcolor", "none");
    } else {
        qElement.setAttribute("fillcolor", m_fillColor.name());
    }
}

/**
 * A virtual method to load the properties of this widget from a
 * QDomElement into this widget.
 *
 * Subclasses should reimplement this to load addtional properties
 * required, calling this base method to load the basic properties of
 * the widget.
 *
 * @param qElement A QDomElement which contains xml info for this widget.
 *
 * @todo Add support to load older version.
 */
bool WidgetBase::loadFromXMI(QDomElement& qElement)
{
    // first load from "linecolour" and then overwrite with the "linecolor"
    // attribute if that one is present. The "linecolour" name was a "typo" in
    // earlier versions of Umbrello
    QString lineColor = qElement.attribute( "linecolour", "none" );
    lineColor = qElement.attribute( "linecolor", lineColor );
    if (lineColor != "none") {
        setLineColor( QColor(lineColor) );
        m_usesDiagramLineColor = false;
    } else if (m_baseType != WidgetBase::wt_Box && m_scene != NULL) {
        setLineColor( m_scene->lineColor() );
        m_usesDiagramLineColor = true;
    }
    QString lineWidth = qElement.attribute( "linewidth", "none" );
    if (lineWidth != "none") {
        setLineWidth( lineWidth.toInt() );
        m_usesDiagramLineWidth = false;
    } else if ( m_scene ) {
        setLineWidth( m_scene->lineWidth() );
        m_usesDiagramLineWidth = true;
    }
    QString textColor = qElement.attribute( "textcolor", "none" );
    if (textColor != "none") {
        setTextColor( QColor(textColor) );
        m_usesDiagramTextColor = false;
    } else if ( m_scene ) {
        setTextColor( m_scene->textColor() );
        m_usesDiagramTextColor = true;
    }
    QString usefillcolor = qElement.attribute("usefillcolor", "1");
    m_useFillColor = (bool)usefillcolor.toInt();
    /*
      For the next three *color attributes, there was a mixup of american and english spelling for "color".
      So first we need to keep backward compatibility and try to retrieve the *colour attribute.
      Next we overwrite this value if we find a *color, otherwise the former *colour is kept.
    */
    QString fillColor = qElement.attribute("fillcolour", "none");
    fillColor = qElement.attribute("fillcolor", fillColor);
    if (fillColor != "none") {
        m_fillColor = QColor(fillColor);
    }

    QString usesDiagramFillColor = qElement.attribute("usesdiagramfillcolour", "1");
    usesDiagramFillColor = qElement.attribute("usesdiagramfillcolor", usesDiagramFillColor);
    m_usesDiagramFillColor = (bool)usesDiagramFillColor.toInt();

    QString usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolour", "1");
    usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolor", usesDiagramUseFillColor);
    m_usesDiagramUseFillColor = (bool)usesDiagramUseFillColor.toInt();

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
    m_textColor = other.m_textColor;
    m_lineColor = other.m_lineColor;
    m_fillColor = other.m_fillColor;
    m_brush = other.m_brush;
    m_font = other.m_font;
    m_lineWidth  = other.m_lineWidth;
    m_useFillColor = other.m_useFillColor;
    m_usesDiagramTextColor = other.m_usesDiagramTextColor;
    m_usesDiagramLineColor = other.m_usesDiagramLineColor;
    m_usesDiagramFillColor = other.m_usesDiagramFillColor;
    m_usesDiagramLineWidth  = other.m_usesDiagramLineWidth;

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
 * Draws the UMLWidget on the given paint device
 *
 * @param p The painter for the drawing device
 * @param offsetX x position to start the drawing.
 * @param offsetY y position to start the drawing.
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

/**
 *
 */
void WidgetBase::setupContextMenuActions(ListPopupMenu &menu)
{
/* the following was code from UMLWidget::setupPopupMenu():
    //if in a multi- selection to a specific m_pMenu for that
    // NEW: ask UMLView to count ONLY the widgets and not their floatingtextwidgets
    int count = m_scene->selectedCount(true);
    //a MessageWidget when selected will select its text widget and vice versa
    //so take that into account for popup menu.

    // determine multi state
    bool multi = (m_selected && count > 1);

    // if multiple selected items have the same type
    bool unique = false;

    // if multiple items are selected, we have to check if they all have the same
    // base type
    if (multi == true)
        unique = m_scene->checkUniqueSelection();

    // create the right click context menu
    m_pMenu = new ListPopupMenu(m_scene->activeView(), this, multi, unique);
*/

    // disable the "view code" menu for simple code generators
    if (UMLApp::app()->isSimpleCodeGeneratorActive())
        menu.setActionEnabled(ListPopupMenu::mt_ViewCode, false);
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

    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(trigger);
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
//:TODO:            newColour = widget->brush().color();
            newColour = widget->fillColor();
        }
        if (KColorDialog::getColor(newColour)) {
            umlScene()->selectionSetFillColor(newColour);
            umlDoc()->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Use_Fill_Color:
        umlScene()->selectionUseFillColor(!m_useFillColor);
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
        if (dynamic_cast<UMLClassifier*>(umlObject())) {
            UMLApp::app()->refactor(static_cast<UMLClassifier*>(umlObject()));
        }
        break;

     case ListPopupMenu::mt_Clone:
        // In principle we clone all the uml objects.
        {
            UMLObject *pClone = umlObject()->clone();
            umlScene()->addObject(pClone);
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

#include "widgetbase.moc"
