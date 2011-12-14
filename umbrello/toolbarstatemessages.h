/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEMESSAGES_H
#define TOOLBARSTATEMESSAGES_H

#include "basictypes.h"
#include "toolbarstatepool.h"

class ObjectWidget;

/**
 * Messages tool to create messages between objects in sequence diagrams.
 * With messages tool, two objects are selected clicking with left button on
 * them and a message of the needed type (depending on the message button
 * selected) is created between the objects. When the first object is selected,
 * a temporal visual message that follows the cursor movement is created until
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
 * the line, a way to make them prioritaries over other widgets in testOnWidget
 * will be needed. For example, when creating a message clicking on an already
 * created message,the message line must be got instead of the message, even if
 * the message is smaller than the line.
 */
class ToolBarStateMessages : public ToolBarStatePool
{
    Q_OBJECT
public:

    ToolBarStateMessages(UMLScene *umlScene);
    virtual ~ToolBarStateMessages();

    virtual void init();

    virtual void cleanBeforeChange();

    virtual void mouseMove(UMLSceneMouseEvent* ome);

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

    Uml::Sequence_Message_Type getMessageType();

    void cleanMessage();

    /**
     * The first object in the message.
     */
    ObjectWidget* m_firstObject;

    /**
     * The message line shown while the first widget is selected and the
     * second one wasn't selected yet.
     */
    UMLSceneLine* m_messageLine;

    /**
     * If there is a current widget, it is true if the press event happened on
     * the line of an object, or false if it happened on a normal UMLWidget.
     */
    bool m_isObjectWidgetLine;

private:

    /**
     * x and y clicked for lost and found messages
     */
    int xclick;
    int yclick;

};

#endif //TOOLBARSTATEMESSAGES_H
