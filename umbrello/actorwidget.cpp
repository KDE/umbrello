/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
	//calculateSize();  Doing this during loadFromXMI() gives futile updates.
	//                  Instead, it is done afterwards by UMLWidget::activate()
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorWidget::~ActorWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorWidget::draw(QPainter & p, int offsetX, int offsetY) {
	UMLWidget::draw(p, offsetX, offsetY);
	if( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	int w = width();
	int textStartY = A_HEIGHT + A_MARGIN;
	p.setFont( UMLWidget::getFont() );
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();

	int middleX = w / 2;
	int thirdY = A_HEIGHT / 3;

	//draw actor
	p.drawEllipse(offsetX + middleX - A_WIDTH / 2, offsetY,  A_WIDTH, thirdY);//head
	p.drawLine(offsetX + middleX, offsetY + thirdY, offsetX + middleX, offsetY + thirdY * 2);//body
	p.drawLine(offsetX + middleX, offsetY + 2 * thirdY, offsetX + middleX - A_WIDTH / 2, offsetY + A_HEIGHT);//left leg
	p.drawLine(offsetX + middleX, offsetY +  2 * thirdY, offsetX + middleX + A_WIDTH / 2, offsetY + A_HEIGHT);//right leg
	p.drawLine(offsetX + middleX - A_WIDTH / 2, offsetY + thirdY + thirdY / 2, offsetX + middleX + A_WIDTH / 2, offsetY + thirdY + thirdY / 2);//arms
	//draw text
	p.setPen(QPen(black));
	p.drawText(offsetX + A_MARGIN, offsetY + textStartY, w - A_MARGIN * 2, fontHeight, AlignCenter, getName());
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorWidget::calculateSize() {
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	int textWidth = fm.width(getName());
	int width = textWidth > A_WIDTH?textWidth:A_WIDTH;
	int height = A_HEIGHT + fontHeight + A_MARGIN;

	width += A_MARGIN * 2;
	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement actorElement = qDoc.createElement( "actorwidget" );
	UMLWidget::saveToXMI( qDoc, actorElement );
	qElement.appendChild( actorElement );
}

