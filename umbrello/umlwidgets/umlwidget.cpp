/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umlwidget.h"

// local includes
#include "associationwidget.h"
#include "classifier.h"
#include "classpropertiesdialog.h"
#include "cmds.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "floatingtextwidget.h"
#include "idchangelog.h"
#include "listpopupmenu.h"
#include "settingsdialog.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"
#include "uniqueid.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QPointer>

using namespace Uml;

DEBUG_REGISTER_DISABLED(UMLWidget)

const QSizeF UMLWidget::DefaultMinimumSize(50, 20);
const QSizeF UMLWidget::DefaultMaximumSize(1000, 5000);
const qreal UMLWidget::defaultMargin = 5;

/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param type  The WidgetType to construct.
 *              This must be set to the appropriate value by the constructors of inheriting classes.
 * @param o The UMLObject to represent.
 */
UMLWidget::UMLWidget(UMLScene * scene, WidgetType type, UMLObject * o)
  : WidgetBase(scene, type)
{
    init();
    m_umlObject = o;
    if (m_umlObject) {
        connect(m_umlObject, SIGNAL(modified()), this, SLOT(updateWidget()));
        m_nId = m_umlObject->id();
    }
}

/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param type  The WidgetType to construct.
 *              This must be set to the appropriate value by the constructors of inheriting classes.
 * @param id The id of the widget.
 *  The default value (id_None) will prompt generation of a new ID.
 */
UMLWidget::UMLWidget(UMLScene *scene, WidgetType type, Uml::ID::Type id)
  : WidgetBase(scene, type)
{
    init();
    if (id == Uml::ID::None)
        m_nId = UniqueID::gen();
    else
        m_nId = id;
}

/**
 * Destructor.
 */
UMLWidget::~UMLWidget()
{
    cleanup();
}

/**
 * Assignment operator
 */
UMLWidget& UMLWidget::operator=(const UMLWidget & other)
{
    if (this == &other)
        return *this;

    WidgetBase::operator=(other);

    // assign members loaded/saved
    m_useFillColor = other.m_useFillColor;
    m_usesDiagramFillColor = other.m_usesDiagramFillColor;
    m_usesDiagramUseFillColor = other.m_usesDiagramUseFillColor;
    m_fillColor = other.m_fillColor;
    m_Assocs = other.m_Assocs;
    m_isInstance = other.m_isInstance;
    m_instanceName = other.m_instanceName;
    m_instanceName = other.m_instanceName;
    m_showStereotype = other.m_showStereotype;
    setX(other.x());
    setY(other.y());
    setRect(rect().x(), rect().y(), other.width(), other.height());

    // assign volatile (non-saved) members
    m_startMove = other.m_startMove;
    m_nPosX = other.m_nPosX;
    m_doc = other.m_doc;    //new
    m_resizable = other.m_resizable;
    for (unsigned i = 0; i < FT_INVALID; ++i)
        m_pFontMetrics[i] = other.m_pFontMetrics[i];
    m_activated = other.m_activated;
    m_ignoreSnapToGrid = other.m_ignoreSnapToGrid;
    m_ignoreSnapComponentSizeToGrid = other.m_ignoreSnapComponentSizeToGrid;
    return *this;
}

/**
 * Overload '==' operator
 */
bool UMLWidget::operator==(const UMLWidget& other) const
{
    if (this == &other)
        return true;

    if (baseType() != other.baseType()) {
        return false;
    }

    if (id() != other.id())
        return false;

    /* Testing the associations is already an exaggeration, no?
       The type and ID should uniquely identify an UMLWidget.
     */
    if (m_Assocs.count() != other.m_Assocs.count()) {
        return false;
    }

    // if(getBaseType() != wt_Text) // DON'T do this for floatingtext widgets, an infinite loop will result
    // {
    AssociationWidgetListIt assoc_it(m_Assocs);
    AssociationWidgetListIt assoc_it2(other.m_Assocs);
    AssociationWidget * assoc = 0, *assoc2 = 0;
    while (assoc_it.hasNext() &&  assoc_it2.hasNext()) {
        assoc = assoc_it.next();
        assoc2 = assoc_it2.next();

        if (!(*assoc == *assoc2)) {
            return false;
        }
    }
    // }
    return true;
    // NOTE:  In the comparison tests we are going to do, we don't need these values.
    // They will actually stop things functioning correctly so if you change these, be aware of that.
    /*
    if(m_useFillColor != other.m_useFillColor)
        return false;
    if(m_nId != other.m_nId)
        return false;
    if(m_nX  != other.m_nX)
        return false;
    if(m_nY != other.m_nY)
        return false;
     */
}

/**
 * Sets the local id of the object.
 *
 * @param id   The local id of the object.
 */
void UMLWidget::setLocalID(Uml::ID::Type id)
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
Uml::ID::Type UMLWidget::localID() const
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
UMLWidget* UMLWidget::widgetWithID(Uml::ID::Type id)
{
    if (id == m_nLocalID ||
        (m_umlObject != 0 && id == m_umlObject->id()) ||
        id == m_nId)
        return this;
    return 0;
}

/**
 * Compute the minimum possible width and height.
 *
 * @return QSizeF(mininum_width, minimum_height)
 */
QSizeF UMLWidget::minimumSize() const
{
    return m_minimumSize;
}

/**
 * This method is used to set the minimum size variable for this
 * widget.
 *
 * @param newSize The size being set as minimum.
 */
void UMLWidget::setMinimumSize(const QSizeF& newSize)
{
    m_minimumSize = newSize;
}

/**
 * Compute the maximum possible width and height.
 *
 * @return maximum size
 */
QSizeF UMLWidget::maximumSize()
{
    return m_maximumSize;
}

/**
 * This method is used to set the maximum size variable for this
 * widget.
 *
 * @param newSize The size being set as maximum.
 */
void UMLWidget::setMaximumSize(const QSizeF& newSize)
{
    m_maximumSize = newSize;
}

/**
 * Event handler for context menu events.
 */
void UMLWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    WidgetBase::contextMenuEvent(event);
}

/**
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * This method doesn't adjust associations. It only moves the widget.
 *
 * It can be overridden to constrain movement only in one axis even when
 * the user isn't constraining the movement with shift or control buttons, for example.
 * The movement policy set here is applied whenever the widget is moved, being it
 * moving it explicitly, or as a part of a selection but not receiving directly the
 * mouse events.
 *
 * Default behaviour is move the widget to the new position using the diffs.
 * @see constrainMovementForAllWidgets
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void UMLWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    setX(x() + diffX);
    setY(y() + diffY);
}

/**
 * Modifies the value of the diffX and diffY variables used to move the widgets.
 *
 * It can be overridden to constrain movement of all the selected widgets only in one
 * axis even when the user isn't constraining the movement with shift or control
 * buttons, for example.
 * The difference with moveWidgetBy is that the diff positions used here are
 * applied to all the selected widgets instead of only to m_widget, and that
 * moveWidgetBy, in fact, moves the widget, and here simply the diff positions
 * are modified.
 *
 * Default behaviour is do nothing.
 * @see moveWidgetBy
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void UMLWidget::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY)
{
    Q_UNUSED(diffX) Q_UNUSED(diffY)
}

/**
 * Bring the widget at the pressed position to the foreground.
 */
void UMLWidget::toForeground()
{
    QRectF rect = QRectF(scenePos(), QSizeF(width(), height()));
    QList<QGraphicsItem*> items = scene()->items(rect, Qt::IntersectsItemShape, Qt::DescendingOrder);
    DEBUG(DBG_SRC) << "items at " << rect << " = " << items.count();
    if (items.count() > 1) {
        foreach(QGraphicsItem* i, items) {
            UMLWidget* w = dynamic_cast<UMLWidget*>(i);
            if (w) {
                DEBUG(DBG_SRC) << "item=" << w->name() << " with zValue=" << w->zValue();
                if (w->name() != name()) {
                    if (w->zValue() >= zValue()) {
                        setZValue(w->zValue() + 1.0);
                        DEBUG(DBG_SRC) << "bring to foreground with zValue: " << zValue();
                    }
                }
            }
        }
    }
    else {
        setZValue(0.0);
    }
    DEBUG(DBG_SRC) << "zValue is " << zValue();
}

/**
 * Handles a mouse press event.
 * It'll select the widget (or mark it to be deselected) and prepare it to
 * be moved or resized. Go on reading for more info about this.
 *
 * Widget values and message bar status are saved.
 *
 * If shift or control buttons are pressed, we're in move area no matter
 * where the button was pressed in the widget. Moreover, if the widget
 * wasn't already selected, it's added to the selection. If already selected,
 * it's marked to be deselected when releasing the button (provided it isn't
 * moved).
 * Also, if the widget is already selected with other widgets but shift nor
 * control buttons are pressed, we're in move area. If finally we don't move
 * the widget, it's selected and the other widgets deselected when releasing
 * the left button.
 *
 * If shift nor control buttons are pressed, we're facing a single selection.
 * Depending on the position of the cursor, we're in move or in resize area.
 * If the widget wasn't selected (both when there are no widgets selected, or
 * when there're other widgets selected but not the one receiving the press
 * event) it's selected and the others deselected, if any. If already selected,
 * it's marked to be deselected when releasing the button (provided it wasn't
 * moved or resized).
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    event->accept();
    DEBUG(DBG_SRC) << "widget = " << name() << " / type = " << baseTypeStr();

    toForeground();

    m_startMovePostion = pos();
    m_startResizeSize = QSizeF(width(), height());

    // saving the values of the widget
    m_pressOffset = event->scenePos() - pos();
    DEBUG(DBG_SRC) << "press offset=" << m_pressOffset;

    m_oldStatusBarMsg = UMLApp::app()->statusBarMsg();

    if (event->modifiers() == Qt::ShiftModifier || event->modifiers() == Qt::ControlModifier) {
        m_shiftPressed = true;

        if (event->button() == Qt::LeftButton) {
            m_inMoveArea = true;
        }

        if (!isSelected()) {
            selectMultiple(event);
        }
        return;
    }

    m_shiftPressed = false;

    int count = m_scene->selectedCount(true);
    if (event->button() == Qt::LeftButton) {
        if (isSelected() && count > 1) {
            // single selection is made in release event if the widget wasn't moved
            m_inMoveArea = true;
            m_oldPos = pos();
            return;
        }

        if (isInResizeArea(event)) {
            m_inResizeArea = true;
            m_oldW = width();
            m_oldH = height();
        } else {
            m_inMoveArea = true;
        }
    }

    // if widget wasn't selected, or it was selected but with other widgets also selected
    if (!isSelected() || count > 1) {
        selectSingle(event);
    }
}

/**
 * Handles a mouse move event.
 * It resizes or moves the widget, depending on where the cursor is pressed
 * on the widget. Go on reading for more info about this.
 *
 * If resizing, the widget is resized using UMLWidget::resizeWidget (where specific
 * widget resize constraint can be applied), and then the associations are
 * adjusted.
 * The resizing can be constrained also to a specific axis using control
 * and shift buttons. If on or another is pressed, it's constrained to X axis.
 * If both are pressed, it's constrained to Y axis.
 *
 * If not resizing, the widget is being moved. If the move is being started,
 * the selection bounds are set (which includes updating the list of selected
 * widgets).
 * The difference between the previous position of the selection and the new
 * one is got (taking in account the selection bounds so widgets don't go
 * beyond the scene limits). Then, it's constrained to X or Y axis depending
 * on shift and control buttons.
 * A further constraint is made using constrainMovementForAllWidgets (for example,
 * if the widget that receives the event can only be moved in Y axis, with this
 * method the movement of all the widgets in the selection can be constrained to
 * be moved only in Y axis).
 * Then, all the selected widgets are moved using moveWidgetBy (where specific
 * widget movement constraint can be applied) and, if a certain amount of time
 * passed from the last move event, the associations are also updated (they're
 * not updated always to be easy on the CPU). Finally, the scene is resized,
 * and selection bounds updated.
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_inResizeArea) {
        resize(event);
        return;
    }

    if (!m_moved) {
        UMLApp::app()->document()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel move."));

        m_moved = true;
        //Maybe needed by AssociationWidget
        m_startMove = true;

        setSelectionBounds();
    }

    QPointF position = event->scenePos() - m_pressOffset;
    qreal diffX = position.x() - x();
    qreal diffY = position.y() - y();

    if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier)) {
        // move only in Y axis
        diffX = 0;
    } else if ((event->modifiers() & Qt::ShiftModifier) || (event->modifiers() & Qt::ControlModifier)) {
        // move only in X axis
        diffY = 0;
    }

    constrainMovementForAllWidgets(diffX, diffY);

    // nothing to move
    if (diffX == 0 && diffY == 0) {
        return;
    }

    QPointF delta = event->scenePos() - event->lastScenePos();
    adjustUnselectedAssocs(delta.x(), delta.y());

    DEBUG(DBG_SRC) << "diffX=" << diffX << " / diffY=" << diffY;
    foreach(UMLWidget* widget, umlScene()->selectedWidgets()) {
        if ((widget->parentItem() == 0) || (!widget->parentItem()->isSelected())) {
            widget->moveWidgetBy(diffX, diffY);
            widget->adjustUnselectedAssocs(delta.x(), delta.y());
            widget->slotSnapToGrid();
        }
    }

    // Move any selected associations.
    foreach(AssociationWidget* aw, m_scene->selectedAssocs()) {
        if (aw->isSelected()) {
            aw->moveEntireAssoc(diffX, diffY);
        }
    }

    umlScene()->resizeSceneToItems();
}

/**
 * Handles a mouse release event.
 * It selects or deselects the widget and cancels or confirms the move or
 * resize. Go on reading for more info about this.
 * No matter which tool is selected, Z position of widget is updated.
 *
 * Middle button release resets the selection.
 * Left button release, if it wasn't moved nor resized, selects the widget
 * and deselect the others if it wasn't selected and there were other widgets
 * selected. If the widget was marked to be deselected, deselects it.
 * If it was moved or resized, the document is set to modified if position
 * or size changed. Also, if moved, all the associations are adjusted because
 * the timer could have prevented the adjustment in the last move event before
 * the release.
 * If mouse was pressed in resize area, cursor is set again to normal cursor
 * Right button release if right button was pressed shows the pop up menu for
 * the widget.
 * If left button was pressed, it cancels the move or resize with a mouse move
 * event at the same position than the cursor was when pressed. Another left
 * button release is also sent.
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_moved && !m_resized) {
        if (!m_shiftPressed && (m_scene->selectedCount(true) > 1)) {
            selectSingle(event);
        } else if (!isSelected()) {
            deselect(event);
        }
    } else {
        // Commands
        if (m_moved) {
            int selectionCount = umlScene()->selectedWidgets().count();
            if (selectionCount > 1) {
                UMLApp::app()->beginMacro(i18n("Move widgets"));
            }
            foreach(UMLWidget* widget, umlScene()->selectedWidgets()) {
                UMLApp::app()->executeCommand(new Uml::CmdMoveWidget(widget));
            }
            if (selectionCount > 1) {
                UMLApp::app()->endMacro();
            }
            m_moved = false;
        } else {
            UMLApp::app()->executeCommand(new Uml::CmdResizeWidget(this));
            m_autoResize = false;
            m_resized = false;
        }

        if ((m_inMoveArea && wasPositionChanged()) ||
                (m_inResizeArea && wasSizeChanged())) {
            umlDoc()->setModified(true);
        }

        UMLApp::app()->document()->writeToStatusBar(m_oldStatusBarMsg);
    }

    if (m_inResizeArea) {
        m_inResizeArea = false;
        m_scene->activeView()->setCursor(Qt::ArrowCursor);
    } else {
        m_inMoveArea = false;
    }
    m_startMove = false;
}

/**
 * Event handler for mouse double click events.
 * @param event the QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        DEBUG(DBG_SRC) << "widget = " << name() << " / type = " << baseTypeStr();
        switch(baseType()) {
        case WidgetBase::wt_Message:  // will be handled in its class
            QGraphicsItem::mouseDoubleClickEvent(event);
            break;
        default:
            showPropertiesDialog();
            event->accept();
            break;
        }
    }
}

/**
 * Return the start position of the move action.
 * @return   point where the move began
 */
QPointF UMLWidget::startMovePosition() const
{
    return m_startMovePostion;
}

/**
 * Set the start position of the move action.
 * @param position point where the move began
 */
void UMLWidget::setStartMovePosition(const QPointF &position)
{
    m_startMovePostion = position;
}

/**
 * Return the start size of the resize action.
 * @return   size where the resize began
 */
QSizeF UMLWidget::startResizeSize() const
{
    return m_startResizeSize;
}

/**
 * Resizes the widget.
 * It's called from resize, after the values are constrained and before
 * the associations are adjusted.
 *
 * Default behaviour is resize the widget using the new size values.
 * @see resize
 *
 * @param newW   The new width for the widget.
 * @param newH   The new height for the widget.
 */
void UMLWidget::resizeWidget(qreal newW, qreal newH)
{
    setSize(newW, newH);
}

/**
 * Notify child widget about parent resizes.
 * Child widgets can override this function to move when their parent is resized.
 */
void UMLWidget::notifyParentResize()
{
}


/**
 * When a widget changes this slot captures that signal.
 */
void UMLWidget::updateWidget()
{
    updateGeometry();
    switch (baseType()) {
    case WidgetBase::wt_Class:
        m_scene->createAutoAttributeAssociations(this);
        break;
    case WidgetBase::wt_Entity:
        m_scene->createAutoConstraintAssociations(this);
        break;
    default:
        break;
    }

    if (isVisible())
        update();
}

/**
 * Apply possible constraints to the given candidate width and height.
 * The default implementation limits input values to the bounds returned
 * by minimumSize()/maximumSize().
 *
 * @param width  input value, may be modified by the constraint
 * @param height input value, may be modified by the constraint
 */
void UMLWidget::constrain(qreal& width, qreal& height)
{
    QSizeF minSize = minimumSize();
    if (width < minSize.width())
        width = minSize.width();
    if (height < minSize.height())
        height = minSize.height();
    QSizeF maxSize = maximumSize();
    if (width > maxSize.width())
        width = maxSize.width();
    if (height > maxSize.height())
        height = maxSize.height();

    if (fixedAspectRatio()) {
        QSizeF size = rect().size();
        float aspectRatio = size.width() > 0 ? (float)size.height()/size.width() : 1;
        height = width * aspectRatio;
    }
}

/**
 * Initializes key attributes of the class.
 */
void UMLWidget::init()
{
    m_nId = Uml::ID::None;
    m_nLocalID = UniqueID::gen();
    m_isInstance = false;
    setMinimumSize(DefaultMinimumSize);
    setMaximumSize(DefaultMaximumSize);

    m_font = QApplication::font();
    for (int i = (int)FT_INVALID - 1; i >= 0; --i) {
        FontType fontType = (FontType)i;
        setupFontType(m_font, fontType);
        m_pFontMetrics[fontType] = new QFontMetrics(m_font);
    }

    if (m_scene) {
        m_useFillColor = true;
        m_usesDiagramFillColor = true;
        m_usesDiagramUseFillColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_fillColor = optionState.uiState.fillColor;
        m_showStereotype = optionState.classState.showStereoType;
    } else {
        uError() << "SERIOUS PROBLEM - m_scene is NULL";
        m_useFillColor = false;
        m_usesDiagramFillColor = false;
        m_usesDiagramUseFillColor = false;
        m_showStereotype = false;
    }

    m_resizable = true;
    m_fixedAspectRatio = false;

    m_startMove = false;
    m_activated = false;
    m_ignoreSnapToGrid = false;
    m_ignoreSnapComponentSizeToGrid = false;
    m_doc = UMLApp::app()->document();
    m_nPosX = 0;
    connect(m_scene, SIGNAL(sigFillColorChanged(Uml::ID::Type)), this, SLOT(slotFillColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigLineColorChanged(Uml::ID::Type)), this, SLOT(slotLineColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigTextColorChanged(Uml::ID::Type)), this, SLOT(slotTextColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigLineWidthChanged(Uml::ID::Type)), this, SLOT(slotLineWidthChanged(Uml::ID::Type)));

    m_umlObject = 0;

    m_oldPos = QPointF();
    m_pressOffset = QPointF();
    m_oldW = 0;
    m_oldH = 0;

    m_shiftPressed = false;
    m_inMoveArea = false;
    m_inResizeArea = false;
    m_moved = false;
    m_resized = false;

    // propagate line color set by base class constructor
    // which does not call the virtual methods from this class.
    setLineColor(lineColor());

    setZValue(2.0);  // default for most widgets
}

/**
 * This is usually called synchronously after menu.exec() and \a
 * trigger's parent is always the ListPopupMenu which can be used to
 * get the type of action of \a trigger.
 *
 * @note Subclasses can reimplement to handle specific actions and
 *       leave the rest to WidgetBase::slotMenuSelection.
 */
void UMLWidget::slotMenuSelection(QAction *trigger)
{
    if (!trigger) {
        return;
    }

    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(trigger);
    switch (sel) {
    case ListPopupMenu::mt_Resize:
        umlScene()->resizeSelection();
        break;

    case ListPopupMenu::mt_AutoResize:
        setAutoResize(trigger->isChecked());
        updateGeometry();
        break;

    case ListPopupMenu::mt_Rename_Object: {
        QString name = m_instanceName;
        bool ok = Dialog_Utils::askName(i18n("Rename Object"),
                                        i18n("Enter object name:"),
                                        name);
        if (ok) {
            m_instanceName = name;
            updateGeometry();
            moveEvent(0);
            update();
            UMLApp::app()->document()->setModified(true);
        }
        break;
    }

    default:
        WidgetBase::slotMenuSelection(trigger);
        break;
    }
}

/**
 * Captures when another widget moves if this widget is linked to it.
 * @see sigWidgetMoved
 *
 * @param id The id of object behind the widget.
 */
void UMLWidget::slotWidgetMoved(Uml::ID::Type /*id*/)
{
}

/**
 * Captures a color change signal.
 *
 * @param viewID  The id of the UMLScene behind the widget.
 */
void UMLWidget::slotFillColorChanged(Uml::ID::Type viewID)
{
    //only change if on the diagram concerned
    if (m_scene->ID() != viewID) {
        return;
    }
    if (m_usesDiagramFillColor) {
        WidgetBase::setFillColor(m_scene->fillColor());
    }
    if (m_usesDiagramUseFillColor) {
        WidgetBase::setUseFillColor(m_scene->useFillColor());
    }
    update();
}

/**
 * Captures a text color change signal.
 *
 * @param viewID  The id of the UMLScene behind the widget.
 */
void UMLWidget::slotTextColorChanged(Uml::ID::Type viewID)
{
    //only change if on the diagram concerned
    if (m_scene->ID() != viewID)
        return;
    WidgetBase::setTextColor(m_scene->textColor());
    update();
}


/**
 * Captures a line color change signal.
 *
 * @param viewID  The id of the UMLScene behind the widget.
 */
void UMLWidget::slotLineColorChanged(Uml::ID::Type viewID)
{
    //only change if on the diagram concerned
    if (m_scene->ID() != viewID)
        return;

    if (m_usesDiagramLineColor) {
        WidgetBase::setLineColor(m_scene->lineColor());
    }
    update();
}

/**
 * Captures a linewidth change signal.
 *
 * @param viewID  The id of the UMLScene behind the widget.
 */
void UMLWidget::slotLineWidthChanged(Uml::ID::Type viewID)
{
    //only change if on the diagram concerned
    if (m_scene->ID() != viewID) {
        return;
    }
    if (m_usesDiagramLineWidth) {
        WidgetBase::setLineWidth(m_scene->lineWidth());
    }
    update();
}

/**
 * Set the status of using fill color (undo action)
 *
 * @param fc the status of using fill color.
 */
void UMLWidget::setUseFillColor(bool fc)
{
    if (useFillColor() != fc) {
        UMLApp::app()->executeCommand(new CmdChangeUseFillColor(this, fc));
    }
}

/**
 * Set the status of using fill color.
 *
 * @param fc the status of using fill color.
 */
void UMLWidget::setUseFillColorCmd(bool fc)
{
    WidgetBase::setUseFillColor(fc);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColorCmd(const QColor &color)
{
    WidgetBase::setTextColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColor(const QColor &color)
{
    if (textColor() != color) {
        UMLApp::app()->executeCommand(new CmdChangeTextColor(this, color));
        update();
    }
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColorCmd(const QColor &color)
{
    WidgetBase::setLineColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColor(const QColor &color)
{
    if (lineColor() != color) {
        UMLApp::app()->executeCommand(new CmdChangeLineColor(this, color));
    }
}

/**
 * Overrides the method from WidgetBase, execute CmdChangeLineWidth
 */
void UMLWidget::setLineWidth(uint width)
{
    if (lineWidth() != width) {
        UMLApp::app()->executeCommand(new CmdChangeLineWidth(this, width));
    }
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineWidthCmd(uint width)
{
    WidgetBase::setLineWidth(width);
    update();
}

/**
 * Sets the background fill color
 *
 * @param color the new fill color
 */
void UMLWidget::setFillColor(const QColor &color)
{
    if (fillColor() != color) {
        UMLApp::app()->executeCommand(new CmdChangeFillColor(this, color));
    }
}

/**
 * Sets the background fill color
 *
 * @param color the new fill color
 */
void UMLWidget::setFillColorCmd(const QColor &color)
{
    WidgetBase::setFillColor(color);
    update();
}

/**
 * Activate the object after serializing it from a QDataStream
 *
 * @param ChangeLog
 * @return  true for success
 */
bool UMLWidget::activate(IDChangeLog* /*ChangeLog  = 0 */)
{
    if (widgetHasUMLObject(baseType()) && m_umlObject == 0) {
        m_umlObject = m_doc->findObjectById(m_nId);
        if (m_umlObject == 0) {
            uError() << "cannot find UMLObject with id=" << Uml::ID::toString(m_nId);
            return false;
        }
    }
    setFontCmd(m_font);
    setSize(width(), height());
    m_activated = true;
    updateGeometry();
    if (m_scene->getPaste()) {
        FloatingTextWidget * ft = 0;
        QPointF point = m_scene->getPastePoint();
        int x = point.x() + this->x();
        int y = point.y() + this->y();
        if (m_scene->type() == Uml::DiagramType::Sequence) {
            switch (baseType()) {
            case WidgetBase::wt_Object:
            case WidgetBase::wt_Precondition :
                setY(this->y());
                setX(x);
                break;

            case WidgetBase::wt_Message:
                setY(this->y());
                setX(x);
                break;

            case WidgetBase::wt_Text:
                ft = static_cast<FloatingTextWidget *>(this);
                if (ft->textRole() == Uml::TextRole::Seq_Message) {
                    setX(x);
                    setY(this->y());
                } else {
                    setX(this->x());
                    setY(this->y());
                }
                break;

            default:
                setY(y);
                break;
            }//end switch base type
        }//end if sequence
        else {
            setX(x);
            setY(y);
        }
    }//end if pastepoint
    else {
        setX(this->x());
        setY(this->y());
    }
    if (m_scene->getPaste())
        m_scene->createAutoAssociations(this);
    updateGeometry();
    return true;
}

/**
 * Returns true if the Activate method has been called for this instance
 *
 * @return The activate status.
 */
bool UMLWidget::isActivated() const
{
    return m_activated;
}

/**
 * Set the m_activated flag of a widget but does not perform the Activate method
 *
 * @param active  Status of activation is to be set.
 */
void UMLWidget::setActivated(bool active /*=true*/)
{
    m_activated = active;
}

/**
 * Adds an already created association to the list of
 * associations that include this UMLWidget
 */
void UMLWidget::addAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc && !m_Assocs.contains(pAssoc)) {
        m_Assocs.append(pAssoc);
    }
}

/**
 * Removes an already created association from the list of
 * associations that include this UMLWidget
 */
void UMLWidget::removeAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc) {
        m_Assocs.removeAll(pAssoc);
    }
}

/**
 * Adjusts associations with the given co-ordinates
 *
 * @param dx  The amount by which the widget moved in X direction.
 * @param dy  The amount by which the widget moved in Y direction.
 */
void UMLWidget::adjustAssocs(qreal dx, qreal dy)
{
    // don't adjust Assocs on file load, as
    // the original positions, which are stored in XMI
    // should be reproduced exactly
    // (don't try to reposition assocs as long
    //   as file is only partly loaded -> reposition
    //   could be misguided)
    /// @todo avoid trigger of this event during load
    if (m_doc->loading()) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }

    foreach(AssociationWidget* assocwidget, m_Assocs) {
        assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget, m_Assocs) {
        assocwidget->widgetMoved(this, dx, dy);
    }
}

/**
 * Adjusts all unselected associations with the given co-ordinates
 *
 * @param dx  The amount by which the widget moved in X direction.
 * @param dy  The amount by which the widget moved in Y direction.
 */
void UMLWidget::adjustUnselectedAssocs(qreal dx, qreal dy)
{
    foreach(AssociationWidget* assocwidget, m_Assocs) {
        if (!assocwidget->isSelected())
            assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget, m_Assocs) {
        if (!assocwidget->isSelected()) {
            assocwidget->widgetMoved(this, dx, dy);
        }
    }
}

/**
 * Show a properties dialog for a UMLWidget.
 */
void UMLWidget::showPropertiesDialog()
{
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    UMLApp::app()->docWindow()->updateDocumentation(false);
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        UMLApp::app()->docWindow()->showDocumentation(umlObject(), true);
        m_doc->setModified(true);
    }
    dlg->close(); //wipe from memory
    delete dlg;
}

/**
 * Move the widget by an X and Y offset relative to
 * the current position.
 */
void UMLWidget::moveByLocal(qreal dx, qreal dy)
{
    qreal newX = x() + dx;
    qreal newY = y() + dy;
    setX(newX);
    setY(newY);
    adjustAssocs(dx, dy);
}

/**
 * Set the pen.
 */
void UMLWidget::setPenFromSettings(QPainter & p)
{
    p.setPen(QPen(m_lineColor, m_lineWidth));
}

/**
 * Set the pen.
 */
void UMLWidget::setPenFromSettings(QPainter *p)
{
    p->setPen(QPen(m_lineColor, m_lineWidth));
}

/**
 * Returns the cursor to be shown when resizing the widget.
 * Default cursor is KCursor::sizeFDiagCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor UMLWidget::resizeCursor() const
{
    return Qt::SizeFDiagCursor;
}

/**
 * Checks if the mouse is in resize area (right bottom corner), and sets
 * the cursor depending on that.
 * The cursor used when resizing is gotten from resizeCursor().
 *
 * @param me The QMouseEVent to check.
 * @return true if the mouse is in resize area, false otherwise.
 */
bool UMLWidget::isInResizeArea(QGraphicsSceneMouseEvent *me)
{
    qreal m = 10.0;
    const qreal w = width();
    const qreal h = height();

    // If the widget itself is very small then make the resize area small, too.
    // Reason: Else it becomes impossible to do a move instead of resize.
    if (w - m < m || h - m < m) {
        m = 2.0;
    }

    if (m_resizable &&
            me->scenePos().x() >= (x() + w - m) &&
            me->scenePos().y() >= (y() + h - m)) {
        m_scene->activeView()->setCursor(resizeCursor());
        return true;
    } else {
        m_scene->activeView()->setCursor(Qt::ArrowCursor);
        return false;
    }
}

/**
 * calculate content related size of widget.
 *
 * @return calculated widget size
 */
QSizeF UMLWidget::calculateSize(bool withExtensions /* = true */) const
{
    Q_UNUSED(withExtensions)
    return QSizeF(width(), height());
}

/**
 * Resize widget to minimum size.
 */
void UMLWidget::resize()
{
    qreal oldW = width();
    qreal oldH = height();
    // @TODO minimumSize() do not work in all cases, we need a dedicated autoResize() method
    QSizeF size = minimumSize();
    setSize(size.width(), size.height());
    DEBUG(DBG_SRC) << "size=" << size;
    adjustAssocs(size.width()-oldW, size.height()-oldH);
}

/**
 * Resizes the widget and adjusts the associations.
 * It's called when a mouse move event happens and the cursor was
 * in resize area when pressed.
 * Resizing can be constrained to an specific axis using control and shift buttons.
 *
 * @param me The QGraphicsSceneMouseEvent to get the values from.
 */
void UMLWidget::resize(QGraphicsSceneMouseEvent *me)
{
    // TODO the status message lies for at least MessageWidget which could only be resized vertical
    UMLApp::app()->document()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel resize."));

    m_resized = true;

    qreal newW = m_oldW + me->scenePos().x() - x() - m_pressOffset.x();
    qreal newH = m_oldH + me->scenePos().y() - y() - m_pressOffset.y();

    if ((me->modifiers() & Qt::ShiftModifier) && (me->modifiers() & Qt::ControlModifier)) {
        //Move in Y axis
        newW = m_oldW;
    } else if ((me->modifiers() & Qt::ShiftModifier) || (me->modifiers() & Qt::ControlModifier)) {
        //Move in X axis
        newH = m_oldH;
    }

    constrain(newW, newH);
    resizeWidget(newW, newH);
    DEBUG(DBG_SRC) << "event=" << me->scenePos() << "/ pos=" << pos() << " / newW=" << newW << " / newH=" << newH;
    QPointF delta = me->scenePos() - me->lastScenePos();
    adjustAssocs(delta.x(), delta.y());

    m_scene->resizeSceneToItems();
}

/**
 * Checks if the size of the widget changed respect to the size that
 * it had when press event was fired.
 *
 * @return true if was resized, false otherwise.
 */
bool UMLWidget::wasSizeChanged()
{
    return m_oldW != width() || m_oldH != height();
}

/**
 * Checks if the position of the widget changed respect to the position that
 * it had when press event was fired.
 *
 * @return true if was moved, false otherwise.
 */
bool UMLWidget::wasPositionChanged()
{
    return m_oldPos != pos();
}

/**
 * Fills m_selectedWidgetsList and sets the selection bounds ((m_min/m_max)X/Y attributes).
 */
void UMLWidget::setSelectionBounds()
{
}

void UMLWidget::setSelectedFlag(bool _select)
{
    WidgetBase::setSelected(_select);
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select The state of whether the widget is selected.
 */
void UMLWidget::setSelected(bool _select)
{
    WidgetBase::setSelected(_select);
    const WidgetBase::WidgetType wt = baseType();
    if (_select) {
        if (m_scene->selectedCount() == 0) {
            if (widgetHasUMLObject(wt)) {
                UMLApp::app()->docWindow()->showDocumentation(m_umlObject, false);
            } else {
                UMLApp::app()->docWindow()->showDocumentation(this, false);
            }
        }//end if
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(9);//keep text on top and boxes behind so don't touch Z value
        } */
    } else {
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(m_origZ);
        } */
        if (isSelected())
            UMLApp::app()->docWindow()->updateDocumentation(true);
    }

    // TODO: isn't this handled by toForeground() ?
    const QPoint pos(x(), y());
    UMLWidget *bkgnd = m_scene->widgetAt(pos);
    if (bkgnd && bkgnd != this && _select) {
        DEBUG(DBG_SRC) << "setting Z to " << bkgnd->zValue() + 1.0 << ", SelectState: " << _select;
        setZValue(bkgnd->zValue() + 1.0);
    }

    update();

    // selection changed, we have to make sure the copy and paste items
    // are correctly enabled/disabled
    UMLApp::app()->slotCopyChanged();

    // select in tree view as done for diagrams
    if (_select) {
        UMLListViewItem * item = UMLApp::app()->listView()->findItem(id());
        if (item)
            UMLApp::app()->listView()->setCurrentItem(item);
        else
            UMLApp::app()->listView()->clearSelection();
    }
}

/**
 * Selects the widget and clears the other selected widgets, if any.
 *
 * @param me The QGraphicsSceneMouseEvent which made the selection.
 */
void UMLWidget::selectSingle(QGraphicsSceneMouseEvent *me)
{
    m_scene->clearSelected();

    // Adds the widget to the selected widgets list, but as it has been cleared
    // only the current widget is selected.
    selectMultiple(me);
}

/**
 * Selects the widget and adds it to the list of selected widgets.
 *
 * @param me The QGraphicsSceneMouseEvent which made the selection.
 */
void UMLWidget::selectMultiple(QGraphicsSceneMouseEvent *me)
{
    Q_UNUSED(me);

    setSelected(true);
}

/**
 * Deselects the widget and removes it from the list of selected widgets.
 *
 * @param me The QGraphicsSceneMouseEvent which made the selection.
 */
void UMLWidget::deselect(QGraphicsSceneMouseEvent *me)
{
    Q_UNUSED(me);

    setSelected(false);
}

/**
 * Clears the selection, resets the toolbar and deselects the widget.
 */
//void UMLWidget::resetSelection()
//{
//    m_scene->clearSelected();
//    m_scene->resetToolbar();
//    setSelected(false);
//}

/**
 * Sets the view the widget is on.
 *
 * @param scene  The UMLScene the widget is on.
 */
void UMLWidget::setScene(UMLScene *scene)
{
    //remove signals from old view - was probably 0 anyway
    disconnect(m_scene, SIGNAL(sigFillColorChanged(Uml::ID::Type)), this, SLOT(slotFillColorChanged(Uml::ID::Type)));
    disconnect(m_scene, SIGNAL(sigTextColorChanged(Uml::ID::Type)), this, SLOT(slotTextColorChanged(Uml::ID::Type)));
    disconnect(m_scene, SIGNAL(sigLineWidthChanged(Uml::ID::Type)), this, SLOT(slotLineWidthChanged(Uml::ID::Type)));
    m_scene = scene;
    connect(m_scene, SIGNAL(sigFillColorChanged(Uml::ID::Type)), this, SLOT(slotFillColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigTextColorChanged(Uml::ID::Type)), this, SLOT(slotTextColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigLineWidthChanged(Uml::ID::Type)), this, SLOT(slotLineWidthChanged(Uml::ID::Type)));
}

/**
 * Sets the x-coordinate.
 * Currently, the only class that reimplements this method is
 * ObjectWidget.
 *
 * @param x The x-coordinate to be set.
 */
void UMLWidget::setX(qreal x)
{
    QGraphicsObject::setX(x);
}

/**
 * Sets the y-coordinate.
 * Currently, the only class that reimplements this method is
 * ObjectWidget.
 *
 * @param y The y-coordinate to be set.
 */
void UMLWidget::setY(qreal y)
{
    QGraphicsObject::setY(y);
}

/**
 * Used to cleanup any other widget it may need to delete.
 * Used by child classes.  This should be called before deleting a widget of a diagram.
 */
void UMLWidget::cleanup()
{
}

/**
 * Tells the widget to snap to grid.
 * Will use the grid settings of the @ref UMLView it belongs to.
 */
void UMLWidget::slotSnapToGrid()
{
    if (!m_ignoreSnapToGrid) {
        qreal newX = m_scene->snappedX(x());
        setX(newX);
        qreal newY = m_scene->snappedY(y());
        setY(newY);
    }
}

/**
 * Returns whether the widget type has an associated UMLObject
 */
bool UMLWidget::widgetHasUMLObject(WidgetBase::WidgetType type)
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
 * Set m_ignoreSnapToGrid.
 */
void UMLWidget::setIgnoreSnapToGrid(bool to)
{
    m_ignoreSnapToGrid = to;
}

/**
 * Return the value of m_ignoreSnapToGrid.
 */
bool UMLWidget::getIgnoreSnapToGrid() const
{
    return m_ignoreSnapToGrid;
}

/**
 * Sets the size.
 * If m_scene->snapComponentSizeToGrid() is true, then
 * set the next larger size that snaps to the grid.
 */
void UMLWidget::setSize(qreal width, qreal height)
{
    // snap to the next larger size that is a multiple of the grid
    if (!m_ignoreSnapComponentSizeToGrid
            && m_scene->snapComponentSizeToGrid()) {
        // integer divisions
        int numX = width / m_scene->snapX();
        int numY = height / m_scene->snapY();
        // snap to the next larger valid value
        if (width > numX * m_scene->snapX())
            width = (numX + 1) * m_scene->snapX();
        if (height > numY * m_scene->snapY())
            height = (numY + 1) * m_scene->snapY();
    }

    const QRectF newRect(rect().x(), rect().y(), width, height);
    setRect(newRect);
    foreach(QGraphicsItem* child, childItems()) {
        UMLWidget* umlChild = static_cast<UMLWidget*>(child);
        umlChild->notifyParentResize();
    }
}

/**
 * Sets the size with another size.
 */
void UMLWidget::setSize(const QSizeF& size)
{
    setSize(size.width(), size.height());
}

/**
 * Update the size of this widget.
 */
void UMLWidget::updateGeometry()
{
    if (m_doc->loading()) {
        return;
    }
    if (!m_autoResize)
        return;
    qreal oldW = width();
    qreal oldH = height();
    QSizeF size = calculateSize();
    qreal clipWidth = size.width();
    qreal clipHeight = size.height();
    constrain(clipWidth, clipHeight);
    setSize(clipWidth, clipHeight);
    slotSnapToGrid();
    adjustAssocs(size.width()-oldW, size.height()-oldH);
}

/**
 * clip the size of this widget against the
 * minimal and maximal limits.
 */
void UMLWidget::clipSize()
{
    qreal clipWidth = width();
    qreal clipHeight = height();
    constrain(clipWidth, clipHeight);
    setSize(clipWidth, clipHeight);
}

/**
 * Template Method, override this to set the default font metric.
 */
void UMLWidget::setDefaultFontMetrics(QFont &font, UMLWidget::FontType fontType)
{
    setupFontType(font, fontType);
    setFontMetrics(fontType, QFontMetrics(font));
}

void UMLWidget::setupFontType(QFont &font, UMLWidget::FontType fontType)
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

void UMLWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (option->state & QStyle::State_Selected) {
        const qreal w = width();
        const qreal h = height();
        const qreal s = 4;
        QBrush brush(Qt::blue);
        painter->fillRect(0, 0, s,  s, brush);
        painter->fillRect(0, 0 + h - s, s, s, brush);
        painter->fillRect(0 + w - s, 0, s, s, brush);

        // Draw the resize anchor in the lower right corner.
        // Don't draw it if the widget is so small that the
        // resize anchor would cover up most of the widget.
        if (m_resizable && w >= s+8 && h >= s+8) {
            brush.setColor(Qt::red);
            const int right = 0 + w;
            const int bottom = 0 + h;
            painter->drawLine(right - s, 0 + h - 1, 0 + w - 1, 0 + h - s);
            painter->drawLine(right - (s*2), bottom - 1, right - 1, bottom - (s*2));
            painter->drawLine(right - (s*3), bottom - 1, right - 1, bottom - (s*3));
        } else {
            painter->fillRect(0 + w - s, 0 + h - s, s, s, brush);
        }
        // debug info
        if (Tracer::instance()->isEnabled(QLatin1String(metaObject()->className()))) {
            painter->setPen(Qt::green);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(shape());
            painter->setPen(Qt::red);
            painter->drawRect(boundingRect());
            // origin
            painter->drawLine(-10, 0, 10, 0);
            painter->drawLine(0, -10, 0, 10);
        }
    }

    if (umlScene()->isShowDocumentationIndicator() && hasDocumentation()) {
        const qreal h = height();
        const qreal d = 8;
        QPolygonF p;
        p << QPointF(0, h - d) << QPointF(d, h) << QPointF(0, h);
        painter->setPen(Qt::blue);
        painter->setBrush(Qt::red);
        painter->drawPolygon(p);
    }
}

/**
 * Template Method, override this to set the default font metric.
 */
void UMLWidget::setDefaultFontMetrics(QFont &font, UMLWidget::FontType fontType, QPainter &painter)
{
    setupFontType(font, fontType);
    painter.setFont(font);
    setFontMetrics(fontType, painter.fontMetrics());
}

/**
 * Returns the font metric used by this object for Text
 * which uses bold/italic fonts.
 */
QFontMetrics &UMLWidget::getFontMetrics(UMLWidget::FontType fontType) const
{
    return *m_pFontMetrics[fontType];
}

/**
 * Set the font metric to use.
 */
void UMLWidget::setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm)
{
    delete m_pFontMetrics[fontType];
    m_pFontMetrics[fontType] = new QFontMetrics(fm);
}

/**
 * Sets the font the widget is to use.
 *
 * @param font Font to be set.
 */
void UMLWidget::setFont(const QFont &font)
{
    QFont newFont = font;
    forceUpdateFontMetrics(newFont, 0);

    if (m_font != newFont) {
        UMLApp::app()->executeCommand(new CmdChangeFont(this, font));
    }
}

/**
 * Sets the font the widget is to use.
 *
 * @param font Font to be set.
 */
void UMLWidget::setFontCmd(const QFont &font)
{
    WidgetBase::setFont(font);
    forceUpdateFontMetrics(0);
    if (m_doc->loading())
        return;
    update();
}

/**
 * Updates font metrics for widgets current m_font
 */
void UMLWidget::forceUpdateFontMetrics(QPainter *painter)
{
    forceUpdateFontMetrics(m_font, painter);
}

/**
 * @note For performance Reasons, only FontMetrics for already used
 *  font types are updated. Not yet used font types will not get a font metric
 *  and will get the same font metric as if painter was zero.
 *  This behaviour is acceptable, because diagrams will always be shown on Display
 *  first before a special painter like a printer device is used.
 */
void UMLWidget::forceUpdateFontMetrics(QFont& font, QPainter *painter)
{
    if (painter == 0) {
        for (int i = (int)FT_INVALID - 1; i >= 0; --i) {
            if (m_pFontMetrics[(UMLWidget::FontType)i] != 0)
                setDefaultFontMetrics(font, (UMLWidget::FontType)i);
        }
    } else {
        for (int i2 = (int)FT_INVALID - 1; i2 >= 0; --i2) {
            if (m_pFontMetrics[(UMLWidget::FontType)i2] != 0)
                setDefaultFontMetrics(font, (UMLWidget::FontType)i2, *painter);
        }
    }
    if (m_doc->loading())
        return;
    // calculate the size, based on the new font metric
    updateGeometry();
}

/**
 * Set the status of whether to show Stereotype.
 *
 * @param flag   True if stereotype shall be shown.
 */
void UMLWidget::setShowStereotype(bool flag)
{
    m_showStereotype = flag;
    updateGeometry();
    update();
}

/**
 * Returns the status of whether to show Stereotype.
 *
 * @return  True if stereotype is shown.
 */
bool UMLWidget::showStereotype() const
{
    return m_showStereotype;
}

/**
 * Overrides the standard operation.
 *
 * @param me The move event.
 */
void UMLWidget::moveEvent(QGraphicsSceneMouseEvent* me)
{
  Q_UNUSED(me)
}

void UMLWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    /*
      Call after required actions in child class.
      Type must be set in the child class.
    */
    WidgetBase::saveToXMI(qDoc, qElement);
    qElement.setAttribute(QLatin1String("xmi.id"), Uml::ID::toString(id()));

    qreal dpiScale = UMLApp::app()->document()->dpiScale();
    qElement.setAttribute(QLatin1String("x"), QString::number(x() / dpiScale));
    qElement.setAttribute(QLatin1String("y"), QString::number(y() / dpiScale));
    qElement.setAttribute(QLatin1String("width"), QString::number(width() / dpiScale));
    qElement.setAttribute(QLatin1String("height"), QString::number(height() / dpiScale));

    qElement.setAttribute(QLatin1String("isinstance"), m_isInstance);
    if (!m_instanceName.isEmpty())
        qElement.setAttribute(QLatin1String("instancename"), m_instanceName);
    if (m_showStereotype)
        qElement.setAttribute(QLatin1String("showstereotype"), m_showStereotype);

    // Unique identifier for widget (todo: id() should be unique, new attribute
    // should indicate the UMLObject's ID it belongs to)
    qElement.setAttribute(QLatin1String("localid"), Uml::ID::toString(m_nLocalID));
}

bool UMLWidget::loadFromXMI(QDomElement & qElement)
{
    QString id = qElement.attribute(QLatin1String("xmi.id"), QLatin1String("-1"));
    m_nId = Uml::ID::fromString(id);

    WidgetBase::loadFromXMI(qElement);
    QString x = qElement.attribute(QLatin1String("x"), QLatin1String("0"));
    QString y = qElement.attribute(QLatin1String("y"), QLatin1String("0"));
    QString h = qElement.attribute(QLatin1String("height"), QLatin1String("0"));
    QString w = qElement.attribute(QLatin1String("width"), QLatin1String("0"));
    qreal dpiScale = UMLApp::app()->document()->dpiScale();
    setSize(toDoubleFromAnyLocale(w) * dpiScale,
            toDoubleFromAnyLocale(h) * dpiScale);
    setX(toDoubleFromAnyLocale(x) * dpiScale);
    setY(toDoubleFromAnyLocale(y) * dpiScale);

    QString isinstance = qElement.attribute(QLatin1String("isinstance"), QLatin1String("0"));
    m_isInstance = (bool)isinstance.toInt();
    m_instanceName = qElement.attribute(QLatin1String("instancename"));
    QString showstereo = qElement.attribute(QLatin1String("showstereotype"), QLatin1String("0"));
    m_showStereotype = (bool)showstereo.toInt();

    QString localid = qElement.attribute(QLatin1String("localid"), QLatin1String("0"));
    if (localid != QLatin1String("0")) {
        m_nLocalID = Uml::ID::fromString(localid);
    }

    return true;
}

