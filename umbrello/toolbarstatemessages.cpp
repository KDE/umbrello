/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "toolbarstatemessages.h"

// local includes
#include "cmds.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "object_factory.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlscene.h"
#include "widget_factory.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

DEBUG_REGISTER(ToolBarStateMessages)

/**
 * Creates a new ToolBarStateMessages.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateMessages::ToolBarStateMessages(UMLScene *umlScene)
  : ToolBarStatePool(umlScene),
    m_firstObject(nullptr),
    m_messageLine(nullptr),
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
    cleanMessage();
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
void ToolBarStateMessages::mouseMove(QGraphicsSceneMouseEvent* ome)
{
    ToolBarStatePool::mouseMove(ome);

    if (m_messageLine) {
        QPointF sp = m_messageLine->line().p1();
        m_messageLine->setLine(sp.x(), sp.y(), m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y());
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
    const QString funcInfo(QString::fromLatin1(Q_FUNC_INFO));
    if (objectWidgetLine) {
        logDebug1("ToolBarStateMessages::setCurrentElement %1 Object detected", funcInfo);
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
        return;
    }
    logDebug1("ToolBarStateMessages::setCurrentElement %1 Object NOT detected", funcInfo);
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
                !currentWidget()->isObjectWidget()) {
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
    Uml::SequenceMessage::Enum msgType = getMessageType();

    if (m_firstObject && msgType ==  Uml::SequenceMessage::Creation) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        bool state = m_pUMLScene->getCreateObject();
        m_pUMLScene->setCreateObject(false);
        UMLObject *object = Object_Factory::createUMLObject(UMLObject::ot_Class);
        m_pUMLScene->setCreateObject(state);
        if (object) {
            ObjectWidget *widget = (ObjectWidget *)Widget_Factory::createWidget(m_pUMLScene, object);
            widget->setX(xclick);
            widget->activate();
            m_pUMLScene->addWidgetCmd(widget);

            MessageWidget* message = new MessageWidget(m_pUMLScene, m_firstObject, widget, yclick, msgType);
            setupMessageWidget(message, false);
        }
        cleanMessage();
        xclick = 0;
        yclick = 0;
    } else if (m_firstObject && msgType ==  Uml::SequenceMessage::Lost) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();

        MessageWidget* message = new MessageWidget(m_pUMLScene, m_firstObject, xclick, yclick, msgType);
        setupMessageWidget(message);
        cleanMessage();
        xclick = 0;
        yclick = 0;
    }

    else if (!m_firstObject && msgType == Uml::SequenceMessage::Found && xclick == 0 && yclick == 0) {
        xclick = m_pMouseEvent->scenePos().x();
        yclick = m_pMouseEvent->scenePos().y();
        cleanMessage();
        m_messageLine = new QGraphicsLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x();
        qreal y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->lineColor(), m_pUMLScene->lineWidth(), Qt::DashLine));
        m_messageLine->setVisible(true);

        m_pUMLScene->activeView()->viewport()->setMouseTracking(true);
    }
    else {
        cleanMessage();
    }
}

/**
 * Sets the first object of the message using the specified object.
 * The temporary visual message is created and mouse tracking enabled, so
 * mouse events will be delivered.
 *
 * @param firstObject The first object of the message.
 */
void ToolBarStateMessages::setFirstWidget(ObjectWidget* firstObject)
{
    m_firstObject = firstObject;
    Uml::SequenceMessage::Enum msgType = getMessageType();

    if (msgType ==  Uml::SequenceMessage::Found && xclick!=0 && yclick!=0) {
        MessageWidget* message = new MessageWidget(m_pUMLScene, m_firstObject, xclick, yclick, msgType);
        setupMessageWidget(message);
        cleanMessage();
        xclick = 0;
        yclick = 0;
    }
    else {
        // TODO use cleanMessage()
        if (m_messageLine)
            delete m_messageLine;
        m_messageLine = new QGraphicsLineItem();
        m_pUMLScene->addItem(m_messageLine);
        qreal x = m_pMouseEvent->scenePos().x();
        qreal y = m_pMouseEvent->scenePos().y();
        m_messageLine->setLine(x, y, x, y);
        m_messageLine->setPen(QPen(m_pUMLScene->lineColor(), m_pUMLScene->lineWidth(), Qt::DashLine));
        m_messageLine->setVisible(true);

        m_pUMLScene->activeView()->viewport()->setMouseTracking(true);
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
    Uml::SequenceMessage::Enum msgType = getMessageType();

    //There shouldn't be second widget for a lost or a found message
    if (msgType == Uml::SequenceMessage::Lost || msgType == Uml::SequenceMessage::Found) {
        cleanMessage();
        xclick = 0;
        yclick = 0;
        return;
    }
    qreal y = m_messageLine->line().p1().y();
    if (messageType == CreationMessage) {
        msgType = Uml::SequenceMessage::Creation;
    }

    MessageWidget* message = new MessageWidget(m_pUMLScene, m_firstObject,
                                               secondObject, y, msgType);
    setupMessageWidget(message);
    cleanMessage();
}

/**
 * Returns the message type of this tool.
 *
 * @return The message type of this tool.
 */
Uml::SequenceMessage::Enum ToolBarStateMessages::getMessageType()
{
    if (getButton() == WorkToolBar::tbb_Seq_Message_Creation) {
        return Uml::SequenceMessage::Creation;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Destroy) {
        return Uml::SequenceMessage::Destroy;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Synchronous) {
        return Uml::SequenceMessage::Synchronous;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Found) {
        return Uml::SequenceMessage::Found;
    }
    else if (getButton() == WorkToolBar::tbb_Seq_Message_Lost) {
        return Uml::SequenceMessage::Lost;
    }
    return Uml::SequenceMessage::Asynchronous;
}

/**
 * Cleans the first widget and the temporary message line, if any.
 * Both are set to null, and the message line is also deleted.
 */
void ToolBarStateMessages::cleanMessage()
{
    m_firstObject = nullptr;

    if (m_messageLine) {
        delete m_messageLine;
        m_messageLine = nullptr;
    }
}

void ToolBarStateMessages::setupMessageWidget(MessageWidget *message, bool showOperationDialog)
{
    if (showOperationDialog) {
        FloatingTextWidget *ft = message->floatingTextWidget();
        //TODO cancel doesn't cancel the creation of the message, only cancels setting an operation.
        //Shouldn't it cancel also the whole creation?
        if (message->sequenceMessageType() == Uml::SequenceMessage::Destroy) {
            message->setOperationText(i18n("destroy"));
        } else {
            ft->showOperationDialog();
            m_pUMLScene->addWidgetCmd(ft);
        }
        message->setTextPosition();
    }
    UMLApp::app()->executeCommand(new Uml::CmdCreateWidget(message));
    Q_EMIT finished();
}

