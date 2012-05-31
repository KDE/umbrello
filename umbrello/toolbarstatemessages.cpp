/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatemessages.h"

// local includes
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlscene.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

/**
 * Creates a new ToolBarStateMessages.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateMessages::ToolBarStateMessages(UMLScene *umlScene)
  : ToolBarStatePool(umlScene),
    m_firstObject(0),
    m_messageLine(0),
    m_isObjectWidgetLine(false),
    xclick(0),
    yclick(0)
{
}

/**
 * Destroys this ToolBarStateMessages.
 */
ToolBarStateMessages::~ToolBarStateMessages()
{
    delete m_messageLine;
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateMessages::init()
{
    ToolBarStatePool::init();

    cleanMessage();
}

/**
 * Called when the current tool is changed to use another tool.
 * Executes base method and cleans the message.
 */
void ToolBarStateMessages::cleanBeforeChange()
{
    ToolBarStatePool::cleanBeforeChange();

    cleanMessage();
}

/**
 * Called when a mouse event happened.
 * It executes the base method and then updates the position of the
 * message line, if any.
 */
void ToolBarStateMessages::mouseMove(UMLSceneMouseEvent* ome)
{
    ToolBarStatePool::mouseMove(ome);

    if (m_messageLine) {
        QPointF sp = m_messageLine->line().p1();
        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(sp.x(), sp.y(), x, y);
    }
}

/**
 * A widget was removed from the UMLView.
 * If the widget removed was the current widget, the current widget is set
 * to 0.
 * Also, if it was the first object, the message is cleaned.
 */
void ToolBarStateMessages::slotWidgetRemoved(UMLWidget* widget)
{
    ToolBarState::slotWidgetRemoved(widget);

    if (widget == m_firstObject) {
        cleanMessage();
    }
}

/**
 * Selects only widgets, but no associations.
 * Overrides base class method.
 * If the press event happened on the line of an object, the object is set
 * as current widget. If the press event happened on a widget, the widget is
 * set as current widget.
 */
void ToolBarStateMessages::setCurrentElement()
{
    m_isObjectWidgetLine = false;

    ObjectWidget* objectWidgetLine = m_pUMLScene->onWidgetLine(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        uDebug() << Q_FUNC_INFO << "Object detected";
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
        return;
    }
    uDebug() << Q_FUNC_INFO << "Object NOT detected";
    //commit 515177 fixed a setting creation messages only working properly at 100% zoom
    //However, the applied patch doesn't seem to be necessary no more, so it was removed
    //The widgets weren't got from UMLView, but from a method in this class similarto the
    //one in UMLView but containing special code to handle the zoom
    UMLWidget *widget = m_pUMLScene->widgetAt(m_pMouseEvent->scenePos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

/**
 * Called when the release event happened on a widget.
 * If the button pressed isn't left button or the widget isn't an object
 * widget, the message is cleaned.
 * If the release event didn't happen on the line of an object and the first
 * object wasn't selected, nothing is done. If the first object was already
 * selected, a creation message is made.
 * If the event happened on the line of an object, the first object or the
 * second are set, depending on whether the first object was already set or
 * not.
 */
void ToolBarStateMessages::mouseReleaseWidget()
{
    //TODO When an association between UMLObjects of invalid types is made, an error message
    //is shown. Shouldn't also a message be used here?
    if (m_pMouseEvent->button() != Qt::LeftButton ||
                currentWidget()->baseType() != WidgetBase::wt_Object) {
        cleanMessage();
        return;
    }

    if (!m_isObjectWidgetLine && !m_firstObject) {
        return;
    }

    if (!m_isObjectWidgetLine) {
        setSecondWidget(static_cast<ObjectWidget*>(currentWidget()), CreationMessage);
        return;
    }

    if (!m_firstObject) {
        setFirstWidget(static_cast<ObjectWidget*>(currentWidget()));
    } else {
        setSecondWidget(static_cast<ObjectWidget*>(currentWidget()), NormalMessage);
    }
}

/**
 * Called when the release event happened on an empty space.
 * Cleans the message.
 * Empty spaces are not only actual empty spaces, but also associations.
 */
void ToolBarStateMessages::mouseReleaseEmpty()
{
    Uml::Sequence_Message_Type msgType = getMessageType();

    if (m_firstObject && msgType ==  Uml::sequence_message_lost) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        MessageWidget* message = new MessageWidget(m_firstObject, QPointF(xclick, yclick), msgType);
        setupMessageWidget(message);
        cleanMessage();
        xclick = 0;
        yclick = 0;
    }

    else if (!m_firstObject && msgType == Uml::sequence_message_found && xclick == 0 && yclick == 0) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        m_messageLine = new UMLSceneLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->lineColor(), m_pUMLScene->lineWidth(), Qt::DashLine));

        m_messageLine->setVisible(true);
    }
    else {
        cleanMessage();
    }
}

/**
 * Sets the first object of the message using the specified object.
 * The temporal visual message is created and mouse tracking enabled, so
 * mouse events will be delivered.
 *
 * @param firstObject The first object of the message.
 */
void ToolBarStateMessages::setFirstWidget(ObjectWidget* firstObject)
{
    m_firstObject = firstObject;
    Uml::Sequence_Message_Type msgType = getMessageType();

    if (msgType ==  Uml::sequence_message_found && xclick!=0 && yclick!=0) {
        MessageWidget* message = new MessageWidget(m_firstObject, QPointF(xclick, yclick), msgType);
        setupMessageWidget(message);
        cleanMessage();
        xclick = 0;
        yclick = 0;
    }
    else {
        m_messageLine = new UMLSceneLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x();
        qreal y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->lineColor(), m_pUMLScene->lineWidth(), Qt::DashLine));

        m_messageLine->setVisible(true);
    }
}

/**
 * Sets the second object of the message using the specified widget and
 * creates the message.
 * The association is created and added to the view. The dialog to select
 * the operation of the message is shown.
 *
 * @param secondObject The second object of the message.
 * @param messageType The type of the message to create.
 */
void ToolBarStateMessages::setSecondWidget(ObjectWidget* secondObject, MessageType messageType)
{
    Uml::Sequence_Message_Type msgType = getMessageType();

    //There shouldn't be second widget for a lost or a found message
    if (msgType == Uml::sequence_message_lost || msgType == Uml::sequence_message_found) {
        cleanMessage();
        xclick = 0;
        yclick = 0;
        return;
    }
    //TODO shouldn't start position in the first widget be used also for normal messages
    //and not only for creation?
    qreal y = m_pMouseEvent->scenePos().y();
    if (messageType == CreationMessage) {
        msgType = Uml::sequence_message_creation;
        y = m_messageLine->line().p1().y();
    }

    MessageWidget* message = new MessageWidget(m_firstObject,
                                               secondObject, msgType);
    message->setPos(message->pos().x(), y);
    setupMessageWidget(message);
    cleanMessage();
}

/**
 * Returns the message type of this tool.
 *
 * @return The message type of this tool.
 */
Uml::Sequence_Message_Type ToolBarStateMessages::getMessageType()
{
    if (getButton() == WorkToolBar::tbb_Seq_Message_Synchronous) {
        return Uml::sequence_message_synchronous;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Found) {
        return Uml::sequence_message_found;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Lost) {
        return Uml::sequence_message_lost;
    }
    return Uml::sequence_message_asynchronous;
}

/**
 * Cleans the first widget and the temporal message line, if any.
 * Both are set to null, and the message line is also deleted.
 */
void ToolBarStateMessages::cleanMessage()
{
    m_firstObject = 0;

    delete m_messageLine;
    m_messageLine = 0;
}

void ToolBarStateMessages::setupMessageWidget(MessageWidget *message)
{
    m_pUMLScene->messageList().append(message);
    m_pUMLScene->addItem(message);
    message->activate();

    FloatingTextWidget *ft = message->floatingTextWidget();
    //TODO cancel doesn't cancel the creation of the message, only cancels setting an operation.
    //Shouldn't it cancel also the whole creation?
    ft->showOperationDialog();
    message->setTextPosition();
    m_pUMLScene->widgetList().append(ft);

    UMLApp::app()->document()->setModified();
}

#include "toolbarstatemessages.moc"
