/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "actorwidget.h"

// system includes
#include <qpainter.h>

// local includes
#include "actor.h"
#include "umlview.h"


ActorWidget::ActorWidget(UMLView * view, UMLActor *a) : UMLWidget(view, a) {
    UMLWidget::setBaseType( Uml::wt_Actor );
}

ActorWidget::~ActorWidget() {}

void ActorWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if( UMLWidget::getUseFillColour() )
        p.setBrush( UMLWidget::getFillColour() );
    const int w = width();
    const int h = height();
    p.setFont( UMLWidget::getFont() );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int textWidth = fm.width(getName());
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
    p.setPen(QPen(Qt::black));
    p.drawText(offsetX + A_MARGIN, offsetY + h - fontHeight,
               w - A_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}

QSize ActorWidget::calculateSize() {
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(getName());
    int width = textWidth > A_WIDTH ? textWidth : A_WIDTH;
    int height = A_HEIGHT + fontHeight + A_MARGIN;
    width += A_MARGIN * 2;
    return QSize(width, height);
}

void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement actorElement = qDoc.createElement( "actorwidget" );
    UMLWidget::saveToXMI( qDoc, actorElement );
    qElement.appendChild( actorElement );
}

