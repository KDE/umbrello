/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATEMESSAGES_H
#define TOOLBARSTATEMESSAGES_H

#include "basictypes.h"
#include "toolbarstatepool.h"

class MessageWidget;
class ObjectWidget;
class QGraphicsLineItem;

/**
 * Messages tool to create messages between objects in sequence diagrams.
 * With messages tool, two objects are selected clicking with left button on
 * them and a message of the needed type (depending on the message button
 * selected) is created between the objects. When the first object is selected,
 * a temporary visual message that follows the cursor movement is created until
 * the second object is selected or the message cancelled.
 *
 * A message can be cancelled using right button, which also returns to default
 * tool, or with middle button, which only cancels the message without changing
 * the tool being used.
 *
 * The messages to create can be normal messages or creation messages. Normal
 * messages are created clicking on the line of the two objects. Creation
 * messages are created clicking in the line of the first object, and on the
 * second object itself (not in its line).
 *
 * Associations aren't taken into account, and are treated as empty spaces.
 * Moreover, widgets other than objects aren't neither taken into account.
 *
 * @todo refactor with common code in ToolBarStateAssociation?
 * @todo sequence message lines should be handled by object widgets. Right now,
 * they aren't taken into account in testOnWidget and an explicit check is
 * needed. However, if onWidget in object widgets is changed to also check for
 * the line, a way to give them priority over other widgets in testOnWidget
 * will be needed. For example, when creating a message clicking on an already
 * created message, the message line must be got instead of the message, even if
 * the message is smaller than the line.
 */
class ToolBarStateMessages : public ToolBarStatePool
{
    Q_OBJECT
public:

    explicit ToolBarStateMessages(UMLScene *umlScene);
    virtual ~ToolBarStateMessages();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mouseMove(QGraphicsSceneMouseEvent* ome);

public slots:

    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    virtual void setCurrentElement();

    virtual void mouseReleaseWidget();
    virtual void mouseReleaseEmpty();

protected:

    /**
     * The type of the message to create.
     */
    enum MessageType {
        NormalMessage,
        CreationMessage,
        FoundMessage,
        LostMessage
    };

    void setFirstWidget(ObjectWidget* firstObject);
    void setSecondWidget(ObjectWidget* secondObject, MessageType messageType);

    Uml::SequenceMessage::Enum getMessageType();

    void cleanMessage();

    /**
     * The first object in the message.
     */
    ObjectWidget* m_firstObject;

    /**
     * The message line shown while the first widget is selected and the
     * second one wasn't selected yet.
     */
    QGraphicsLineItem* m_messageLine;

    /**
     * If there is a current widget, it is true if the press event happened on
     * the line of an object, or false if it happened on a normal UMLWidget.
     */
    bool m_isObjectWidgetLine;

private:
    void setupMessageWidget(MessageWidget *msg, bool showOperationDialog = true);

    /**
     * x and y clicked for lost and found messages
     */
    qreal xclick;
    qreal yclick;

};

#endif //TOOLBARSTATEMESSAGES_H
