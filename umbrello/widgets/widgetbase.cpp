/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
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
 * @param type    The WidgetType to construct.  This must be set to the appropriate
 *                value by the constructors of inheriting classes.
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
    setSelected(false);
    scene->addItem(this);

    // TODO 310283
    setFlags(ItemIsSelectable);
    //setFlags(ItemIsSelectable | ItemIsMovable |ItemSendsGeometryChanges);
    if (m_scene) {
        m_usesDiagramLineColor = true;
        m_usesDiagramLineWidth  = true;
        m_usesDiagramTextColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_textColor = optionState.uiState.textColor;
        m_lineColor = optionState.uiState.lineColor;
        m_lineWidth  = optionState.uiState.lineWidth;
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
    return m_baseType;
}

/**
 * @return The type used for rtti as string.
 */
QLatin1String WidgetBase::baseTypeStr() const
{
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, m_baseType));
}

/*
 * Sets the state of whether the widget is selected.
 *
 * @param select   The state of whether the widget is selected.
 */
void WidgetBase::setSelected(bool select)
{
    QGraphicsObject::setSelected(select);
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
 * Returns the font used for diaplaying any text.
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
 * @param qElement A QDomElement representing xml element data.
 */
void WidgetBase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
 {
    Q_UNUSED(qDoc)

    qElement.setAttribute(QLatin1String("textcolor"), m_usesDiagramTextColor ? QLatin1String("none")
                                                                             : m_textColor.name());
    if (m_usesDiagramLineColor) {
        qElement.setAttribute(QLatin1String("linecolor"), QLatin1String("none"));
    } else {
        qElement.setAttribute(QLatin1String("linecolor"), m_lineColor.name());
    }
    if (m_usesDiagramLineWidth) {
        qElement.setAttribute(QLatin1String("linewidth"), QLatin1String("none"));
    } else {
        qElement.setAttribute(QLatin1String("linewidth"), m_lineWidth);
    }
    qElement.setAttribute(QLatin1String("usefillcolor"), m_useFillColor);
    // for consistency the following attributes now use american spelling for "color"
    qElement.setAttribute(QLatin1String("usesdiagramfillcolor"), m_usesDiagramFillColor);
    qElement.setAttribute(QLatin1String("usesdiagramusefillcolor"), m_usesDiagramUseFillColor);
    if (m_usesDiagramFillColor) {
        qElement.setAttribute(QLatin1String("fillcolor"), QLatin1String("none"));
    } else {
        qElement.setAttribute(QLatin1String("fillcolor"), m_fillColor.name());
    }
    qElement.setAttribute(QLatin1String("font"), m_font.toString());
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
    QString lineColor = qElement.attribute(QLatin1String("linecolour"), QLatin1String("none"));
    lineColor = qElement.attribute(QLatin1String("linecolor"), lineColor);
    if (lineColor != QLatin1String("none")) {
        m_lineColor = QColor(lineColor);
        m_usesDiagramLineColor = false;
    } else if (m_baseType != WidgetBase::wt_Box && m_scene != NULL) {
        m_lineColor = m_scene->lineColor();
        m_usesDiagramLineColor = true;
    }
    QString lineWidth = qElement.attribute(QLatin1String("linewidth"), QLatin1String("none"));
    if (lineWidth != QLatin1String("none")) {
        m_lineWidth = lineWidth.toInt();
        m_usesDiagramLineWidth = false;
    } else if (m_scene) {
        m_lineWidth = m_scene->lineWidth();
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
    UMLWidget *uw = dynamic_cast<UMLWidget*>(this);
    if (uw == NULL)
        return NULL;
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
        return NULL;
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
            scene->selectWidget(dynamic_cast<UMLWidget*>(this));
        } else {
            setSelected(true);
        }
    }

    int count = scene->selectedCount(true);

    // Determine multi state
    bool multi = (isSelected() && count > 1);

    ListPopupMenu popup(0, this, multi, scene->getUniqueSelectionType());

    // Disable the "view code" menu for simple code generators
    if (UMLApp::app()->isSimpleCodeGeneratorActive()) {
        popup.setActionEnabled(ListPopupMenu::mt_ViewCode, false);
    }

    QAction *triggered = popup.exec(event->screenPos());
    ListPopupMenu *parentMenu = ListPopupMenu::menuFromAction(triggered);

    if (!parentMenu) {
        uDebug() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    WidgetBase *ownerWidget = parentMenu->ownerWidget();
    // assert because logic is based on only WidgetBase being the owner of
    // ListPopupMenu actions executed in this context menu.
    Q_ASSERT_X(ownerWidget != 0, "WidgetBase::contextMenuEvent",
            "ownerWidget is null which means action belonging to UMLView, UMLScene"
            " or UMLObject is the one triggered in ListPopupMenu");

    ownerWidget->slotMenuSelection(triggered);
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
            wt == WidgetBase::wt_Port      ||
            (wt == WidgetBase::wt_Class && umlScene()->type() == Uml::DiagramType::Class)) {

            showPropertiesDialog();

        } else if (wt == WidgetBase::wt_Object) {
            m_umlObject->showPropertiesDialog();
        } else {
            uWarning() << "making properties dialog for unknown widget type";
        }
        break;

    case ListPopupMenu::mt_Line_Color:
    case ListPopupMenu::mt_Line_Color_Selection:
        newColor = lineColor();
        if (KColorDialog::getColor(newColor)) {
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
        newColor = fillColor();
        if (KColorDialog::getColor(newColor)) {
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
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(umlObject());
        if (c) {
            UMLApp::app()->viewCodeDocument(c);
        }
        break;
    }

    case ListPopupMenu::mt_Delete:
        umlScene()->deleteSelection();
        break;

    case ListPopupMenu::mt_Change_Font:
    case ListPopupMenu::mt_Change_Font_Selection: {
        QFont newFont = font();
        if (KFontDialog::getFont(newFont, KFontChooser::NoDisplayFlags, 0) == KFontDialog::Accepted) {
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
        if (dynamic_cast<UMLClassifier*>(umlObject())) {
            UMLApp::app()->refactor(static_cast<UMLClassifier*>(umlObject()));
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

#include "widgetbase.moc"
