/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TOOLBARSTATEOTHER_H
#define TOOLBARSTATEOTHER_H

#include "toolbarstatepool.h"
#include "umlobject.h"

/**
 * Other tool creates almost all the objects (except associations and messages).
 * Objects are created when left button is released, no matter if it was
 * released on an association, on a widget or on an empty space.
 *
 * Associations and widgets aren't taken into account, and are treated as empty
 * spaces.
 */
class ToolBarStateOther : public ToolBarStatePool
{
    Q_OBJECT
public:
    explicit ToolBarStateOther(UMLScene *umlScene);
    virtual ~ToolBarStateOther();

private:
    virtual void setCurrentElement();

    virtual void mouseReleaseEmpty();

    UMLObject::ObjectType getObjectType() const;

    bool newWidget();

};

#endif //TOOLBARSTATEOTHER_H
