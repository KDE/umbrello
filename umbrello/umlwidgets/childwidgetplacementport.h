/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    virtual void detectConnectedSide();
    virtual void setInitialPosition(const QPointF &scenePos = QPointF());
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

