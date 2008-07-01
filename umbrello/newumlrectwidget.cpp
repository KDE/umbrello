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

#include <kdebug.h>

const QSizeF NewUMLRectWidget::DefaultMinimumSize(50, 20);
const QSizeF NewUMLRectWidget::DefaultMaximumSize(1000, 1000);
const QSizeF NewUMLRectWidget::DefaultPreferredSize(70, 20);

NewUMLRectWidget::NewUMLRectWidget(UMLObject *object) :
    NewUMLWidget(object),
    m_size(20, 20),
    m_resizable(true),
    m_widgetHandle(0)
{
}

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

NewUMLRectWidget::~NewUMLRectWidget()
{
}

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

void NewUMLRectWidget::setSize(const QSizeF &size)
{
    QSizeF oldSize = m_size;

    const QSizeF minSize = sizeHint(Qt::MinimumSize);
    const QSizeF maxSize = sizeHint(Qt::MaximumSize);

    m_size = minSize.expandedTo(size);
    m_size = maxSize.boundedTo(m_size);


    QRectF boundRect = rect();
    // Adjust bounding rect with half the pen width(lineWidth).
    qreal hpw = 0.5 * lineWidth();
    boundRect.adjust(-hpw, -hpw, hpw, hpw);

    setBoundingRect(boundRect);

    // Notify by calling sizeHasChanged method with old size
    sizeHasChanged(oldSize);
}

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
    NewUMLWidget::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }
}

void NewUMLRectWidget::sizeHasChanged(const QSizeF& oldSize)
{
    Q_UNUSED(oldSize);

    QPainterPath newShape;
    newShape.addRect(boundingRect());
    setShape(newShape);
}

void NewUMLRectWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the sizeHint constraints.

    // Subclasses might calculate their sizeHint constraints
    setSize(m_size);
}

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

ListPopupMenu* NewUMLRectWidget::setupPopupMenu()
{
    delete m_pMenu;
    m_pMenu = new ListPopupMenu(0, this, false, false);
    return m_pMenu;
}

#include "newumlrectwidget.moc"
