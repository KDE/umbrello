/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CHILDWIDGETPLACEMENTPIN_H
#define CHILDWIDGETPLACEMENTPIN_H

#include "umlwidgets/childwidgetplacement.h"

/**
 * Implementation of child widget placement where parent and child widget borders are
 * always connected.
 *
 * @short Child widget placement attached to parent
 */
class ChildWidgetPlacementPin : public ChildWidgetPlacement
{
public:
    ChildWidgetPlacementPin(PinPortBase* widget);
    virtual ~ChildWidgetPlacementPin();

    virtual void detectConnectedSide();
    virtual void setInitialPosition(const QPointF &scenePos = QPointF());
    virtual void setNewPositionWhenMoved(qreal diffX, qreal diffY);
    virtual void setNewPositionOnParentResize();

private:
    bool isAboveParent() const;
    bool isBelowParent() const;
    bool isLeftOfParent() const;
    bool isRightOfParent() const;
    qreal getNewXOnJumpToTopOrBottom() const;
    void jumpToTopOfParent();
    void jumpToBottomOfParent();
    qreal getNewYOnJumpToSide() const;
    void jumpToLeftOfParent();
    void jumpToRightOfParent();
};

#endif /* ! CHILDWIDGETPLACEMENTPIN_H */


