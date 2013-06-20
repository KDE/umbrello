/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlwidget.h"

// local includes
#include "associationwidget.h"
#include "classifier.h"
#include "classpropdlg.h"
#include "cmds.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "idchangelog.h"
#include "listpopupmenu.h"
#include "settingsdlg.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidgetcontroller.h"
#include "uniqueid.h"

// kde includes
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QColor>
#include <QPainter>
#include <QPointer>

using namespace Uml;

DEBUG_REGISTER_DISABLED(UMLWidget)

const UMLSceneSize UMLWidget::DefaultMinimumSize(50, 20);
const UMLSceneSize UMLWidget::DefaultMaximumSize(1000, 5000);

/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param o The UMLObject to represent.
 * @param widgetController The UMLWidgetController of this UMLWidget
 */
UMLWidget::UMLWidget(UMLScene * scene, WidgetType type, UMLObject * o, UMLWidgetController *widgetController)
  : WidgetBase(scene, type)
{
    if (widgetController) {
        m_widgetController = widgetController;
    } else {
        m_widgetController = new UMLWidgetController(this);
    }
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
 * @param id The id of the widget.
 *  The default value (id_None) will prompt generation of a new ID.
 * @param widgetController The UMLWidgetController of this UMLWidget
 */
UMLWidget::UMLWidget(UMLScene *scene, WidgetType type, Uml::ID::Type id, UMLWidgetController *widgetController)
  : WidgetBase(scene, type)
{
    if (widgetController) {
        m_widgetController = widgetController;
    } else {
        m_widgetController = new UMLWidgetController(this);
    }
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
    delete m_widgetController;
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
    m_Font = other.m_Font;
    m_isInstance = other.m_isInstance;
    m_instanceName = other.m_instanceName;
    m_instanceName = other.m_instanceName;
    m_showStereotype = other.m_showStereotype;
    setX(other.x());
    setY(other.y());
    setRect(rect().x(), rect().y(), other.width(), other.height());

    // assign volatile (non-saved) members
    m_selected = other.m_selected;
    m_startMove = other.m_startMove;
    m_nPosX = other.m_nPosX;
    m_doc = other.m_doc;    //new
    m_resizable = other.m_resizable;
    for (unsigned i = 0; i < FT_INVALID; ++i)
        m_pFontMetrics[i] = other.m_pFontMetrics[i];
    m_activated = other.m_activated;
    m_ignoreSnapToGrid = other.m_ignoreSnapToGrid;
    m_ignoreSnapComponentSizeToGrid = other.m_ignoreSnapComponentSizeToGrid;
    // m_widgetController do not have any settings
    return *this;
}

/**
 * Overload '==' operator
 */
bool UMLWidget::operator==(const UMLWidget& other) const
{
    if (this == &other)
        return true;

    if (m_baseType != other.m_baseType) {
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
    if( m_Font != other.m_Font )
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
 * @return UMLSceneSize(mininum_width, minimum_height)
 */
UMLSceneSize UMLWidget::minimumSize()
{
    return m_minimumSize;
}

/**
 * This method is used to set the minimum size variable for this
 * widget.
 *
 * @param newSize The size being set as minimum.
 */
void UMLWidget::setMinimumSize(const UMLSceneSize& newSize)
{
    m_minimumSize = newSize;
}

/**
 * Compute the maximum possible width and height.
 *
 * @return maximum size
 */
UMLSceneSize UMLWidget::maximumSize()
{
    return m_maximumSize;
}

/**
 * This method is used to set the maximum size variable for this
 * widget.
 *
 * @param newSize The size being set as maximum.
 */
void UMLWidget::setMaximumSize(const UMLSceneSize& newSize)
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
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mouseMoveEvent
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_widgetController->mouseMoveEvent(event);
    slotSnapToGrid();
}

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mousePressEvent
 *
 * @param event The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    uDebug() << "widget = " << name() << " / type = " << baseTypeStr();
    if (event->button() == Qt::RightButton) {
        event->ignore();
        return;
    }
    m_widgetController->mousePressEvent(event);
}

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mouseReleaseEvent
 *
 * @param me The QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    m_widgetController->mouseReleaseEvent(me);
}

/**
 * Event handler for mouse double click events.
 * @param event the QGraphicsSceneMouseEvent event.
 */
void UMLWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        uDebug() << "widget = " << name() << " / type = " << baseTypeStr();
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
 * When a widget changes this slot captures that signal.
 */
void UMLWidget::updateWidget()
{
    updateGeometry();
    switch (m_baseType) {
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
void UMLWidget::constrain(UMLSceneValue& width, UMLSceneValue& height)
{
    UMLSceneSize minSize = minimumSize();
    if (width < minSize.width())
        width = minSize.width();
    if (height < minSize.height())
        height = minSize.height();
    UMLSceneSize maxSize = maximumSize();
    if (width > maxSize.width())
        width = maxSize.width();
    if (height > maxSize.height())
        height = maxSize.height();

    if (fixedAspectRatio()) {
        UMLSceneSize size = rect().size();
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
    m_isInstance = false;
    setMinimumSize(DefaultMinimumSize);
    setMaximumSize(DefaultMaximumSize);
    
    for (int i = 0; i < (int)FT_INVALID; ++i) {
        m_pFontMetrics[(UMLWidget::FontType)i] = 0;
    }
    
    if (m_scene) {
        m_useFillColor = true;
        m_usesDiagramFillColor = true;
        m_usesDiagramUseFillColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_fillColor = optionState.uiState.fillColor;
        // FIXME: using setFont() here let umbrello hang probably because of doc->loading() not set. 
        m_Font = optionState.uiState.font;
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

    m_selected = false;
    m_startMove = false;
    m_activated = false;
    m_ignoreSnapToGrid = false;
    m_ignoreSnapComponentSizeToGrid = false;
    m_doc = UMLApp::app()->document();
    m_nPosX = 0;
    connect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));

    connect(m_scene, SIGNAL(sigFillColorChanged(Uml::ID::Type)), this, SLOT(slotFillColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigLineColorChanged(Uml::ID::Type)), this, SLOT(slotLineColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigTextColorChanged(Uml::ID::Type)), this, SLOT(slotTextColorChanged(Uml::ID::Type)));
    connect(m_scene, SIGNAL(sigLineWidthChanged(Uml::ID::Type)), this, SLOT(slotLineWidthChanged(Uml::ID::Type)));

    m_umlObject = 0;
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
    case ListPopupMenu::mt_Delete:
        umlScene()->removeWidget(this);
        break;

    case ListPopupMenu::mt_Resize:
        resize();
        m_doc->setModified();
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
 * @param sceneID The id of the object behind the widget.
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
 * @param sceneID The id of the object behind the widget.
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
 * @param sceneID The id of the object behind the widget.
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
 * @param sceneID The id of the object behind the widget.
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
 * Set the status of using fill color.
 *
 * @param fc the status of using fill color.
 */
void UMLWidget::setUseFillColor(bool fc)
{
    WidgetBase::setUseFillColor(fc);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColorcmd(const QColor &color)
{
    WidgetBase::setTextColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColor(const QColor &color)
{
    UMLApp::app()->executeCommand(new CmdChangeTextColor(this, color));
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColorcmd(const QColor &color)
{
    WidgetBase::setLineColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColor(const QColor &color)
{
    UMLApp::app()->executeCommand(new CmdChangeLineColor(this, color));
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineWidth(uint width)
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
    UMLApp::app()->executeCommand(new CmdChangeFillColor(this, color));
}

/**
 * Sets the background fill color
 *
 * @param color the new fill color
 */
void UMLWidget::setFillColorcmd(const QColor &color)
{
    WidgetBase::setFillColor(color);
    update();
}

/**
 * Draws that the widget is selected.
 *
 * @param p Device on which is the selection is to be drawn.
 * @param offsetX The x-coordinate for drawing.
 * @param offsetY The y-coordinate for drawing.
 */
void UMLWidget::paintSelected(QPainter * p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();
    int s = 4;
    QBrush brush(Qt::blue);
    p->fillRect(offsetX, offsetY, s,  s, brush);
    p->fillRect(offsetX, offsetY + h - s, s, s, brush);
    p->fillRect(offsetX + w - s, offsetY, s, s, brush);

    // Draw the resize anchor in the lower right corner.
    // Don't draw it if the widget is so small that the
    // resize anchor would cover up most of the widget.
    if (m_resizable && w * h > (s*3) * (s*3)) {
        brush.setColor(Qt::red);
        const int right = offsetX + w;
        const int bottom = offsetY + h;
        p->drawLine(right - s, offsetY + h - 1, offsetX + w - 1, offsetY + h - s);
        p->drawLine(right - (s*2), bottom - 1, right - 1, bottom - (s*2));
        p->drawLine(right - (s*3), bottom - 1, right - 1, bottom - (s*3));
    } else {
        p->fillRect(offsetX + w - s, offsetY + h - s, s, s, brush);
    }
}

/**
 * Activate the object after serializing it from a QDataStream
 *
 * @param ChangeLog
 * @return  true for success
 */
bool UMLWidget::activate(IDChangeLog* /*ChangeLog  = 0 */)
{
    if (widgetHasUMLObject(m_baseType) && m_umlObject == NULL) {
        m_umlObject = m_doc->findObjectById(m_nId);
        if (m_umlObject == NULL) {
            uError() << "cannot find UMLObject with id=" << Uml::ID::toString(m_nId);
            return false;
        }
    }
    setFont(m_Font);
    setSize(width(), height());
    m_activated = true;
    updateGeometry();
    if (m_scene->getPaste()) {
        FloatingTextWidget * ft = 0;
        UMLScenePoint point = m_scene->getPastePoint();
        int x = point.x() + this->x();
        int y = point.y() + this->y();
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
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
bool UMLWidget::isActivated()
{
    return m_activated;
}

/**
 * Set the m_activated flag of a widget but does not perform the Activate method
 *
 * @param Active Status of activation is to be set.
 */
void UMLWidget::setActivated(bool Active /*=true*/)
{
    m_activated = Active;
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
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 */
void UMLWidget::adjustAssocs(int x, int y)
{
    // 2004-04-30: Achim Spangler
    // don't adjust Assocs on file load, as
    // the original positions, which are stored in XMI
    // should be reproduced exactly
    // ( don't try to reposition assocs as long
    //   as file is only partly loaded -> reposition
    //   could be misguided )
    /// @todo avoid trigger of this event during load
    if (m_doc->loading()) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        assocwidget->widgetMoved(this, x, y);
    }
}

/**
 * Adjusts all unselected associations with the given co-ordinates
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 */
void UMLWidget::adjustUnselectedAssocs(int x, int y)
{
    foreach(AssociationWidget* assocwidget , m_Assocs) {

        if (!assocwidget->isSelected())
            assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        if (!assocwidget->isSelected())
            assocwidget->widgetMoved(this, x, y);
    }
}

/**
 * Show a properties dialog for a UMLWidget.
 */
void UMLWidget::showPropertiesDialog()
{
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    umlScene()->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        umlScene()->showDocumentation(umlObject() , true);
        m_doc->setModified(true);
    }
    dlg->close(); //wipe from memory
    delete dlg;
}

/**
 * Returns 0 if the given point is not in the boundaries of the widget,
 * else returns a number which is proportional to the size of the widget.
 *
 * @param p Point to be checked.
 *
 * @return 0 if the given point is not in the boundaries of the widget;
 *         (width()+height())/2 if the point is within the boundaries.
 */
UMLSceneValue UMLWidget::onWidget(const UMLScenePoint & p)
{
    const UMLSceneValue w = width();
    const UMLSceneValue h = height();
    const UMLSceneValue left = x();
    const UMLSceneValue right = left + w;
    const UMLSceneValue top = y();
    const UMLSceneValue bottom = top + h;
    if (p.x() < left || p.x() > right ||
            p.y() < top || p.y() > bottom)   // Qt coord.sys. origin in top left corner
        return 0;
    return (w + h) / 2;
}

/**
 * Move the widget by an X and Y offset relative to
 * the current position.
 */
void UMLWidget::moveByLocal(int dx, int dy)
{
    int newX = x() + dx;
    int newY = y() + dy;
    setX(newX);
    setY(newY);
    // DEBUG(DBG_SRC) << "********** x=" << newX << " / y=" << newY;
    adjustAssocs(newX, newY);
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
 * calculate content related size of widget.
 *
 * @return calculated widget size
 */
UMLSceneSize UMLWidget::calculateSize()
{
    return UMLSceneSize(width(), height());
}

void UMLWidget::resize()
{
    // @TODO minimumSize() do not work in all cases, we need a dedicated autoResize() method
    UMLSceneSize size = minimumSize();
    setSize(size.width(), size.height());
    adjustAssocs(x(), y());    // adjust assoc lines
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select The state of whether the widget is selected.
 */
void UMLWidget::setSelected(bool _select)
{
    const WidgetBase::WidgetType wt = m_baseType;
    if (_select) {
        if (m_scene->selectedCount() == 0) {
            if (widgetHasUMLObject(wt)) {
                m_scene->showDocumentation(m_umlObject, false);
            } else {
                m_scene->showDocumentation(this, false);
            }
        }//end if
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(9);//keep text on top and boxes behind so don't touch Z value
        } */
    } else {
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(m_origZ);
        } */
        if (m_selected)
            m_scene->updateDocumentation(true);
    }
    m_selected = _select;

    const QPoint pos(x(), y());
    UMLWidget *bkgnd = m_scene->widgetAt(pos);
    if (bkgnd && bkgnd != this && _select) {
        DEBUG(DBG_SRC) << "setting Z to " << bkgnd->zValue() + 1.0 << ", SelectState: " << _select;
        setZValue(bkgnd->zValue() + 1.0);
//:TODO:    } else {
//:TODO:        setZ(m_origZ);
    }

    update();

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

/**
 *   Captures a sigClearAllSelected signal sent by @ref UMLView
 */
void UMLWidget::slotClearAllSelected()
{
    setSelected(false);
}

/**
 * Sets the view the widget is on.
 *
 * @param v The view the widget is on.
 */
void UMLWidget::setScene(UMLScene * v)
{
    //remove signals from old view - was probably 0 anyway
    disconnect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));
    disconnect(m_scene, SIGNAL(sigFillColorChanged(Uml::ID::Type)), this, SLOT(slotFillColorChanged(Uml::ID::Type)));
    disconnect(m_scene, SIGNAL(sigTextColorChanged(Uml::ID::Type)), this, SLOT(slotTextColorChanged(Uml::ID::Type)));
    disconnect(m_scene, SIGNAL(sigLineWidthChanged(Uml::ID::Type)), this, SLOT(slotLineWidthChanged(Uml::ID::Type)));
    m_scene = v;
    connect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));
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
void UMLWidget::setX(UMLSceneValue x)
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
void UMLWidget::setY(UMLSceneValue y)
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
        qreal newY = m_scene->snappedX(y());
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
void UMLWidget::setSize(UMLSceneValue width, UMLSceneValue height)
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

    setRect(rect().x(), rect().y(), width, height);
}

/**
 * Update the size of this widget.
 */
void UMLWidget::updateGeometry()
{
    if (m_doc->loading())
        return;
    UMLSceneSize size = calculateSize();
    UMLSceneValue clipWidth = size.width();
    UMLSceneValue clipHeight = size.height();
    constrain(clipWidth, clipHeight);
    setSize(clipWidth, clipHeight);
    slotSnapToGrid();
    adjustAssocs(x(), y());    // adjust assoc lines
}

/**
 * Update the look of this widget.
 */
void UMLWidget::updateLook()
{
    if (m_doc->loading()) {
        return;
    }
    if (umlScene()) {
        m_useFillColor = true;
        m_usesDiagramFillColor = true;
        m_usesDiagramUseFillColor = true;
        const Settings::OptionState& optionState = umlScene()->optionState();
        m_fillColor = optionState.uiState.fillColor;
        // FIXME: using setFont() here let umbrello hang probably because of doc->loading() not set.
        m_Font = optionState.uiState.font;
        m_showStereotype = optionState.classState.showStereoType;
    } else {
        uError() << "SERIOUS PROBLEM - scene is NULL";
        m_useFillColor = false;
        m_usesDiagramFillColor = false;
        m_usesDiagramUseFillColor = false;
        m_showStereotype = false;
    }
}

/**
 * clip the size of this widget against the
 * minimal and maximal limits.
 */
void UMLWidget::clipSize()
{
    UMLSceneValue clipWidth = width();
    UMLSceneValue clipHeight = height();
    constrain(clipWidth, clipHeight);
    setSize(clipWidth, clipHeight);
}

/**
 * Template Method, override this to set the default font metric.
 */
void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType)
{
    setupFontType(m_Font, fontType);
    setFontMetrics(fontType, QFontMetrics(m_Font));
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

/**
 * Template Method, override this to set the default font metric.
 */
void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter)
{
    setupFontType(m_Font, fontType);
    painter.setFont(m_Font);
    setFontMetrics(fontType, painter.fontMetrics());
}

/**
 * Returns the font metric used by this object for Text
 * which uses bold/italic fonts.
 */
QFontMetrics &UMLWidget::getFontMetrics(UMLWidget::FontType fontType)
{
    if (m_pFontMetrics[fontType] == 0) {
        setDefaultFontMetrics(fontType);
    }
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
 *  Returns the font the widget is to use.
 */
QFont UMLWidget::font() const
{
    return m_Font;
}

/**
 * Sets the font the widget is to use.
 *
 * @param font Font to be set.
 */
void UMLWidget::setFont(QFont font)
{
    m_Font = font;
    forceUpdateFontMetrics(0);
    if (m_doc->loading())
        return;
    update();
}

/**
 * @note For performance Reasons, only FontMetrics for already used
 *  font types are updated. Not yet used font types will not get a font metric
 *  and will get the same font metric as if painter was zero.
 *  This behaviour is acceptable, because diagrams will always be showed on Display
 *  first before a special painter like a printer device is used.
 */
void UMLWidget::forceUpdateFontMetrics(QPainter *painter)
{
    if (painter == 0) {
        for (int i = 0; i < (int)UMLWidget::FT_INVALID; ++i) {
            if (m_pFontMetrics[(UMLWidget::FontType)i] != 0)
                setDefaultFontMetrics((UMLWidget::FontType)i);
        }
    } else {
        for (int i2 = 0; i2 < (int)UMLWidget::FT_INVALID; ++i2) {
            if (m_pFontMetrics[(UMLWidget::FontType)i2] != 0)
                setDefaultFontMetrics((UMLWidget::FontType)i2, *painter);
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
void UMLWidget::moveEvent(QGraphicsSceneMouseEvent* /*me*/)
{
}

void UMLWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    /*
      Call after required actions in child class.
      Type must be set in the child class.
    */
    WidgetBase::saveToXMI(qDoc, qElement);
    qElement.setAttribute("xmi.id", Uml::ID::toString(id()));
    qElement.setAttribute("font", m_Font.toString());
    qElement.setAttribute("x", x());
    qElement.setAttribute("y", y());
    qElement.setAttribute("width", width());
    qElement.setAttribute("height", height());
    qElement.setAttribute("isinstance", m_isInstance);
    if (!m_instanceName.isEmpty())
        qElement.setAttribute("instancename", m_instanceName);
    if (m_showStereotype)
        qElement.setAttribute("showstereotype", m_showStereotype);
}

bool UMLWidget::loadFromXMI(QDomElement & qElement)
{
    WidgetBase::loadFromXMI(qElement);
    QString id = qElement.attribute("xmi.id", "-1");
    QString font = qElement.attribute("font", "");
    QString x = qElement.attribute("x", "0");
    QString y = qElement.attribute("y", "0");
    QString h = qElement.attribute("height", "0");
    QString w = qElement.attribute("width", "0");

    m_nId = Uml::ID::fromString(id);

    if (!font.isEmpty()) {
        QFont newFont;
        newFont.fromString(font);
        setFont(newFont);
    } else {
        uWarning() << "Using default font " << m_Font.toString()
                   << " for widget with xmi.id " << Uml::ID::toString(m_nId);
    }

    setSize(w.toInt(), h.toInt());
    setX(x.toInt());
    setY(y.toInt());
    QString isinstance = qElement.attribute("isinstance", "0");
    m_isInstance = (bool)isinstance.toInt();
    m_instanceName = qElement.attribute("instancename", "");
    QString showstereo = qElement.attribute("showstereotype", "0");
    m_showStereotype = (bool)showstereo.toInt();
    return true;
}

/**
 * Returns the UMLWdigetController for this widget.
 */
UMLWidgetController* UMLWidget::getWidgetController()
{
    return m_widgetController;
}

#include "umlwidget.moc"
