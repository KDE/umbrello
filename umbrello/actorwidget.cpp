/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "actorwidget.h"
#include <qpainter.h>
#include "actorwidgetdata.h"
#include "umlview.h"

ActorWidget::ActorWidget(UMLView * view, UMLObject *o, UMLWidgetData* pData) : UMLWidget(view, o,  pData) {}

ActorWidget::ActorWidget(UMLView * view, UMLObject *o) : UMLWidget(view, o, new ActorWidgetData(view->getOptionState() )) {
	m_pData->setType(wt_Actor);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorWidget::ActorWidget(UMLView * view) : UMLWidget(view, new ActorWidgetData(view->getOptionState() )) {
	m_pData->setType(wt_Actor);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorWidget::~ActorWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	int w = width();
	int textStartY = A_HEIGHT + A_MARGIN;
	p.setFont( m_pData -> getFont() );
	QFontMetrics fm = QFontMetrics( m_pData -> getFont() );
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
	QFontMetrics fm = QFontMetrics( m_pData -> getFont() );
	int fontHeight  = fm.lineSpacing();
	int textWidth = fm.width(getName());
	int width = textWidth > A_WIDTH?textWidth:A_WIDTH;
	int height = A_HEIGHT + fontHeight + A_MARGIN;

	width += A_MARGIN * 2;
	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}

/** Synchronizes the Widget's m_pData member with its display properties, for example:
 the X and Y positions of the widget, etc */
void ActorWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}
