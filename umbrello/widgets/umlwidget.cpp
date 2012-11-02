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

#include "umlwidget.h"

#include "associationline.h"
#include "associationspacemanager.h"
#include "associationwidget.h"
#include "classpropdlg.h"
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "textitemgroup.h"
#include "textitem.h"
#include "umldoc.h"
#include "uml.h"
#include "umlscene.h"
#include "widgethandle.h"
#include "widget_utils.h"

#include <QDialog>
#include <QPointer>

const UMLSceneSize UMLWidget::DefaultMinimumSize(50, 20);
const UMLSceneSize UMLWidget::DefaultMaximumSize(1000, 1000);

/**
 * Construct a UMLWidget using an id.
 *
 * @param type The type of the object.
 * @param id   The object id.
 * @see WidgetBase::WidgetBase()
 */
UMLWidget::UMLWidget(WidgetType type, Uml::IDType id)
  : WidgetBase(type),
    m_size(20, 20),
    m_minimumSize(UMLWidget::DefaultMinimumSize),
    m_maximumSize(UMLWidget::DefaultMaximumSize),
    m_margin(5.0), // Default margin size
    m_isInstance(false),
    m_resizable(true),
    m_widgetHandle(0),
    m_mouseMoveEventStore(0)
{
    if (id != Uml::id_None) {
        setID(id);
    }
    
    m_associationSpaceManager = new AssociationSpaceManager(this);
}

/**
 * Construct a UMLWidget associtated with object.
 *
 * @param type The type of the object.
 * @param object UMLObject with which widget should be associated.
 * @see WidgetBase::WidgetBase()
 */
UMLWidget::UMLWidget(WidgetType type, UMLObject *object)
  : WidgetBase(type, object),
    m_size(20, 20),
    m_minimumSize(UMLWidget::DefaultMinimumSize),
    m_maximumSize(UMLWidget::DefaultMaximumSize),
    m_margin(5.0), // Default margin size
    m_isInstance(false),
    m_resizable(true),
    m_widgetHandle(0),
    m_mouseMoveEventStore(0)
{
    m_associationSpaceManager = new AssociationSpaceManager(this);
}

/**
 * Destructor.
 */
UMLWidget::~UMLWidget()
{
    qDeleteAll(m_textItemGroups);
    delete m_associationSpaceManager;
}

/**
 * This method is used to set the minimum size variable for this
 * widget.
 *
 * @param newSize The size being set as minimum.
 *
 * @param option Adds (2 * margin()) to the size if
 *               "option = AddMargin". Otherwise adds nothing.
 *
 * This method only sets the variable m_minimumSize and doesn't update
 * the geometry. The geometry should be explicitly updated using
 * WidgetBase::updateGeometry()
 */
void UMLWidget::setMinimumSize(const UMLSceneSize& newSize, SizeHintOption option)
{
    m_minimumSize = newSize;
    if (option == AddMargin) {
        qreal m = 2 * margin();
        m_minimumSize.rwidth() += m;
        m_minimumSize.rheight() += m;
    }
}

/**
 * This method is used to set the maximum size variable for this
 * widget.
 *
 * @param newSize The size being set as maximum.
 *
 * @param option Adds (2 * margin()) to the size if
 *               "option = AddMargin". Otherwise adds nothing.
 *
 * This method only sets the variable m_maximumSize and doesn't update
 * the geometry. The geometry should be explicitly updated using
 * WidgetBase::updateGeometry()
 */
void UMLWidget::setMaximumSize(const UMLSceneSize& newSize, SizeHintOption option)
{
    m_maximumSize = newSize;
    if (option == AddMargin) {
        qreal m = 2 * margin();
        m_maximumSize.rwidth() += m;
        m_maximumSize.rheight() += m;
    }
}

/**
 * Sets the size of the widget to \a size.
 *
 * Also notifies self and subclasses with @ref SizeHasChanged
 * notification after setting the new size.
 * @see UMLWidget::attributeChange
 *
 * @param size The new size (minimumSize < size < maximumSize)
 */
void UMLWidget::setSize(const UMLSceneSize &size)
{
    const UMLSceneSize oldSize = m_size;

    // Satisfy (minimumSize < size < maximumSize) requirement.
    m_size = minimumSize().expandedTo(size);
    m_size = maximumSize().boundedTo(m_size);

    QRectF boundRect = rect();
    // Adjust bounding rect with half the pen width(lineWidth).
    const qreal hpw = 0.5 * lineWidth();
    boundRect.adjust(-hpw, -hpw, hpw, hpw);

    // Now set the bounding rect, (slightly larger or equal to rect())
    setBoundingRect(boundRect);

    // Notify self and subclasses about change of size.
    attributeChange(SizeHasChanged, oldSize);
}

/**
 * @return The rect area of widget in scene coordinates.
 */
QRectF UMLWidget::sceneRect() const
{
    return mapToScene(rect()).boundingRect();
}

/**
 * Sets the margin of this widget to \a margin. This method only
 * updates the variable. To see the effective margin, updateGeometry()
 * should be called.
 */
void UMLWidget::setMargin(qreal margin)
{
    m_margin = margin;
}

/**
 * Set the instance name for this widget. Calls updateGeometry
 * implicitly.
 */
void UMLWidget::setInstanceName(const QString &name)
{
    m_instanceName = name;
    // No need for attributeChange notification mechanism as of now.
    updateTextItemGroups();
}

/**
 * Sets whether this object is instance or not.
 */
void UMLWidget::setIsInstance(bool b)
{
    m_isInstance = b;
    updateTextItemGroups();
}

/**
 * Setter for flag whether stereotypes are shown.
 */
void UMLWidget::setShowStereotype(bool b)
{
    m_showStereotype = b;
    // No need for attributeChange notification mechanism as of now.
    updateTextItemGroups();
}

/**
 * Dialog of the properties is shown.
 */
void UMLWidget::showPropertiesDialog()
{
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    umlScene()->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);
    if (dlg->exec()) {
        umlScene()->showDocumentation(umlObject() , true);
        umlDoc()->setModified(true);
    }
    dlg->close();
    delete dlg;
}

/**
 * @return The list of AssociationWidget's having one or both its ends associated
 *         with this ClassifierWidget.
 */
AssociationWidgetList UMLWidget::associationWidgetList() const
{
    return m_associationSpaceManager->associationWidgets().toList();
}

/**
 *
 */
AssociationSpaceManager* UMLWidget::associationSpaceManager() const
{
    return m_associationSpaceManager;
}

/**
 *
 */
bool UMLWidget::activate()
{
    setActivatedFlag(false);
    if (!m_loadData.isEmpty()) {
        if (widgetHasUMLObject(baseType()) && !umlObject()) {
            Uml::IDType id = STR2ID(m_loadData.value("id", "-1").toString());
            UMLObject *obj = umlDoc()->findObjectById(id);
            if (!obj) {
                uError() << "cannot find UMLObject with id=" << ID2STR(id);
                return isActivated();
            }
            setUMLObject(obj);
        }
        setSize(m_loadData.value("size").toSizeF());
        setInstanceName(m_loadData.value("instanceName").toString());
        setShowStereotype(m_loadData.value("showStereotype").toBool());
    }
    return WidgetBase::activate();
}

/**
 * Adjusts the position and lines of connected association
 * widgets. This method is used usually after this widget moves
 * requiring assocition widgets to be updated.
 *
 * Subclasses can reimplement to fine tune this behvaior.
 */
void UMLWidget::adjustAssociations(bool userChangeAdjust)
{
    foreach (AssociationWidget *assoc,
            m_associationSpaceManager->associationWidgets()) {
        if (userChangeAdjust) {
            assoc->setUserChange(AssocAdjustChange, true);
        }
        assoc->associationLine()->calculateEndPoints();
        if (userChangeAdjust) {
            assoc->setUserChange(AssocAdjustChange, false);
        }
    }
    // m_associationSpaceManager->adjust();
    //TODO: Implement this once AssociationWidget's are implemented.
}

/**
 * XMI structure is read and attributes are loaded.
 */
bool UMLWidget::loadFromXMI(QDomElement &qElement)
{
    if (!WidgetBase::loadFromXMI(qElement)) {
        return false;
    }

    m_loadData.insert("id", qElement.attribute("xmi.id", "-1"));

    UMLSceneSize size;
    size.setWidth(qElement.attribute("width", "0").toDouble());
    size.setHeight(qElement.attribute("height", "0").toDouble());
    m_loadData.insert("size", size);
    m_isInstance = (bool)qElement.attribute("isinstance", "0").toInt();
    m_loadData.insert("instanceName", qElement.attribute("instancename"));
    m_loadData.insert("showStereotype",
            (bool)qElement.attribute("showstereotype", "0").toInt());

    return true;
}

/**
 * Properties of the UMLWidget are written into XMI structure.
 */
void UMLWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    WidgetBase::saveToXMI(qDoc, qElement);

    qElement.setAttribute("xmi.id", ID2STR(id()));
    const UMLSceneSize sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());
    qElement.setAttribute("isinstance", m_isInstance);

    if (!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }

    if (m_showStereotype) {
        qElement.setAttribute("showstereotype", m_showStereotype);
    }
}

/**
 * Reimplemented from WidgetBase::attributeChange to handle change
 * of size, change of font and change of font color.
 */
QVariant UMLWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if(change == SizeHasChanged) {
        // We should update the shape of this widget.
        QPainterPath newShape;
        newShape.addRect(rect());
        setShape(newShape);

        // Also update widget Handles position.
        if(m_widgetHandle) {
            m_widgetHandle->updateHandlePosition();
        }
        adjustAssociations(true);
        return QVariant();
    }
    else if(change == FontHasChanged) {
        foreach(TextItemGroup *group, m_textItemGroups) {
            group->setFont(font());
        }
    }
    else if(change == TextColorHasChanged) {
        foreach(TextItemGroup *group, m_textItemGroups) {
            group->setTextColor(textColor());
        }
    }
    else if(change == NameHasChanged) {
        updateTextItemGroups();
        // No need for base method here as updateTextItemGroups
        // updates the geometry too.
        return QVariant();
    }

    // Pass on remaining handling responsibility to base method.
    return WidgetBase::attributeChange(change, oldValue);
}

/**
 * Reimplemented to ensure widget current size fits the sizeHint
 * constraints.
 */
void UMLWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the size hint constraints.

    // Subclasses might set the minimum and maximum sizes in the
    // overriden version.
    setSize(m_size);
}

/**
 * Set whether this widget is resizable or not.
 */
void UMLWidget::setResizable(bool resizable)
{
    m_resizable = resizable;
    if (!m_resizable) {
        delete m_widgetHandle;
        m_widgetHandle = 0;
    }
}

/**
 * Event handler for context menu events.
 * Note: It is public because it is called from UMLScene::contextMenuEvent(event).
 *       This should not be.
 */
void UMLWidget::contextMenuEvent(UMLSceneContextMenuEvent * event)
{
    uDebug() << "widget = " << name() << " / type = " << baseTypeStr();
    WidgetBase::contextMenuEvent(event);
}

/**
 * Reimplemented to support multi selection using either Shift or
 * Control modifier.
 *
 * This is implemented by reusing QGraphicsItem::mousePressEvent, but by
 * customizing the data stored in the event.
 */
void UMLWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // inform scene that item is not yet being moved using mouse
    umlScene()->setIsMouseMovingItems(false);
    const Qt::KeyboardModifiers save = event->modifiers();
    const Qt::KeyboardModifiers forMulti = (Qt::ControlModifier | Qt::ShiftModifier);
    if ((save & forMulti) != 0) {
        event->setModifiers(Qt::KeyboardModifiers(save | forMulti));
    }
    // Now do the call with modified event.
    WidgetBase::mousePressEvent(event);
    // Reset the event data.
    event->setModifiers(save);
}

/**
 * Reimplemented to call UMLScene::setItemMovingUsingMouse to true and also handle
 * modifier based X constrained or Y constrained movement.
 */
void UMLWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    // Now inform scene that item is being moved using mouse.
    umlScene()->setIsMouseMovingItems(true);

    bool beganMoveNow = (!m_mouseMoveEventStore);
    if (!m_mouseMoveEventStore)  {
        m_mouseMoveEventStore = new QGraphicsSceneMouseEvent();

        m_mouseMoveEventStore->setPos(e->pos());
        m_mouseMoveEventStore->setScenePos(e->scenePos());
        m_mouseMoveEventStore->setScreenPos(e->screenPos());

        m_mouseMoveEventStore->setLastPos(e->lastPos());
        m_mouseMoveEventStore->setLastScenePos(e->lastScenePos());
        m_mouseMoveEventStore->setLastScreenPos(e->lastScreenPos());
    }
    const Qt::KeyboardModifiers save = e->modifiers();
    const Qt::KeyboardModifiers forMulti = (Qt::ControlModifier | Qt::ShiftModifier);

    if ((save & forMulti) != 0) {
        bool constrainY = ((save & forMulti) != forMulti);

        QPointF p = e->pos();
        if (constrainY) {
            p.setY(m_mouseMoveEventStore->lastPos().y());
        } else {
            p.setX(m_mouseMoveEventStore->lastPos().x());
        }
        m_mouseMoveEventStore->setPos(p);

        p = e->scenePos();
        if (constrainY) {
            p.setY(m_mouseMoveEventStore->lastScenePos().y());
        } else {
            p.setX(m_mouseMoveEventStore->lastScenePos().x());
        }
        m_mouseMoveEventStore->setScenePos(p);

        QPoint pt = e->screenPos();
        if (constrainY) {
            pt.setY(m_mouseMoveEventStore->lastScreenPos().y());
        } else {
            pt.setX(m_mouseMoveEventStore->lastScreenPos().x());
        }
        m_mouseMoveEventStore->setScreenPos(pt);

    } else {
        m_mouseMoveEventStore->setPos(e->pos());
        m_mouseMoveEventStore->setScenePos(e->scenePos());
        m_mouseMoveEventStore->setScreenPos(e->screenPos());
    }

    e->setPos(m_mouseMoveEventStore->pos());
    e->setScenePos(m_mouseMoveEventStore->scenePos());
    e->setScreenPos(m_mouseMoveEventStore->screenPos());

    e->setLastPos(m_mouseMoveEventStore->lastPos());
    e->setLastScenePos(m_mouseMoveEventStore->lastScenePos());
    e->setLastScreenPos(m_mouseMoveEventStore->lastScreenPos());

    QList<QGraphicsItem*> selection = scene()->selectedItems();
    if (beganMoveNow) {
        foreach (QGraphicsItem *item, selection) {
            if (item != this) {
                if (item->parentItem() && item->parentItem()->isSelected()) {
                    item->setSelected(false);
                }
            } else {
                if (item->parentItem() && item->parentItem()->isSelected()) {
                    item->parentItem()->setSelected(false);
                }
            }
        }
    }

    foreach (QGraphicsItem *item, selection) {
        WidgetBase *wid = qobject_cast<WidgetBase*>(item->toGraphicsObject());
        if (wid) {
            wid->setUserChange(PositionChange, true);
        }
    }

    WidgetBase::mouseMoveEvent(e);

    foreach (QGraphicsItem *item, selection) {
        WidgetBase *wid = qobject_cast<WidgetBase*>(item->toGraphicsObject());
        if (wid) {
            wid->setUserChange(PositionChange, false);
        }
    }

    // reset text positions of associations while moving
    foreach (AssociationWidget *assoc,
            m_associationSpaceManager->associationWidgets()) {
        assoc->resetTextPositions();
    }

    m_mouseMoveEventStore->setLastPos(m_mouseMoveEventStore->pos());
    m_mouseMoveEventStore->setLastScenePos(m_mouseMoveEventStore->scenePos());
    m_mouseMoveEventStore->setLastScreenPos(m_mouseMoveEventStore->screenPos());
}

/**
 * Reimplemented to call UMLScene::setItemMovingUsingMouse to false.
 */
void UMLWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // inform scene that item is not yet being moved using mouse
    umlScene()->setIsMouseMovingItems(false);
    WidgetBase::mouseReleaseEvent(event);

    if (m_mouseMoveEventStore) {
        //TODO: Push cmds to stack.
        delete m_mouseMoveEventStore;
        m_mouseMoveEventStore = 0;
    }
}

/**
 * Event handler for mouse double clicks.
 * The properties dialog is shown.
 */
void UMLWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        showPropertiesDialog();
        event->accept();
    }
}

/**
 * Reimplemented to catch selection change notification and
 * enable/disable widget handles based on whether this widget is
 * selected or not.
 */
QVariant UMLWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        // create/delete widget handle on selection change
        bool selection = value.toBool();
        if (selection) {
            if (!m_widgetHandle) {
                m_widgetHandle = new WidgetHandle(this);
            }
        }
        else {
            delete m_widgetHandle;
            m_widgetHandle = 0;
        }
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        // adjust the association lines by new computations.
        adjustAssociations(true);
        // Move the self association widgets separately.
        QPointF diff(pos() - m_itemPositionChangePos);
        foreach (AssociationWidget* assoc,
                m_associationSpaceManager->associationWidgets()) {
            if (assoc->isSelf()) {
                assoc->setUserChange(AssocAdjustChange, true);
                //TODO: Push undo command
                AssociationLine *line = assoc->associationLine();
                for (int i = 0; i < line->count(); ++i) {
                    line->setPoint(i, line->point(i) + diff);
                }
                assoc->setUserChange(AssocAdjustChange, false);
            }
        }
    } else if (change == QGraphicsItem::ItemVisibleHasChanged) {
        foreach (TextItemGroup *grp, m_textItemGroups) {
            grp->updateVisibility();
        }
    }
    return WidgetBase::itemChange(change, value);
}

/**
 * Creates a new TextItemGroup, appends it to internal list -
 * m_textItemGroups and returns a pointer to the same.
 */
TextItemGroup* UMLWidget::createTextItemGroup()
{
    TextItemGroup *newGrp = new TextItemGroup(this);
    newGrp->setMargin(margin());
    m_textItemGroups.append(newGrp);
    return newGrp;
}

/**
 * @return Index of the group in the internal list - m_textItemGroups
 */
int UMLWidget::indexOfTextItemGroup(TextItemGroup *group) const
{
    return m_textItemGroups.indexOf(group);
}

/**
 * @return TextItemGroup present at index in internal list -
 *         m_textItemGroups
 */
TextItemGroup* UMLWidget::textItemGroupAt(int index) const
{
    if (index < 0 || index >= m_textItemGroups.size()) {
        qFatal("UMLWidget::textItemGroupAt: Invalid index %d", index);
    }
    return m_textItemGroups[index];
}

/**
 * Subclasses should reimlement this method to update the TextItem's
 * texts and properties for all the TextItemGroup's of this widget i.e
 * \a m_textItemGroups.
 */
void UMLWidget::updateTextItemGroups()
{
    // Update the geometry as we don't know about texts.
    updateGeometry();
}

/**
 * Reimplmented from WidgetBase::slotUMLObjectDataChanged to update
 * the texts of the TextItemGroups
 */
void UMLWidget::slotUMLObjectDataChanged()
{
    updateTextItemGroups();
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

    ListPopupMenu *menu = ListPopupMenu::menuFromAction(trigger);
    if (!menu) {
        uError() << "Action's data field does not contain ListPopupMenu pointer";
        return;
    }

    ListPopupMenu::MenuType sel = menu->getMenuType(trigger);
    switch (sel) {
    case ListPopupMenu::mt_Delete:
        umlScene()->removeWidget(this);
        break;

    default:
        WidgetBase::slotMenuSelection(trigger);
        break;
    }
}

#include "umlwidget.moc"
