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

/** Returns the amount of bytes needed to serialize this object */
/* If the serialization method of this class is changed this function will have to be CHANGED TOO*/

/*This function is used by the Copy and Paste Functionality*/
/*The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{
			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior
*/
long ActorWidget::getClipSizeOf() {
	return UMLWidget::getClipSizeOf();
}

/** Synchronizes the Widget's m_pData member with its display properties, for example:
 the X and Y positions of the widget, etc */
void ActorWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}
