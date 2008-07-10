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

#include "newumlrectwidget.h"
#include "listpopupmenu.h"
#include "widget_utils.h"
#include "widgethandle.h"
#include "umlscene.h"

#include <QtGui/QDialog>
#include <QtGui/QGraphicsSceneHoverEvent>

// Inline and other documentation BEGIN

/**
 * @class NewUMLRectWidget
 *
 * @short The base class for rectangular base UML widgets.
 *
 * This class extends the functionality provided by NewUMLWidget with
 * facilities to resize the widget, provide flexible constraints
 * management.
 *
 * Also this class handles connected association widgets whenever
 * required.
 */

/**
 * @fn NewUMLRectWidget::size
 *
 * @return The current size of the widget.
 */

/**
 * @fn NewUMLRectWidget::rect
 *
 * Shortcut for QRectF(QPointF(0, 0), size())
 *
 * @note rect() is not same as boundingRect(). The latter also
 *       includes extra portions to cover resize handles.
 */

/**
 * @fn NewUMLRectWidget::instanceName
 *
 * @return The instance name of this widget(if used).
 */

/**
 * @fn NewUMLRectWidget::associationWidgetList
 *
 * @return A list of AssociationWidget connected with this widet.
 */

/**
 * @fn NewUMLRectWidget::isResizable
 *
 * @return Return whether this widget is resizable or not.
 * @see setResizable
 */

// Inline and other documentation ENDS

const QSizeF NewUMLRectWidget::DefaultMinimumSize(50, 20);
const QSizeF NewUMLRectWidget::DefaultMaximumSize(1000, 1000);
const QSizeF NewUMLRectWidget::DefaultPreferredSize(70, 20);

/**
 * Construct a NewUMLRectWidget associtated with object.
 *
 * @param object UMLObject with which widget should be associated.
 * @see NewUMLWidget::NewUMLWidget()
 */
NewUMLRectWidget::NewUMLRectWidget(UMLObject *object) :
    NewUMLWidget(object),
    m_size(20, 20),
    m_resizable(true),
    m_widgetHandle(0)
{
}

NewUMLRectWidget::~NewUMLRectWidget()
{
}

/**
 * This virtual method should be implemented by the subclasses to
 * provide Maximum and Minimum size information for this widget. This
 * information is used in setSize and also by the UMLScene.
 *
 * @param which The Qt::SizeHint enum value indicates what information
 *              is being queried. For now only Qt::MinimumSize and
 *              Qt::MaximumSize is used.
 *
 * @return The minimum or maximum size based on \a which.
 *
 * @note When sizeHint of a subclass needs a change, updateGeometry
 *       method should be called to ensure the widget is of proper
 *       size.
 *
 * @note Non const to allow subclasses to update their cached sizehint
 *       variable if desired.
 */
QSizeF NewUMLRectWidget::sizeHint(Qt::SizeHint which)
{
    switch(which) {
    case Qt::MinimumSize:
        return DefaultMinimumSize;

    case Qt::MaximumSize:
        return DefaultMaximumSize;

    case Qt::PreferredSize:
    default:
        return DefaultPreferredSize;
    }
}

/**
 * Sets the size of the widget to \a size.
 *
 * It also does cosmetic adjustments(enlarges) to the actual
 * boundingRect to include resize handles.
 *
 * Also notifies self with @ref sizeChanged() virtual method, which
 * can be used to update the shape corresponding to the new size of
 * the widget.
 *
 * @param size The new size to be set which should effectively satisfy
 *             the sizeHint constraints.
 */
void NewUMLRectWidget::setSize(const QSizeF &size)
{
    const QSizeF oldSize = m_size;
    const QSizeF minSize = sizeHint(Qt::MinimumSize);
    const QSizeF maxSize = sizeHint(Qt::MaximumSize);

    m_size = minSize.expandedTo(size);
    m_size = maxSize.boundedTo(m_size);

    QRectF boundRect = rect();
    // Adjust bounding rect with half the pen width(lineWidth).
    const qreal hpw = 0.5 * lineWidth();
    boundRect.adjust(-hpw, -hpw, hpw, hpw);

    setBoundingRect(boundRect);

    // Notify by calling sizeHasChanged method with old size
    sizeHasChanged(oldSize);
}

/**
 * Set the instance name for this widget. Calls updateGeometry
 * implicitly.
 */
void NewUMLRectWidget::setInstanceName(const QString &name)
{
    m_instanceName = name;
    updateGeometry();
}

void NewUMLRectWidget::setShowStereotype(bool b)
{
    m_showStereotype = b;
    updateGeometry();
}

/**
 * Adds a association widget to the internal list that involves this
 * widget.
 *
 * @param assoc An association widget connected to this widget.
 */
void NewUMLRectWidget::addAssociationWidget(AssociationWidget *assoc)
{
    if(!m_associationWidgetList.contains(assoc)) {
        m_associationWidgetList << assoc;
    }
}

/**
 * Removes a association widget from the internal list that involves this widget.
 *
 * @param assoc The association widget that should be removed.
 */
void NewUMLRectWidget::removeAssociationWidget(AssociationWidget *assoc)
{
    m_associationWidgetList.removeAll(assoc);
}

void NewUMLRectWidget::showPropertiesDialog()
{
    QDialog stubDialog;
    stubDialog.setWindowTitle("Stub.. To be worked out still.");
    stubDialog.exec();
}

void NewUMLRectWidget::setupContextMenuActions(ListPopupMenu &menu)
{
    menu.addAction("Stub!!");
}

/**
 * Adjusts the position and lines of connected association
 * widgets. This method is used usually after this widget moves
 * requiring assocition widgets to be updated.
 */
void NewUMLRectWidget::adjustConnectedAssociations()
{
    //TODO: Implement this once AssociationWidget's are implemented.
}

bool NewUMLRectWidget::loadFromXMI(QDomElement &qElement)
{
    if(!NewUMLWidget::loadFromXMI(qElement))
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

void NewUMLRectWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    NewUMLWidget::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }
}

/**
 * This is called on change of size of widget caused due to @ref
 * NewUMLRectWidget::setSize method.  This can be used to update the
 * shape of the widget.
 *
 * The default implementation sets QRectF(0, 0, newSize.width(),
 * newSize.height()) as the shape of this widget.
 *
 * @param oldSize The old size of the widget which can be used for
 *                computation.
 *
 * The new size is available through NewUMLRectWidget::size()
 */
void NewUMLRectWidget::sizeHasChanged(const QSizeF& oldSize)
{
    Q_UNUSED(oldSize);

    QPainterPath newShape;
    newShape.addRect(boundingRect());
    setShape(newShape);

    if(m_widgetHandle) {
        m_widgetHandle->updateHandlePosition();
    }
}

/**
 * Reimplemented to ensure widget current size fits the sizeHint
 * constraints.
 */
void NewUMLRectWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the sizeHint constraints.

    // Subclasses might calculate their sizeHint constraints
    setSize(m_size);
}

/**
 * Set whether this widget is resizable or not.
 */
void NewUMLRectWidget::setResizable(bool resizable)
{
    m_resizable = resizable;
}

void NewUMLRectWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    NewUMLWidget::mousePressEvent(event);
    m_oldGeometry = rect(); // save the current geometry
}

void NewUMLRectWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    NewUMLWidget::mouseMoveEvent(e);
}

void NewUMLRectWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    NewUMLWidget::mouseReleaseEvent(event);
}

void NewUMLRectWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

/**
 * Reimplemented to catch selection change notification and
 * enable/disable hover events based on whether this widget is
 * selected or not.
 */
QVariant NewUMLRectWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged) {
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
    }
    return NewUMLWidget::itemChange(change, value);
}


/////////////////////////// DEPRECATED STUFF ///////////////////////////
NewUMLRectWidget::NewUMLRectWidget(UMLScene *scene, UMLObject *object) :
    NewUMLWidget(scene, object),
    m_size(20, 20),
    m_resizable(true),
    m_widgetHandle(0)
{
}

NewUMLRectWidget::NewUMLRectWidget(UMLScene *scene, const Uml::IDType & id) :
    NewUMLWidget(scene, id),
    m_size(20, 20),
    m_resizable(true),
    m_widgetHandle(0)
{
}

ListPopupMenu* NewUMLRectWidget::setupPopupMenu()
{
    delete m_pMenu;
    m_pMenu = new ListPopupMenu(0, this, false, false);
    return m_pMenu;
}

#include "newumlrectwidget.moc"
