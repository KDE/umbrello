/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umlwidgets/childwidgetplacement.h"

#include "umlwidgets/pinportbase.h"

ChildWidgetPlacement::ChildWidgetPlacement(PinPortBase* widget)
 : m_widget(widget), m_connectedSide(Top)
{
}

void ChildWidgetPlacement::setPos(const QPointF& pos)
{
    m_widget->setPos(pos);
}

void ChildWidgetPlacement::setPos(qreal x, qreal y)
{
    m_widget->setPos(x, y);
}

void ChildWidgetPlacement::setX(qreal x)
{
    m_widget->setX(x);
}

void ChildWidgetPlacement::setY(qreal y)
{
    m_widget->setY(y);
}

qreal ChildWidgetPlacement::x() const
{
    return m_widget->x();
}

qreal ChildWidgetPlacement::y() const
{
    return m_widget->y();
}

qreal ChildWidgetPlacement::width() const
{
    return m_widget->width();
}

qreal ChildWidgetPlacement::height() const
{
    return m_widget->height();
}

UMLWidget* ChildWidgetPlacement::ownerWidget() const
{
    return m_widget->ownerWidget();
}

