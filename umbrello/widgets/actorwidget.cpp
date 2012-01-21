/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
void ActorWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    UMLWidget::setPenFromSettings(p);
    if( UMLWidget::useFillColor() )
        p.setBrush( UMLWidget::fillColor() );
    const int w = width();
    const int h = height();
    p.setFont( UMLWidget::font() );
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
    p.drawEllipse(offsetX + middleX - a_width / 2, offsetY, a_width, thirdY); //head
    p.drawLine(offsetX + middleX, offsetY + thirdY,
               offsetX + middleX, offsetY + thirdY * 2); //body
    p.drawLine(offsetX + middleX, offsetY + 2 * thirdY,
               offsetX + middleX - a_width / 2, offsetY + a_height); //left leg
    p.drawLine(offsetX + middleX, offsetY +  2 * thirdY,
               offsetX + middleX + a_width / 2, offsetY + a_height); //right leg
    p.drawLine(offsetX + middleX - a_width / 2, offsetY + thirdY + thirdY / 2,
               offsetX + middleX + a_width / 2, offsetY + thirdY + thirdY / 2); //arms
    //draw text
    p.setPen(textColor());
    p.drawText(offsetX + A_MARGIN, offsetY + h - fontHeight,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, name());
    if(m_selected)
        drawSelected(&p, offsetX, offsetY);
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


