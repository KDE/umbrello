/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlwidget.h"

// local includes
#include "artifact.h"
#include "artifactwidget.h"
#include "activitywidget.h"
#include "actor.h"
#include "actorwidget.h"
#include "associationwidget.h"
#include "classifier.h"
#include "classpropertiesdialog.h"
#include "cmds.h"
#include "component.h"
#include "componentwidget.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "docwindow.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "interfacewidget.h"
#include "notewidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "object_factory.h"
#include "idchangelog.h"
#include "menus/listpopupmenu.h"
#include "objectnodewidget.h"
#include "pinwidget.h"
#include "port.h"
#include "portwidget.h"
#include "regionwidget.h"
#include "signalwidget.h"
#include "settingsdialog.h"
#include "statewidget.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobject.h"
#include "umlscene.h"
#include "umlview.h"
#include "usecase.h"
#include "usecasewidget.h"
#include "uniqueid.h"
#include "widget_factory.h"
#include "widget_utils.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QPointer>
#include <QXmlStreamWriter>

using namespace Uml;

DEBUG_REGISTER_DISABLED(UMLWidget)

#define I18N_NEXT_RELEASE(a,b) QString(QLatin1String(a)).arg(b))

const QSizeF UMLWidget::DefaultMinimumSize(50, 20);
const QSizeF UMLWidget::DefaultMaximumSize(1000, 5000);
const int UMLWidget::defaultMargin = 5;
const int UMLWidget::selectionMarkerSize = 4;
const int UMLWidget::resizeMarkerLineCount = 3;


/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param type  The WidgetType to construct.
 *              This must be set to the appropriate value by the constructors of inheriting classes.
 * @param o The UMLObject to represent.
 * @note Although a pointer to the scene is required, the widget is not added to the scene by default.
 */
UMLWidget::UMLWidget(UMLScene * scene, WidgetType type, UMLObject * o)
  : WidgetBase(scene, type, o ? o->id() : Uml::ID::None)
  , DiagramProxyWidget(this)
{
    init();
    m_umlObject = o;
    if (m_umlObject) {
        // TODO: calling WidgetBase::setUMLObject does not add this connection
        connect(m_umlObject, SIGNAL(modified()), this, SLOT(updateWidget()));
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
  : WidgetBase(scene, type, id)
  , DiagramProxyWidget(this)
{
    init();
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
    DiagramProxyWidget::operator=(other);

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
       The type and ID should uniquely identify a UMLWidget.
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
    logDebug2("UMLWidget %1 toForeground: items at rect = %2", name(), items.count());
    if (items.count() > 1) {
        foreach(QGraphicsItem* i, items) {
            UMLWidget* w = dynamic_cast<UMLWidget*>(i);
            if (w) {
                logDebug2("- item=%1 with zValue=%2", w->name(), w->zValue());
                if (w->name() != name()) {
                    if (w->zValue() >= zValue()) {
                        setZValue(w->zValue() + 1.0);
                        logDebug1("-- bring to foreground with zValue: %1", zValue());
                    }
                }
            }
        }
    }
    else {
        setZValue(0.0);
    }
    logDebug2("UMLWidget %1 toForeground: zValue is %1", name(), zValue());
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
    /*
    logDebug4("UMLWidget::mousePressEvent: widget=%1 / type=%2 / event->scenePos=%3 / pos=%4",
              name(), baseTypeStr(), event->scenePos(), pos());
     */
    event->accept();
    logDebug2("UMLWidget::mousePressEvent: widget = %1 / type = %2", name(), baseTypeStr());

    toForeground();

    m_startMovePostion = pos();
    m_startResizeSize = QSizeF(width(), height());

    // saving the values of the widget
    m_pressOffset = event->scenePos() - pos();
    logDebug2("UMLWidget::mousePressEvent: press offset x=%1, y=%2", m_pressOffset.x(), m_pressOffset.y());

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

    int count = m_scene->selectedCount();
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
 * and shift buttons. If one or another is pressed, it's constrained to X axis.
 * If both are pressed, it's constrained to Y axis.
 *
 * If not resizing, the widget is being moved. If the move is being started,
 * the selection bounds are set (which includes updating the list of selected
 * widgets).
 * The difference between the previous position of the selection and the new
 * one is calculated (taking in account the selection bounds so widgets don't go
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
        UMLApp::app()->document()->writeToStatusBar
          (i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel move."));

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

    logDebug2("UMLWidget::mouseMoveEvent: diffX=%1 / diffY=%2", diffX, diffY);
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
    logDebug0("UMLWidget::mouseReleaseEvent");
    if (!m_moved && !m_resized) {
        if (!m_shiftPressed && (m_scene->selectedCount() > 1)) {
            selectSingle(event);
        } else if (!isSelected()) {
            deselect(event);
        } else {
            Widget_Utils::ensureNestedVisible(this, umlScene()->widgetList());
        }
    } else {
        // Commands
        if (m_moved) {
            UMLWidgetList selectedWidgets = umlScene()->selectedWidgets();
            int selectionCount = selectedWidgets.count();
            if (selectionCount > 1) {
                UMLApp::app()->beginMacro(i18n("Move widgets"));
            }
            foreach (UMLWidget* widget, selectedWidgets) {
                UMLApp::app()->executeCommand(new Uml::CmdMoveWidget(widget));
                Widget_Utils::ensureNestedVisible(widget, umlScene()->widgetList());
            }
            if (selectionCount > 1) {
                UMLApp::app()->endMacro();
            }
            m_moved = false;
        } else {
            UMLApp::app()->executeCommand(new Uml::CmdResizeWidget(this));
            m_autoResize = false;
            m_resized = false;
            deselect(event);
        }

        if ((m_inMoveArea && wasPositionChanged()) ||
                (m_inResizeArea && wasSizeChanged())) {
            umlDoc()->setModified(true);
            umlScene()->invalidate();
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
        logDebug2("UMLWidget::mouseDoubleClickEvent: widget = %1 / type = %2", name(), baseTypeStr());
        showPropertiesDialog();
        event->accept();
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
        logDebug3("UMLWidget::constrain(%1) : Changing input height %2 to %3 due to fixedAspectRatio",
                  name(), height, (width * aspectRatio));
        height = width * aspectRatio;
    }
}

/**
 * Initializes key attributes of the class.
 */
void UMLWidget::init()
{
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
        logError0("UMLWidget::init SERIOUS PROBLEM - m_scene is null");
        m_useFillColor = false;
        m_usesDiagramFillColor = false;
        m_usesDiagramUseFillColor = false;
        m_showStereotype = Uml::ShowStereoType::None;
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

    case ListPopupMenu::mt_FloatText: {
        FloatingTextWidget* ft = new FloatingTextWidget(umlScene());
        ft->showChangeTextDialog();
        //if no text entered delete
        if (!FloatingTextWidget::isTextValid(ft->text())) {
            delete ft;
        } else {
            ft->setID(UniqueID::gen());
            addWidget(ft, false);
        }
        break;
    }

    case ListPopupMenu::mt_Actor: {
        UMLActor *actor = new UMLActor;
        UMLWidget *widget = new ActorWidget(umlScene(), actor);
        addConnectedWidget(widget, Uml::AssociationType::Association);
        break;
    }

    case ListPopupMenu::mt_Artifact: {
        UMLArtifact *a = new UMLArtifact();
        ArtifactWidget *widget = new ArtifactWidget(umlScene(), a);
        addConnectedWidget(widget, Uml::AssociationType::Association);
        break;
    }

    case ListPopupMenu::mt_Component: {
        UMLComponent *c = new UMLComponent();
        ComponentWidget *widget = new ComponentWidget(umlScene(), c);
        addConnectedWidget(widget, Uml::AssociationType::Association, SetupSize);
        break;
    }

    case ListPopupMenu::mt_Hide_Destruction_Box: {
        ObjectWidget *w = asObjectWidget();
        if (w)
            w->setShowDestruction(false);
        break;
    }

    case ListPopupMenu::mt_Show_Destruction_Box: {
        ObjectWidget *w = asObjectWidget();
        if (w)
            w->setShowDestruction(true);
        break;
    }

    case ListPopupMenu::mt_Interface: {
        UMLPackage* component = umlObject()->asUMLPackage();
        QString name = Model_Utils::uniqObjectName(UMLObject::ot_Interface, component);
        if (Dialog_Utils::askNewName(WidgetBase::wt_Interface, name)) {
            UMLClassifier *c = new UMLClassifier();
            c->setBaseType(UMLObject::ot_Interface);
            ClassifierWidget *widget = new ClassifierWidget(umlScene(), c);
            addConnectedWidget(widget, Uml::AssociationType::Association);
        }
        break;
    }

    case ListPopupMenu::mt_InterfaceComponent:
    case ListPopupMenu::mt_InterfaceProvided: {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Interface);
        InterfaceWidget *w = new InterfaceWidget(umlScene(), o->asUMLClassifier());
        w->setDrawAsCircle(true);
        addConnectedWidget(w, Uml::AssociationType::Association, SetupSize);
        break;
    }

    case ListPopupMenu::mt_InterfaceRequired: {
        UMLObject *o = Object_Factory::createUMLObject(UMLObject::ot_Interface);
        InterfaceWidget *w = new InterfaceWidget(umlScene(), o->asUMLClassifier());
        w->setDrawAsCircle(true);
        addConnectedWidget(w, Uml::AssociationType::Association, SetupSize | SwitchDirection);
        break;
    }

    case ListPopupMenu::mt_Note: {
        NoteWidget *widget = new NoteWidget(umlScene());
        addConnectedWidget(widget, Uml::AssociationType::Anchor);
        break;
    }

    case ListPopupMenu::mt_Port: {
        // TODO: merge with ToolbarStateOneWidget::setWidget()
        UMLPackage* component = umlObject()->asUMLPackage();
        QString name = Model_Utils::uniqObjectName(UMLObject::ot_Port, component);
        if (Dialog_Utils::askNewName(WidgetBase::wt_Port, name)) {
            UMLPort *port = Object_Factory::createUMLObject(UMLObject::ot_Port, name, component)->asUMLPort();
            UMLWidget *umlWidget = Widget_Factory::createWidget(umlScene(), port);
            umlWidget->setParentItem(this);
            QPointF p = mapFromScene(umlScene()->pos());
            umlWidget->setPos(p);
            umlScene()->setupNewWidget(umlWidget, false);

        }
        break;
    }

    case ListPopupMenu::mt_UseCase: {
        UMLUseCase *useCase = new UMLUseCase;
        UMLWidget *widget = new UseCaseWidget(umlScene(), useCase);
        addConnectedWidget(widget, Uml::AssociationType::Association);
        break;
    }

    case ListPopupMenu::mt_MessageCreation:
    case ListPopupMenu::mt_MessageDestroy:
    case ListPopupMenu::mt_MessageSynchronous:
//        MessageWidget *widget = new MessageWidget(umlScene(), this);
//        addConnectedWidget(widget, Uml::AssociationType::Coll_Mesg_Sync);
    case ListPopupMenu::mt_MessageAsynchronous:
    case ListPopupMenu::mt_MessageFound:
    case ListPopupMenu::mt_MessageLost:
        break;

    // activity diagrams
    case ListPopupMenu::mt_Accept_Signal:
        addConnectedWidget(new SignalWidget(umlScene(), SignalWidget::Accept), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Accept_Time_Event:
        addConnectedWidget(new SignalWidget(umlScene(), SignalWidget::Time), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Normal), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Activity_Transition:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Final), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Branch:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Branch), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Exception:
        umlScene()->triggerToolbarButton(WorkToolBar::tbb_Exception);
        break;
    case ListPopupMenu::mt_Final_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Final), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Fork:
        addConnectedWidget(new ForkJoinWidget(umlScene()), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_End_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::End), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Initial_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Initial), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Invoke_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Invok), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Object_Node:
        addConnectedWidget(new ObjectNodeWidget(umlScene(), ObjectNodeWidget::Data), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Pin:
        umlScene()->triggerToolbarButton(WorkToolBar::tbb_Pin);
        break;
    case ListPopupMenu::mt_Param_Activity:
        addConnectedWidget(new ActivityWidget(umlScene(), ActivityWidget::Param), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_PrePostCondition:
        addConnectedWidget(new NoteWidget(umlScene(), NoteWidget::Normal), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Region:
        addConnectedWidget(new RegionWidget(umlScene()), Uml::AssociationType::Activity, NoOption);
        break;
    case ListPopupMenu::mt_Send_Signal:
        addConnectedWidget(new SignalWidget(umlScene(), SignalWidget::Send), Uml::AssociationType::Activity, NoOption);
        break;

  // state diagrams
    case ListPopupMenu::mt_Choice:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::Choice), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_DeepHistory:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::DeepHistory), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_End_State:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::End), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_Junction:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::Junction), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_ShallowHistory:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::ShallowHistory), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_State:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::Normal), Uml::AssociationType::State, ShowProperties);
        break;
    case ListPopupMenu::mt_StateFork:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::Fork), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_StateJoin:
        addConnectedWidget(new StateWidget(umlScene(), StateWidget::Join), Uml::AssociationType::State, NoOption);
        break;
    case ListPopupMenu::mt_StateTransition:
        umlScene()->triggerToolbarButton(WorkToolBar::tbb_State_Transition);
        break;
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
 * Reimplemented from class WidgetBase
 *
 * @param ChangeLog
 * @return  true for success
 */
bool UMLWidget::activate(IDChangeLog* changeLog)
{
    if (!WidgetBase::activate(changeLog))
        return false;
    DiagramProxyWidget::activate(changeLog);

    setFontCmd(m_font);
    setSize(width(), height());
    m_activated = true;
    updateGeometry();
    if (m_scene->getPaste()) {
        FloatingTextWidget * ft = 0;
        QPointF point = m_scene->getPastePoint();
        int x = point.x() + this->x();
        int y = point.y() + this->y();
        if (m_scene->isSequenceDiagram()) {
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
 * Reimplemented from class WidgetBase
 */
void UMLWidget::addAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc && !associationWidgetList().contains(pAssoc)) {
        associationWidgetList().append(pAssoc);
    }
}

/**
 *  Returns the list of associations connected to this widget.
 */
AssociationWidgetList &UMLWidget::associationWidgetList() const
{
    m_Assocs.removeAll(0);
    return m_Assocs;
}

/**
 * Reimplemented from class WidgetBase
 */
void UMLWidget::removeAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc) {
        associationWidgetList().removeAll(pAssoc);
    }

    if (changesShape()) {
        updateGeometry();
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
    logDebug3("UMLWidget::adjustAssocs(%1) : w=%2, h=%3", name(), width(), height());
    // don't adjust Assocs on file load, as
    // the original positions, which are stored in XMI
    // should be reproduced exactly
    // (don't try to reposition assocs as long
    //   as file is only partly loaded -> reposition
    //   could be misguided)
    /// @todo avoid trigger of this event during load
    if (m_doc->loading() || (qFuzzyIsNull(dx) && qFuzzyIsNull(dy))) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }

    foreach(AssociationWidget* assocwidget, associationWidgetList()) {
        assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget, associationWidgetList()) {
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
    foreach(AssociationWidget* assocwidget, associationWidgetList()) {
        if (!assocwidget->isSelected())
            assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget, associationWidgetList()) {
        if (!assocwidget->isSelected() &&
               (this == assocwidget->widgetForRole(Uml::RoleType::A) ||
                this == assocwidget->widgetForRole(Uml::RoleType::B))) {
            assocwidget->widgetMoved(this, dx, dy);
        }
    }
}

/**
 * Show a properties dialog for a UMLWidget.
 */
bool UMLWidget::showPropertiesDialog()
{
    bool result = false;
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    UMLApp::app()->docWindow()->updateDocumentation(false);
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        UMLApp::app()->docWindow()->showDocumentation(umlObject(), true);
        m_doc->setModified(true);
        result = true;
    }
    dlg->close(); //wipe from memory
    delete dlg;
    return result;
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
 * Return true if `this' is located in the bounding rectangle of `other'.
 */
bool UMLWidget::isLocatedIn(const UMLWidget *other) const
{
    const QPointF pos = scenePos();
    const QPointF otherPos = other->scenePos();
    const QString msgProlog = QLatin1String("UMLWidget ") + name() +
                              QLatin1String(" isLocatedIn(") + other->name() + QLatin1String(")");

    if (otherPos.x() > pos.x() || otherPos.y() > pos.y()) {
        logDebug1("%1 returns false due to x or y out of range", msgProlog);
        return false;
    }

    const int endX = pos.x() + width();
    const int endY = pos.y() + height();

    if (otherPos.x() > endX || otherPos.y() > endY) {
        logDebug1("%1 returns false due to endX or endY out of range", msgProlog);
        return false;
    }

    const int otherEndX = otherPos.x() + other->width();
    if (otherEndX < pos.x() || otherEndX < endX) {
        logDebug1("%1 returns false due to otherEndX out of range", msgProlog);
        return false;
    }

    const int otherEndY = otherPos.y() + other->height();
    if (otherEndY < pos.y() || otherEndY < endY) {
        logDebug1("%1 returns false due to otherEndY out of range", msgProlog);
        return false;
    }

    logDebug2("UMLWidget %1 isLocatedIn(%2) returns true", name(), other->name());
    return true;
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
    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    if (m_umlObject) {
        qreal width = 0, height = defaultMargin;
        if (!m_umlObject->stereotype().isEmpty()) {
            height += fontHeight;
            const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
            const int stereoWidth = bfm.size(0, m_umlObject->stereotype(true)).width();
            if (stereoWidth > width)
                width = stereoWidth;
        }
        height += fontHeight;
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const int nameWidth = bfm.size(0, m_umlObject->name()).width();
        if (nameWidth > width)
            width = nameWidth;
        return QSizeF(width + 2*defaultMargin, height);
    }
    else
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
    logDebug3("UMLWidget::resize(%1) w=%2, h=%3", name(), size.width(), size.height());
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
    QString msgX = i18n("Hold shift or control to move in X axis.");
    QString msgY = i18n("Hold shift and control to move in Y axis.");
    QString msg;
    if (isMessageWidget())
        msg = msgY;
    else if (isObjectWidget())
        msg = msgX;
    else
        msg = QString(QLatin1String("%1 %2")).arg(msgX, msgY);
    UMLApp::app()->document()->writeToStatusBar(msg);

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
    DEBUG() << "event=" << me->scenePos() << "/ pos=" << pos() << " / newW=" << newW << " / newH=" << newH;
    QPointF delta = me->scenePos() - me->lastScenePos();
    adjustAssocs(delta.x(), delta.y());
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

    WidgetBase::setSelected(_select);

    logDebug1("UMLWidget::setSelected(%1) : Prevent obscuring", _select);
    Widget_Utils::ensureNestedVisible(this, umlScene()->widgetList());

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
 * Gets the x-coordinate.
 * Currently, the only class that reimplements this method is PinPortBase.
 *
 * @return The x-coordinate.
 */
qreal UMLWidget::getX() const
{
    return QGraphicsObjectWrapper::x();
}

/**
 * Gets the y-coordinate.
 * Currently, the only class that reimplements this method is PinPortBase.
 *
 * @return The y-coordinate.
 */
qreal UMLWidget::getY() const
{
    return QGraphicsObjectWrapper::y();
}

/**
 * Gets the position.
 * Currently, the only class that reimplements this method is PinPortBase.
 *
 * @return The QGraphicsObject position.
 */
QPointF UMLWidget::getPos() const
{
    return QGraphicsObjectWrapper::pos();
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
    if (x < -umlScene()->maxCanvasSize() || x > umlScene()->maxCanvasSize())
        logError1("UMLWidget::setX refusing to set X to %1", x);
    else
        QGraphicsObjectWrapper::setX(x);
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
    if (y < -umlScene()->maxCanvasSize() || y > umlScene()->maxCanvasSize())
        logError1("UMLWidget::setY refusing to set Y to %1", y);
    else
        QGraphicsObjectWrapper::setY(y);
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
 * If m_scene->snapComponentSizeToGrid() is true then
 * set the next larger size that snaps to the grid.
 */
void UMLWidget::setSize(qreal width, qreal height)
{
    // snap to the next larger size that is a multiple of the grid
    if (!m_ignoreSnapComponentSizeToGrid && m_scene
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
    logDebug3("UMLWidget::setSize(%1): setting w=%2, h=%3", name(), newRect.width(), newRect.height());
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
 *
 * @param withAssocs true - update associations too
 */
void UMLWidget::updateGeometry(bool withAssocs)
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
    logDebug5("UMLWidget::updateGeometry(%1) : oldW=%2, oldH=%3, clipWidth=%4, clipHeight=%5",
             name(), oldW, oldH, clipWidth, clipHeight);
    setSize(clipWidth, clipHeight);
    slotSnapToGrid();
    if (withAssocs)
        adjustAssocs(size.width()-oldW, size.height()-oldH);
    update();
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
        const qreal s = selectionMarkerSize;
        QBrush brush(Qt::blue);
        painter->fillRect(0, 0, s,  s, brush);
        painter->fillRect(0, 0 + h - s, s, s, brush);
        painter->fillRect(0 + w - s, 0, s, s, brush);

        // Draw the resize anchor in the lower right corner.
        // Don't draw it if the widget is so small that the
        // resize anchor would cover up most of the widget.
        if (m_resizable && w >= s+8 && h >= s+8) {
            brush.setColor(Qt::red);
            const int bottom = 0 + h;
            int horSide = w;   // horizontal side default: right side
            if (baseType() == wt_Message) {
               MessageWidget *msg = asMessageWidget();
               int x1 = msg->objectWidget(Uml::RoleType::A)->x();
               int x2 = msg->objectWidget(Uml::RoleType::B)->x();
               if (x1 > x2) {
                   // On messages running right to left we use the left side for
                   // placing the resize anchor because the message's execution
                   // specification as at the left in this case.  Furthermore,
                   // the right side may be covered up by another message's
                   // execution specification.
                   horSide = 17;  // execution box width
               }
            }
            painter->drawLine(horSide - s, 0 + h - 1, 0 + w - 1, 0 + h - s);
            painter->drawLine(horSide - (s*2), bottom - 1, horSide - 1, bottom - (s*2));
            painter->drawLine(horSide - (s*3), bottom - 1, horSide - 1, bottom - (s*3));
        } else {
            painter->fillRect(0 + w - s, 0 + h - s, s, s, brush);
        }
        // debug info
        if (Tracer::instance()->isEnabled(QLatin1String(metaObject()->className()))) {
            QPen p(Qt::green);
            p.setWidthF(1.0);
            painter->setPen(p);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(shape());
            painter->setPen(Qt::blue);
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
 * @param flag   Value of type Uml::ShowStereoType::Enum
 */
void UMLWidget::setShowStereotype(Uml::ShowStereoType::Enum flag)
{
    m_showStereotype = flag;
    updateGeometry();
    update();
}

/**
 * Return stereotype concrete attributes concatenated into single string
 * with the attribute name given before each value and delimited by "{"
 * at start and "}" at end.
 * Example:
 * For a stereotype with attribute 'foo' of type Double and attribute 'bar'
 * of type String and concrete values 1.0 for 'foo' and "hello" for 'bar',
 * the result is:  {foo=1.0,bar="hello"}
 */
QString UMLWidget::tags() const
{
    if (m_umlObject == 0)
        return QString();
    UMLStereotype *s = m_umlObject->umlStereotype();
    if (s == 0)
        return QString();
    UMLStereotype::AttributeDefs adefs = s->getAttributeDefs();
    if (adefs.isEmpty())
        return QString();
    const QStringList& umlTags = m_umlObject->tags();
    QString taggedValues(QLatin1String("{"));
    for (int i = 0; i < adefs.size(); i++) {
        UMLStereotype::AttributeDef ad = adefs.at(i);
        taggedValues.append(ad.name);
        taggedValues.append(QLatin1String("="));
        QString value = ad.defaultVal;
        if (i < umlTags.size()) {
            QString umlTag = umlTags.at(i);
            if (!umlTag.isEmpty())
                value = umlTag;
        }
        if (ad.type == Uml::PrimitiveTypes::String)
            value = QLatin1String("\"") + value + QLatin1String("\"");
        taggedValues.append(value);
        if (i < adefs.size() - 1)
            taggedValues.append(QLatin1String(","));
     }
     taggedValues.append(QLatin1String("}"));
     return taggedValues;
}

/**
 * Returns the status of whether to show Stereotype.
 *
 * @return  True if stereotype is shown.
 */
Uml::ShowStereoType::Enum UMLWidget::showStereotype() const
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

void UMLWidget::saveToXMI(QXmlStreamWriter& writer)
{
    /*
      When calling this from child classes bear in mind that the call
      must precede terminated XML subelements.
      Type must be set in the child class.
    */
    WidgetBase::saveToXMI(writer);
    DiagramProxyWidget::saveToXMI(writer);

    qreal dpiScale = UMLApp::app()->document()->dpiScale();
    writer.writeAttribute(QLatin1String("x"), QString::number(x() / dpiScale));
    writer.writeAttribute(QLatin1String("y"), QString::number(y() / dpiScale));
    writer.writeAttribute(QLatin1String("width"), QString::number(width() / dpiScale));
    writer.writeAttribute(QLatin1String("height"), QString::number(height() / dpiScale));

    writer.writeAttribute(QLatin1String("isinstance"), QString::number(m_isInstance));
    if (!m_instanceName.isEmpty())
        writer.writeAttribute(QLatin1String("instancename"), m_instanceName);
    writer.writeAttribute(QLatin1String("showstereotype"), QString::number(m_showStereotype));
}

bool UMLWidget::loadFromXMI(QDomElement & qElement)
{
    WidgetBase::loadFromXMI(qElement);
    DiagramProxyWidget::loadFromXMI(qElement);
    QString x = qElement.attribute(QLatin1String("x"), QLatin1String("0"));
    QString y = qElement.attribute(QLatin1String("y"), QLatin1String("0"));
    QString h = qElement.attribute(QLatin1String("height"), QLatin1String("0"));
    QString w = qElement.attribute(QLatin1String("width"), QLatin1String("0"));
    const qreal dpiScale = UMLApp::app()->document()->dpiScale();
    const qreal scaledW = toDoubleFromAnyLocale(w) * dpiScale;
    const qreal scaledH = toDoubleFromAnyLocale(h) * dpiScale;
    setSize(scaledW, scaledH);
    const qreal nX = toDoubleFromAnyLocale(x);
    const qreal nY = toDoubleFromAnyLocale(y);
    const qreal fixedX = nX + umlScene()->fixX();  // bug 449622
    const qreal fixedY = nY + umlScene()->fixY();
    const qreal scaledX = fixedX * dpiScale;
    const qreal scaledY = fixedY * dpiScale;
    umlScene()->updateCanvasSizeEstimate(scaledX, scaledY, scaledW, scaledH);
    setX(scaledX);
    setY(scaledY);

    QString isinstance = qElement.attribute(QLatin1String("isinstance"), QLatin1String("0"));
    m_isInstance = (bool)isinstance.toInt();
    m_instanceName = qElement.attribute(QLatin1String("instancename"));
    QString showstereo = qElement.attribute(QLatin1String("showstereotype"), QLatin1String("0"));
    m_showStereotype = (Uml::ShowStereoType::Enum)showstereo.toInt();

    return true;
}

/**
 * Adds a widget to the diagram, which is connected to the current widget
 * @param widget widget instance to add to diagram
 * @param type association type
 * @param options widget options
 */
void UMLWidget::addConnectedWidget(UMLWidget *widget, Uml::AssociationType::Enum type, AddWidgetOptions options)
{
    QString name = Widget_Utils::defaultWidgetName(widget->baseType());
    widget->setName(name);
    if (options & ShowProperties) {
        if (!widget->showPropertiesDialog()) {
            delete widget;
            return;
        }
    }

    umlScene()->addItem(widget);
    widget->setX(x() + rect().width() + 100);
    widget->setY(y());
    if (options & SetupSize) {
        widget->setSize(100, 40);
        QSizeF size = widget->minimumSize();
        widget->setSize(size);
    }
    AssociationWidget* assoc = options & SwitchDirection ? AssociationWidget::create(umlScene(), widget, type, this)
                                                    : AssociationWidget::create(umlScene(), this, type, widget);
    umlScene()->addAssociation(assoc);
    umlScene()->clearSelected();
    umlScene()->selectWidget(widget);

    UMLApp::app()->beginMacro(I18N_NEXT_RELEASE("Adding connected '%1'", widget->baseTypeStrWithoutPrefix());
    UMLApp::app()->executeCommand(new CmdCreateWidget(widget));
    UMLApp::app()->executeCommand(new CmdCreateWidget(assoc));
    UMLApp::app()->endMacro();
    m_doc->setModified();
}

/**
 * Adds a widget to the diagram, which is connected to the current widget
 * @param widget widget instance to add to diagram
 * @param showProperties whether to show properties of the widget
 */
void UMLWidget::addWidget(UMLWidget *widget, bool showProperties)
{
    umlScene()->addItem(widget);
    widget->setX(x() + rect().width() + 100);
    widget->setY(y());
    widget->setSize(100, 40);
    if (showProperties)
        widget->showPropertiesDialog();
    QSizeF size = widget->minimumSize();
    widget->setSize(size);
}
