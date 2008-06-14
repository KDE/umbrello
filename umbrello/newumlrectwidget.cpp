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

#include <QtGui/QDialog>
#include <kdebug.h>

NewUMLRectWidget::NewUMLRectWidget (UMLObject *object) :
    NewUMLWidget(object),
    m_size(20, 20),
    m_resizable(true),
    m_isResizing(false)
{
}

NewUMLRectWidget::~NewUMLRectWidget()
{
}

void NewUMLRectWidget::setSize(const QSizeF &size, const QPainterPath &newShape)
{
    QSizeF minSize = sizeHint(Qt::MinimumSize);
    QSizeF maxSize = sizeHint(Qt::MaximumSize);

    m_size = minSize.expandedTo(size);
    m_size = m_size.boundedTo(maxSize);

    QRectF boundRect(QPointF(0, 0), m_size);
    /*
     * There is no need to adjust half the pen width if the widget is
     * resizable since the small resize hint rectangles themselves
     * ensure the geometry of widget adjusts even for half pen width.
     */
    if(isResizable()) {

    }
    else {
        // Adjust bounding rect with half the pen width.
        qreal hpw = 0.5 * pen().width();
        boundRect.adjust(-hpw, -hpw, hpw, hpw);
    }
    setBoundingRectAndShape(boundRect, newShape);
}

void NewUMLRectWidget::setInstanceName(const QString &name)
{
    m_instanceName = name;
    updateGeometry();
}

void NewUMLRectWidget::addAssociationWidget(AssociationWidget *assoc)
{
    if(!m_associationWidgetList.contains(assoc)) {
        m_associationWidgetList << assoc;
    }
}

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
    NewUMLRectWidget::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }
}

void NewUMLRectWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the sizeHint constraints.

    // Subclasses might calculate their sizeHint constraints as well
    // as their shape in this reimplemented method.
    setSize(m_size, shape());
}

void NewUMLRectWidget::setResizable(bool resizable)
{
    m_resizable = resizable;
    updateGeometry();
}

void NewUMLRectWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    NewUMLWidget::mousePressEvent(event);
    if(!isResizable()) {
        return;
    }
}

void NewUMLRectWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!isResizable() || !m_isResizing) {
        NewUMLWidget::mouseMoveEvent(event);
        return;
    }
}

void NewUMLRectWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(!isResizable() || !m_isResizing) {
        NewUMLWidget::mouseReleaseEvent(event);
    }
    m_isResizing = false;
}

void NewUMLRectWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

}

#include "newumlrectwidget.moc"
