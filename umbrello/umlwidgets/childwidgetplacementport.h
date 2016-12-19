/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CHILDWIDGETPLACEMENTPORT_H
#define CHILDWIDGETPLACEMENTPORT_H

#include "umlwidgets/childwidgetplacement.h"

/**
 * Implementation of child widget placement where child widget is placed onto parent border.
 *
 * @short Child widget placement on parent border
 */
class ChildWidgetPlacementPort : public ChildWidgetPlacement
{
public:
    ChildWidgetPlacementPort(PinPortBase* widget);
    virtual ~ChildWidgetPlacementPort();

    virtual void setInitialPosition();
    virtual void setNewPositionWhenMoved(qreal diffX, qreal diffY);
    virtual void setNewPositionOnParentResize();

private:
    qreal trimToRange(qreal value, qreal min, qreal max, bool& setToMin, bool& setToMax) const;
    qreal minX() const;
    qreal maxX() const;
    qreal minY() const;
    qreal maxY() const;
};

#endif /* ! CHILDWIDGETPLACEMENTPORT_H */

