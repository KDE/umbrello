/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
 ***************************************************************************/

// own header file
#include "actorwidget.h"

// local includes
#include "actor.h"
#include "umlview.h"

/**
 * Constructs an ActorWidget.
 *
 * @param scene   The parent of this ActorWidget.
 * @param o      The Actor class this ActorWidget will display.
 */
ActorWidget::ActorWidget(UMLScene * scene, UMLActor *a)
  : UMLWidget(scene, WidgetBase::wt_Actor, a)
{
}

/**
 * Destructor.
 */
ActorWidget::~ActorWidget()
{
}

/**
 * Overrides the standard paint event.
 */
void ActorWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    UMLWidget::setPenFromSettings(painter);
    if( UMLWidget::useFillColor() )
        painter->setBrush( UMLWidget::fillColor() );
    const int w = width();
    const int h = height();
    painter->setFont( UMLWidget::font() );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int textWidth = fm.width(name());
    const int fontHeight = fm.lineSpacing();
    const int a_height = h - fontHeight - A_MARGIN;
    const int h2 = a_height / 2;
    const int w2 = w - A_MARGIN * 2;
    const int a_width = (h2 > w2 || w > textWidth + A_MARGIN * 2 ?  w2 : h2);
    const int middleX = w / 2;
    const int thirdY = a_height / 3;

    //draw actor
    painter->drawEllipse(middleX - a_width / 2, 0, a_width, thirdY); //head
    painter->drawLine(middleX, thirdY,
               middleX, thirdY * 2); //body
    painter->drawLine(middleX, 2 * thirdY,
               middleX - a_width / 2, a_height); //left leg
    painter->drawLine(middleX,  2 * thirdY,
               middleX + a_width / 2, a_height); //right leg
    painter->drawLine(middleX - a_width / 2, thirdY + thirdY / 2,
               middleX + a_width / 2, thirdY + thirdY / 2); //arms
    //draw text
    painter->setPen(textColor());
    painter->drawText(A_MARGIN, h - fontHeight,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
    if(m_selected)
        paintSelected(painter);
}

/**
 * Saves the widget to the "actorwidget" XMI element.
 * Note: For loading from XMI, the inherited parent method is used.
 */
void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement actorElement = qDoc.createElement( "actorwidget" );
    UMLWidget::saveToXMI( qDoc, actorElement );
    qElement.appendChild( actorElement );
}

/**
 * Overrides method from UMLWidget.
 */
UMLSceneSize ActorWidget::minimumSize()
{
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(name());
    int width = textWidth > A_WIDTH ? textWidth : A_WIDTH;
    int height = A_HEIGHT + fontHeight + A_MARGIN;
    width += A_MARGIN * 2;
    return UMLSceneSize(width, height);
}


