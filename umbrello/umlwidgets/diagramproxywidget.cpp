/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "diagramproxywidget.h"

#include "cmds/cmdcreatediagram.h"
#include "debug_utils.h"
#include "diagram_utils.h"
#include "dialog_utils.h"
#include "widget_utils.h"
#include "listpopupmenu.h"
#include "statewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"
#include "selectdiagramdialog.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(DiagramProxyWidget)

DiagramProxyWidget::DiagramProxyWidget(UMLWidget *widget, qreal borderWidth)
  : m_diagramLinkId(Uml::ID::None)
  , m_iconRect(QRectF(0, 0, 10, 10))
  , m_widget(widget)
  , m_borderWidth(borderWidth)
  , m_showLinkedDiagram(true)
{
}

Uml::ID::Type DiagramProxyWidget::diagramLink() const
{
    return m_diagramLinkId;
}

UMLScene *DiagramProxyWidget::linkedDiagram() const
{
    return m_linkedDiagram;
}

bool DiagramProxyWidget::setDiagramLink(const Uml::ID::Type &id)
{
    if (id == Uml::ID::None) {
        m_diagramLinkId = id;
        m_linkedDiagram = nullptr;
        m_widget->updateGeometry(true);
        return true;
    }
    UMLView *view = UMLApp::app()->document()->findView(id);
    if (view) {
        m_diagramLinkId = id;
        m_linkedDiagram = view->umlScene();
        m_widget->updateGeometry(true);
    }
    return view;
}

/**
 * Return the area in which the linked diagram is displayed.
 *
 * @return area in current item coordinates
 */
const QRectF &DiagramProxyWidget::clientRect() const
{
    return m_clientRect;
}

/**
 * Return scene area of the linked diagram.
 *
 * @return scene rectangle
 */
const QRectF &DiagramProxyWidget::sceneRect() const
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

bool DiagramProxyWidget::loadFromXMI(QDomElement &qElement)
{
    QString linkID = qElement.attribute(QStringLiteral("diagramlinkid"), QStringLiteral("-1"));
    m_diagramLinkId = Uml::ID::fromString(linkID);
    return true;
}

void DiagramProxyWidget::saveToXMI(QXmlStreamWriter& writer)
{
    if (m_diagramLinkId != Uml::ID::None)
        writer.writeAttribute(QStringLiteral("diagramlinkid"), Uml::ID::toString(m_diagramLinkId));
}

bool DiagramProxyWidget::isProxyWidget() const
{
    return m_linkedDiagram;
}

UMLWidget *DiagramProxyWidget::getProxiedWidget(const QPointF &p) const
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

QPointF DiagramProxyWidget::mapFromClient(const QPointF &pos) const
{
    QPointF p1 = pos - m_sceneRect.topLeft();
    qreal scaleW = m_sceneRect.width() / (m_clientRect.width() - m_borderWidth);
    qreal scaleH = m_sceneRect.height() / m_clientRect.height();
    QPointF p2 = QPointF(p1.x() / scaleW, p1.y() / scaleH);
    QPointF p3 = p2 + m_clientRect.topLeft();
    return m_widget->mapToScene(p3);
}

QRectF DiagramProxyWidget::mapFromClient(const QRectF &r) const
{
    return QRectF(mapFromClient(r.topLeft()), mapFromClient(r.bottomRight()));
}

/**
 * Maps point from item coordinate to client scene coordinate system.
 *
 * @param pos item coordinated
 * @return point in client scene coordinate system
 */
QPointF DiagramProxyWidget::mapToClient(const QPointF &pos) const
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
 * Set up synthetic graphics scene event
 *
 * @param e event to setup
 * @param event event source
 * @param pos position in item coordinates
 */
void DiagramProxyWidget::setupEvent(QGraphicsSceneMouseEvent &e,
                                    const QGraphicsSceneMouseEvent *event, const QPointF & pos) const
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
 * Set up synthetic graphics scene context menu event
 *
 * @param e event to setup
 * @param event event source
 * @param pos position in item coordinates
 */
void DiagramProxyWidget::setupEvent(QGraphicsSceneContextMenuEvent &e,
                                    const QGraphicsSceneContextMenuEvent *event, const QPointF & pos) const
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
    if (m_showLinkedDiagram) {
        QPointF pos = m_widget->mapFromScene(event->scenePos());
        if (m_linkedDiagram && m_clientRect.contains(pos)) {
            QGraphicsSceneMouseEvent e(event->type());
            setupEvent(e, event, pos);
            m_linkedDiagram->mouseDoubleClickEvent(&e);
            m_widget->update();
            event->ignore();
        }
    } else {
        QPointF p = m_widget->mapFromScene(event->scenePos());
        if (m_iconRect.contains(p)) {
            linkedDiagram()->setWidgetLink(dynamic_cast<WidgetBase *>(this));
            UMLApp::app()->document()->changeCurrentView(diagramLink());
            event->ignore();
        }
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
 * Getter for icon rectangle
 * @return icon rectangle
 */
QRectF DiagramProxyWidget::iconRect() const
{
    return m_iconRect;
}

/**
 * Setter for icon rectangle
 * @param iconRect icon rectangle
 */
void DiagramProxyWidget::setIconRect(const QRectF &iconRect)
{
    m_iconRect = iconRect;
}

/**
 * Return state of 'show linked diagram' attribute
 * @return state
 */
bool DiagramProxyWidget::showLinkedDiagram() const
{
    return m_showLinkedDiagram;
}

/**
 * Set state for 'show linked diagram' attribute
 * @param showLinkedDiagram state to set
 */
void DiagramProxyWidget::setShowLinkedDiagram(bool showLinkedDiagram)
{
    m_showLinkedDiagram = showLinkedDiagram;
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

    if (m_showLinkedDiagram) {
        m_sceneRect = linkedDiagram()->sceneRect().adjusted(-1,-1, 1, 1);
        if (Tracer::instance()->isEnabled(QStringLiteral("DiagramProxyWidget"))) {
            painter->setPen(Qt::magenta);
            painter->drawRect(m_clientRect);
        }
        m_linkedDiagram->render(painter, m_clientRect, m_sceneRect);
    } else {
        QPixmap p = Icon_Utils::smallIcon(Uml::DiagramType::State);
        QRectF source(0,0, p.width(), p.height());
        painter->drawPixmap(m_iconRect, p, source);
    }
}

/**
 * Captures any popup menu signals for menus it created.
 *
 * If the provided action is not handled, it will be forwarded
 * to the contained widget.
 *
 * @param action action to handle
 */
void DiagramProxyWidget::slotMenuSelection(QAction* action)
{
    switch(ListPopupMenu::typeFromAction(action)) {
    // classifier widget
    case ListPopupMenu::mt_State_Diagram:
        {
            QString name = Widget_Utils::defaultWidgetName(WidgetBase::WidgetType::wt_State);
            bool ok;
            do {
                if (!Diagram_Utils::isUniqueDiagramName(Uml::DiagramType::State, name))
                    name.append(QStringLiteral("_1"));
                ok = Dialog_Utils::askNewName(WidgetBase::WidgetType::wt_State, name);
            } while(ok && !Diagram_Utils::isUniqueDiagramName(Uml::DiagramType::State, name));
            if (ok) {
                Uml::CmdCreateDiagram* d = new Uml::CmdCreateDiagram(UMLApp::app()->document(), Uml::DiagramType::State, name);
                UMLScene *scene = d->view()->umlScene();
                UMLApp::app()->executeCommand(d);
                setShowLinkedDiagram(false);
                setDiagramLink(scene->ID());
                scene->setWidgetLink(m_widget);
            }
        }
        break;

    // state widget
    case ListPopupMenu::mt_CombinedState:
        {
            QString diagramName = UMLApp::app()->document()->createDiagramName(Uml::DiagramType::State);
            Uml::CmdCreateDiagram* d = new Uml::CmdCreateDiagram(UMLApp::app()->document(), Uml::DiagramType::State, diagramName);
            UMLApp::app()->executeCommand(d);
            setDiagramLink(d->view()->umlScene()->ID());
            m_widget->asStateWidget()->setStateType(StateWidget::Combined);
        }
        break;

    case ListPopupMenu::mt_SelectStateDiagram:
    {
        SelectDiagramDialog dlg(nullptr, Uml::DiagramType::State, linkedDiagram() ? linkedDiagram()->name() : QString(), QString());
        if (dlg.exec()) {
            setDiagramLink(dlg.currentID());
        }
        break;
    }

    // classifier widget
    case ListPopupMenu::mt_GoToStateDiagram:
    // state widget
    case ListPopupMenu::mt_EditCombinedState:
        if (!linkedDiagram()) {
            logError1("DiagramProxyWidget::slotMenuSelection: no diagram id defined at widget id=%1",
                      Uml::ID::toString(m_widget->id()));
            break;
        }
        linkedDiagram()->setWidgetLink(m_widget);
        UMLApp::app()->document()->changeCurrentView(diagramLink());
        break;

    case ListPopupMenu::mt_RemoveStateDiagram:
        setDiagramLink(Uml::ID::None);
        break;

    default:
        m_widget->UMLWidget::slotMenuSelection(action);
        break;
    }
}
