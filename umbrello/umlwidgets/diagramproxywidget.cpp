/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "diagramproxywidget.h"

#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

DEBUG_REGISTER_DISABLED(DiagramProxyWidget)

DiagramProxyWidget::DiagramProxyWidget(UMLWidget *widget, qreal borderWidth)
  : m_diagramLinkId(Uml::ID::None)
  , m_widget(widget)
  , m_borderWidth(borderWidth)
{
}

Uml::ID::Type DiagramProxyWidget::diagramLink()
{
    return m_diagramLinkId;
}

UMLScene *DiagramProxyWidget::linkedDiagram()
{
    return m_linkedDiagram;
}

bool DiagramProxyWidget::setDiagramLink(const Uml::ID::Type &id)
{
    UMLView *view = UMLApp::app()->document()->findView(id);
    if (view) {
        m_diagramLinkId = id;
        m_linkedDiagram = view->umlScene();
    }
    return view;
}

/**
 * Return the area in which the linked diagram is displayed.
 *
 * @return area in current item coordinates
 */
const QRectF &DiagramProxyWidget::clientRect()
{
    return m_clientRect;
}

/**
 * Return scene area of the linked diagram.
 *
 * @return scene rectangle
 */
const QRectF &DiagramProxyWidget::sceneRect()
{
    return m_sceneRect;
}

/**
 * Set the area in which the linked diagram is displayed
 *
 * @param rect
 */
void DiagramProxyWidget::setClientRect(const QRectF &rect)
{
    m_clientRect = rect;
}

bool DiagramProxyWidget::activate(IDChangeLog *changeLog)
{
    Q_UNUSED(changeLog);

    if (m_diagramLinkId != Uml::ID::None)
        setDiagramLink(m_diagramLinkId);
    return true;
}

bool DiagramProxyWidget::loadFromXMI1(QDomElement &qElement)
{
    QString linkID = qElement.attribute(QLatin1String("diagramlinkid"), QLatin1String("-1"));
    m_diagramLinkId = Uml::ID::fromString(linkID);
    return true;
}

void DiagramProxyWidget::saveToXMI1(QDomDocument &qDoc, QDomElement &qElement)
{
    Q_UNUSED(qDoc);

    if (m_diagramLinkId != Uml::ID::None)
        qElement.setAttribute(QLatin1String("diagramlinkid"), Uml::ID::toString(m_diagramLinkId));
}

bool DiagramProxyWidget::isProxyWidget()
{
    return m_linkedDiagram;
}

UMLWidget *DiagramProxyWidget::getProxiedWidget(const QPointF &p)
{
    QPointF pos = m_widget->mapFromScene(p);
    if (!m_linkedDiagram || !m_clientRect.contains(pos))
        return nullptr;
    QPointF clientPos = mapToClient(pos);
    UMLWidget *w = m_linkedDiagram->widgetAt(clientPos);
    if (w)
        return w;
    return nullptr;
}

QPointF DiagramProxyWidget::mapFromClient(const QPointF &pos)
{
    QPointF p1 = pos - m_sceneRect.topLeft();
    qreal scaleW = m_sceneRect.width() / (m_clientRect.width() - m_borderWidth);
    qreal scaleH = m_sceneRect.height() / m_clientRect.height();
    QPointF p2 = QPointF(p1.x() / scaleW, p1.y() / scaleH);
    QPointF p3 = p2 + m_clientRect.topLeft();
    return m_widget->mapToScene(p3);
}

QRectF DiagramProxyWidget::mapFromClient(const QRectF &r)
{
    return QRectF(mapFromClient(r.topLeft()), mapFromClient(r.bottomRight()));
}

/**
 * Maps point from item coordinate to client scene coordinate system.
 *
 * @param pos item coordinated
 * @return point in client scene coordinate system
 */
QPointF DiagramProxyWidget::mapToClient(const QPointF &pos)
{
    QPointF p1 = pos - m_clientRect.topLeft();
    qreal scaleW = m_sceneRect.width() / (m_clientRect.width() - m_borderWidth);
    qreal scaleH = m_sceneRect.height() / m_clientRect.height();
    QPointF p2 = QPointF(p1.x() * scaleW, p1.y() * scaleH);
    QPointF p3 = p2 + m_sceneRect.topLeft();
    return p3;
}

DiagramProxyWidget &DiagramProxyWidget::operator=(const DiagramProxyWidget &other)
{
    m_diagramLinkId = other.m_diagramLinkId;
    m_linkedDiagram = other.m_linkedDiagram;
    m_widget = other.m_widget;
    m_sceneRect = other.m_sceneRect;
    m_clientRect = other.m_clientRect;
    return *this;
}

/**
 * Setup synthetizied graphics scene event
 *
 * @param e event to setup
 * @param event event source
 * @param pos position in item coordinates
 */
void DiagramProxyWidget::setupEvent(QGraphicsSceneMouseEvent &e, QGraphicsSceneMouseEvent *event, const QPointF & pos)
{
    QPointF p = mapToClient(pos);
    e.setScenePos(p);
    e.setPos(e.scenePos());
    QPointF lastPos = m_widget->mapFromScene(event->lastScenePos());
    QPointF lp = mapToClient(lastPos);
    e.setLastScenePos(lp);
    e.setModifiers(event->modifiers());
    e.setButtons(event->buttons());
    e.setButton(event->button());
}

/**
 * Setup synthetizied graphics scene context menu event
 *
 * @param e event to setup
 * @param event event source
 * @param pos position in item coordinates
 */
void DiagramProxyWidget::setupEvent(QGraphicsSceneContextMenuEvent &e, QGraphicsSceneContextMenuEvent *event, const QPointF & pos)
{
    QPointF p = mapToClient(pos);
    e.setScenePos(p);
    e.setPos(e.scenePos());
    QPoint sp = event->screenPos();
    e.setScreenPos(sp);
    e.setModifiers(event->modifiers());
    e.setReason(event->reason());
}

void DiagramProxyWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QPointF pos = m_widget->mapFromScene(event->scenePos());
    if (m_linkedDiagram && m_clientRect.contains(pos)) {
        QGraphicsSceneContextMenuEvent e(event->type());
        setupEvent(e, event, pos);
        m_linkedDiagram->contextMenuEvent(&e);
        m_widget->update();
        event->ignore();
    }
}

void DiagramProxyWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = m_widget->mapFromScene(event->scenePos());
    if (m_linkedDiagram && m_clientRect.contains(pos)) {
        QGraphicsSceneMouseEvent e(event->type());
        setupEvent(e, event, pos);
        m_linkedDiagram->mouseDoubleClickEvent(&e);
        m_widget->update();
        event->ignore();
    }
}

void DiagramProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = m_widget->mapFromScene(event->scenePos());
    if (m_linkedDiagram && m_clientRect.contains(pos)) {
        QGraphicsSceneMouseEvent e(event->type());
        setupEvent(e, event, pos);
        m_linkedDiagram->mousePressEvent(&e);
        m_widget->update();
        event->ignore();
    }
}

void DiagramProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = m_widget->mapFromScene(event->scenePos());
    if (m_linkedDiagram && m_clientRect.contains(pos)) {
        QGraphicsSceneMouseEvent e(event->type());
        setupEvent(e, event, pos);
        m_linkedDiagram->mouseMoveEvent(&e);
        m_widget->update();
        event->ignore();
    }
}

void DiagramProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = m_widget->mapFromScene(event->scenePos());
    if (m_linkedDiagram && m_clientRect.contains(pos)) {
        QGraphicsSceneMouseEvent e(event->type());
        setupEvent(e, event, pos);
        m_linkedDiagram->mouseReleaseEvent(&e);
        m_widget->update();
        event->ignore();
    }
}

/**
 * Paint linked diagram into current widget
 *
 * @param painter painter to paint on
 * @param option The option parameter provides style options for the item, such as its state, exposed area and its level-of-detail hints
 * @param widget The widget argument is optional. If provided, it points to the widget that is being painted on; otherwise, it is 0
 */
void DiagramProxyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    m_sceneRect = linkedDiagram()->sceneRect().adjusted(-1,-1, 1, 1);
    if (Tracer::instance()->isEnabled(QLatin1String("DiagramProxyWidget"))) {
        painter->setPen(Qt::magenta);
        painter->drawRect(m_clientRect);
    }
    m_linkedDiagram->render(painter, m_clientRect, m_sceneRect);
}
