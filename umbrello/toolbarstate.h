/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATE_H
#define TOOLBARSTATE_H

#include <QEvent>
#include <QObject>
#include <QPointF>

class AssociationWidget;
class FloatingDashLineWidget;
class MessageWidget;
class QGraphicsSceneMouseEvent;
class UMLScene;
class UMLWidget;

/**
 * Base class for toolbar states.
 * All toolbar states inherit directly or indirectly from this class. Toolbar
 * states represent tools that work with the diagram (for example, to create
 * widgets, make associations...). All the mouse events received in the diagram
 * are delivered to the toolbar state currently active. The events are handled
 * in the tool and it executes the needed actions.
 *
 * All the mouse event handlers can be overridden in subclasses. However, the
 * behaviour of the main handlers shouldn't be modified (apart from extend it,
 * that is, call the base implementation before any other actions in the derived
 * method).
 *
 * In order to handle the events, each main handler has three protected
 * "sub-handlers" named like the main handler with the suffixes "Association",
 * "Widget" and "Empty". The events received in the main handlers are delivered
 * to the suitable handler, depending on if the event happened on an association,
 * on a widget or on an empty space of the diagram. Those methods are the ones to
 * override or extend to specify the behaviour of the toolbar state.
 *
 * The mouse events received in main handlers are tweaked to use the inverse
 * position. The modified event is saved in m_pMouseEvent. This is the event that
 * must be used everywhere.
 *
 * The association or widget that will receive the events is set in press event.
 * How they are set can be tweaked in subclasses overriding setCurrentElement().
 * Once a press event happens, all the mouse events and the release event are sent
 * to the same widget or association. Mouse events are delivered only when mouse
 * tracking is enabled. It is enabled in press event, and disabled in release
 * event. Also, it is disabled in the toolbar state initialization. Additionally,
 * it can be enabled or disabled in other situations by subclasses if needed.
 *
 * After handling a release event, the tool is changed if needed. Default
 * implementation sets the default tool if the button released was the right
 * button. Subclasses can override this behaviour if needed.
 *
 * When a toolbar state is selected, method init is called to revert its state
 * to the initial. Subclasses should extend that method as needed. Also, method
 * cleanBeforeChange() is called before changing it to the new tool. Subclasses
 * should extend that method as needed.
 *
 * @todo Handle, for example, left press, right press, left release, right
 *       release and other similar strange combinations?
 */
class ToolBarState: public QObject
{
    Q_OBJECT
public:
    virtual ~ToolBarState();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mousePress(QGraphicsSceneMouseEvent* ome);
    virtual void mouseRelease(QGraphicsSceneMouseEvent* ome);
    virtual void mouseDoubleClick(QGraphicsSceneMouseEvent* ome);
    virtual void mouseMove(QGraphicsSceneMouseEvent* ome);

public:
    Q_SLOT virtual void slotAssociationRemoved(AssociationWidget* association);
    Q_SLOT virtual void slotWidgetRemoved(UMLWidget* widget);

    // signals finish of operation
    Q_SIGNAL void finished();

protected:
    ToolBarState(UMLScene* umlScene);

    virtual void setCurrentElement();

    virtual void mousePressAssociation();
    virtual void mousePressWidget();
    virtual void mousePressEmpty();
    virtual void mouseReleaseAssociation();
    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();
    virtual void mouseDoubleClickAssociation();
    virtual void mouseDoubleClickWidget();
    virtual void mouseDoubleClickEmpty();
    virtual void mouseMoveAssociation();
    virtual void mouseMoveWidget();
    virtual void mouseMoveEmpty();

    virtual void changeTool();

    virtual UMLWidget* currentWidget() const;
    virtual void setCurrentWidget(UMLWidget* widget);

    virtual AssociationWidget* currentAssociation() const;
    virtual void setCurrentAssociation(AssociationWidget* association);

    void setMouseEvent(QGraphicsSceneMouseEvent* ome, const QEvent::Type &type);

    AssociationWidget* associationAt(const QPointF& pos);
    MessageWidget* messageAt(const QPointF& pos);
    FloatingDashLineWidget* floatingLineAt(const QPointF& pos);

    UMLScene*                 m_pUMLScene;     ///< The UMLScene.
    QGraphicsSceneMouseEvent* m_pMouseEvent;   ///< The mouse event currently in use.
    // This event is the equivalent of the received event after transforming it
    // using the inverse world matrix in the UMLScene.

private:
    UMLWidget*          m_currentWidget;       ///< The widget currently in use, if any.
    AssociationWidget*  m_currentAssociation;  ///< The association currently in use, if any.

};

#endif //TOOLBARSTATE_H
