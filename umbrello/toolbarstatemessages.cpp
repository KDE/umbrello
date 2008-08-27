/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatemessages.h"

// kde includes
#include <kdebug.h>

// local includes
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlscene.h"

#include <klocale.h>
#include <kmessagebox.h>

ToolBarStateMessages::ToolBarStateMessages(UMLScene *umlScene) : ToolBarStatePool(umlScene)
{
    m_firstObject = 0;
    m_messageLine = 0;
    xclick = 0;
    yclick = 0;
}

ToolBarStateMessages::~ToolBarStateMessages()
{
    delete m_messageLine;
}

void ToolBarStateMessages::init()
{
    ToolBarStatePool::init();

    cleanMessage();
}

void ToolBarStateMessages::cleanBeforeChange()
{
    ToolBarStatePool::cleanBeforeChange();

    cleanMessage();
}

void ToolBarStateMessages::mouseMove(QGraphicsSceneMouseEvent* ome) {
    ToolBarStatePool::mouseMove(ome);

    if (m_messageLine) {
        QPointF sp = m_messageLine->line().p1();
        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(sp.x(), sp.y(), x, y);
    }
}

void ToolBarStateMessages::slotWidgetRemoved(NewUMLRectWidget* widget)
{
    ToolBarState::slotWidgetRemoved(widget);

    if (widget == m_firstObject) {
        cleanMessage();
    }
}

void ToolBarStateMessages::setCurrentElement()
{
    m_isObjectWidgetLine = false;

    ObjectWidget* objectWidgetLine = m_pUMLScene->onWidgetLine(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        qDebug() << Q_FUNC_INFO << "Object detected";
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
        return;
    }
    qDebug() << Q_FUNC_INFO << "Object NOT detected";
    //commit 515177 fixed a setting creation messages only working properly at 100% zoom
    //However, the applied patch doesn't seem to be necessary no more, so it was removed
    //The widgets weren't got from UMLView, but from a method in this class similarto the
    //one in UMLView but containing special code to handle the zoom
    NewUMLRectWidget *widget = m_pUMLScene->getWidgetAt(m_pMouseEvent->scenePos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

void ToolBarStateMessages::mouseReleaseWidget()
{
    //TODO When an association between UMLObjects of invalid types is made, an error message
    //is shown. Shouldn't also a message be used here?
    if (m_pMouseEvent->button() != Qt::LeftButton ||
                getCurrentWidget()->getBaseType() != Uml::wt_Object) {
        cleanMessage();
        return;
    }

    if (!m_isObjectWidgetLine && !m_firstObject) {
        return;
    }

    if (!m_isObjectWidgetLine) {
        setSecondWidget(static_cast<ObjectWidget*>(getCurrentWidget()), CreationMessage);
        return;
    }

    if (!m_firstObject) {
        setFirstWidget(static_cast<ObjectWidget*>(getCurrentWidget()));
    } else {
        setSecondWidget(static_cast<ObjectWidget*>(getCurrentWidget()), NormalMessage);
    }
}

void ToolBarStateMessages::mouseReleaseEmpty()
{
    Uml::Sequence_Message_Type msgType = getMessageType();

    if (m_firstObject && msgType ==  Uml::sequence_message_lost) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        MessageWidget* message = new MessageWidget(m_firstObject, QPointF(xclick, yclick), msgType);
        m_pUMLScene->addWidget(message);
        cleanMessage();
        m_pUMLScene->getMessageList().append(message);
        xclick = 0;
        yclick = 0;

        FloatingTextWidget *ft = message->floatingTextWidget();
        //TODO cancel doesn't cancel the creation of the message, only cancels setting an operation.
        //Shouldn't it cancel also the whole creation?
        ft->showOperationDialog();
        message->setTextPosition();
        m_pUMLScene->getWidgetList().append(ft);

        UMLApp::app()->getDocument()->setModified();
    }

    else if (!m_firstObject && msgType == Uml::sequence_message_found && xclick == 0 && yclick == 0) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        m_messageLine = new QGraphicsLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->getLineColor(), m_pUMLScene->getLineWidth(), Qt::DashLine));

        m_messageLine->setVisible(true);

        // [PORT]
        // m_pUMLScene->viewport()->setMouseTracking(true);
    }
    else
        cleanMessage();
}

void ToolBarStateMessages::setFirstWidget(ObjectWidget* firstObject)
{
    m_firstObject = firstObject;
    Uml::Sequence_Message_Type msgType = getMessageType();

    if (msgType ==  Uml::sequence_message_found && xclick!=0 && yclick!=0) {
        MessageWidget* message = new MessageWidget(m_firstObject, QPointF(xclick, yclick), msgType);
        m_pUMLScene->addWidget(message);
        cleanMessage();
        m_pUMLScene->getMessageList().append(message);

        xclick = 0;
        yclick = 0;

        FloatingTextWidget *ft = message->floatingTextWidget();
        //TODO cancel doesn't cancel the creation of the message, only cancels setting an operation.
        //Shouldn't it cancel also the whole creation?
        ft->showOperationDialog();
        message->setTextPosition();
        m_pUMLScene->getWidgetList().append(ft);

        UMLApp::app()->getDocument()->setModified();
    }
    else {
        m_messageLine = new QGraphicsLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x();
        qreal y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->getLineColor(), m_pUMLScene->getLineWidth(), Qt::DashLine));

        m_messageLine->setVisible(true);

        // [PORT]
        // m_pUMLScene->viewport()->setMouseTracking(true);
    }
}

void ToolBarStateMessages::setSecondWidget(ObjectWidget* secondObject, MessageType messageType) {
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
    m_pUMLScene->addWidget(message);
    message->setY(y);

    cleanMessage();

    m_pUMLScene->getMessageList().append(message);

    FloatingTextWidget *ft = message->floatingTextWidget();
    if (ft) {

        //TODO cancel doesn't cancel the creation of the message, only cancels setting an operation.
        //Shouldn't it cancel also the whole creation?
        ft->showOperationDialog();
        message->setTextPosition();
        m_pUMLScene->getWidgetList().append(ft);
    }
    UMLApp::app()->getDocument()->setModified();
}

Uml::Sequence_Message_Type ToolBarStateMessages::getMessageType() {
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

void ToolBarStateMessages::cleanMessage() {
    m_firstObject = 0;

    delete m_messageLine;
    m_messageLine = 0;
}

#include "toolbarstatemessages.moc"
