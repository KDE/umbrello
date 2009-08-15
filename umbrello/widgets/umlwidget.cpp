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
#include "dialogs/classpropdlg.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "textitemgroup.h"
#include "textitem.h"
#include "umldoc.h"
#include "uml.h"
#include "umlscene.h"
#include "widgethandle.h"
#include "widget_utils.h"

#include <QtCore/QPointer>
#include <QtGui/QDialog>
#include <QtGui/QGraphicsSceneHoverEvent>

const QSizeF UMLWidget::DefaultMinimumSize(50, 20);
const QSizeF UMLWidget::DefaultMaximumSize(1000, 1000);

/**
 * Construct a UMLWidget associtated with object.
 *
 * @param object UMLObject with which widget should be associated.
 * @see WidgetBase::WidgetBase()
 */
UMLWidget::UMLWidget(UMLObject *object) :
    WidgetBase(object),
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

/// Destructor
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
void UMLWidget::setMinimumSize(const QSizeF& newSize, SizeHintOption option)
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
void UMLWidget::setMaximumSize(const QSizeF& newSize, SizeHintOption option)
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
void UMLWidget::setSize(const QSizeF &size)
{
    const QSizeF oldSize = m_size;

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

/// Sets whether this object is instance or not.
void UMLWidget::setIsInstance(bool b)
{
    m_isInstance = b;
    updateTextItemGroups();
}

void UMLWidget::setShowStereotype(bool b)
{
    m_showStereotype = b;
    // No need for attributeChange notification mechanism as of now.
    updateTextItemGroups();
}

void UMLWidget::showPropertiesDialog()
{
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        docwindow->showDocumentation(umlObject() , true);
        umlDoc()->setModified(true);
    }
    dlg->close();
    delete dlg;
}

void UMLWidget::setupContextMenuActions(ListPopupMenu &menu)
{
    Q_UNUSED(menu);
}

/**
 * @return The list of AssociationWidget's having one or both its ends associated
 *         with this ClassifierWidget.
 */
AssociationWidgetList UMLWidget::associationWidgetList() const
{
    return m_associationSpaceManager->associationWidgets().toList();
}

AssociationSpaceManager* UMLWidget::associationSpaceManager() const
{
    return m_associationSpaceManager;
}

/**
 * Adjusts the position and lines of connected association
 * widgets. This method is used usually after this widget moves
 * requiring assocition widgets to be updated.
 *
 * Subclasses can reimplement to fine tune this behvaior.
 */
void UMLWidget::adjustAssociations()
{
    foreach (AssociationWidget *assoc,
            m_associationSpaceManager->associationWidgets()) {
        assoc->associationLine()->calculateEndPoints();
    }
    // m_associationSpaceManager->adjust();
    //TODO: Implement this once AssociationWidget's are implemented.
}

bool UMLWidget::loadFromXMI(QDomElement &qElement)
{
    if(!WidgetBase::loadFromXMI(qElement))
        return false;

    qreal h  = qElement.attribute("height", "-1").toDouble();
    qreal w = qElement.attribute("width", "-1").toDouble();
    if(h < 0) {
        h = 20;
    }
    if(w < 0) {
        w = 20;
    }
    m_size.setWidth(w);
    m_size.setHeight(h);
    updateGeometry(); // Force updation of the shape as well.

    QString instName = qElement.attribute("instancename", QString());
    setInstanceName(instName);

    return true;
}

void UMLWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    WidgetBase::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
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
        adjustAssociations();
        return QVariant();
    }
    else if(change == FontHasChanged) {
        foreach(TextItemGroup *group, m_textItemGroups) {
            group->setFont(font());
        }
    }
    else if(change == FontColorHasChanged) {
        foreach(TextItemGroup *group, m_textItemGroups) {
            group->setFontColor(fontColor());
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
    if(!m_resizable) {
        delete m_widgetHandle;
        m_widgetHandle = 0;
    }
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

    WidgetBase::mouseMoveEvent(e);

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

    delete m_mouseMoveEventStore;
    m_mouseMoveEventStore = 0;
}

void UMLWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * Reimplemented to catch selection change notification and
 * enable/disable widget handles based on whether this widget is
 * selected or not.
 */
QVariant UMLWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged) {
        // create/delete widget handle on selection change
        bool selection = value.toBool();
        if(selection) {
            if(!m_widgetHandle) {
                m_widgetHandle = new WidgetHandle(this);
            }
        }
        else {
            delete m_widgetHandle;
            m_widgetHandle = 0;
        }
    } else if (change == QGraphicsItem::ItemPositionChange) {
        // move all points of self associations before this widget is moved.
        // normal adjusting is not enough for self association updation.
        QPointF diff(value.toPointF() - pos());
        foreach (AssociationWidget* assoc,
                m_associationSpaceManager->associationWidgets()) {
            if (assoc->isSelf()) {
                AssociationLine *line = assoc->associationLine();
                for (int i = 0; i < line->count(); ++i) {
                    line->setPoint(i, line->point(i) + diff);
                }
            }
        }
    } else if (change == QGraphicsItem::ItemPositionHasChanged) {
        // adjust the association lines by new computations.
        adjustAssociations();
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
    if(index < 0 || index >= m_textItemGroups.size()) {
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

#include "umlwidget.moc"
