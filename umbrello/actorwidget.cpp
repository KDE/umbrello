/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "actorwidget.h"

// system includes
#include <qpainter.h>

// local includes
#include "actor.h"
#include "umlview.h"


ActorWidget::ActorWidget(UMLScene * scene, UMLActor *a) : UMLWidget(scene, a)
{
    UMLWidget::setBaseType( Uml::wt_Actor );
}

ActorWidget::~ActorWidget() {}

void ActorWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPenFromSettings(p);
    if( UMLWidget::getUseFillColour() )
        p.setBrush( UMLWidget::getFillColour() );
    const qreal w = getWidth();
    const qreal h = getHeight();
    p.setFont( UMLWidget::getFont() );
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal textWidth = fm.width(getName());
    const qreal fontHeight = fm.lineSpacing();
    const qreal a_height = h - fontHeight - A_MARGIN;
    const qreal h2 = a_height / 2;
    const qreal w2 = w - A_MARGIN * 2;
    const qreal a_width = (h2 > w2 || w > textWidth + A_MARGIN * 2 ?  w2 : h2);
    const qreal middleX = w / 2;
    const qreal thirdY = a_height / 3;

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
    if(isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF ActorWidget::calculateSize() {
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    const int textWidth = fm.width(getName());
    qreal width = textWidth > A_WIDTH ? textWidth : A_WIDTH;
    qreal height = A_HEIGHT + fontHeight + A_MARGIN;
    width += A_MARGIN * 2;
    return QSizeF(width, height);
}

void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement actorElement = qDoc.createElement( "actorwidget" );
    UMLWidget::saveToXMI( qDoc, actorElement );
    qElement.appendChild( actorElement );
}

