 /***************************************************************************
                               unknownumlwidget.cpp
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 

#include "unknownumlwidget.h"
#include "diagram.h"
#include "../umlobject.h"
#include <qpainter.h>
#include <qrect.h>

namespace{
 //vertical and horizontal margins
 int vMargin = 10;
 int hMargin = 10;
}


namespace Umbrello{

UnknownUMLWidget::UnknownUMLWidget( Diagram *diagram, uint id, UMLObject *object):
                 DiagramWidget(diagram, id), m_umlObject(object)
{
	calculateSize();
}

UnknownUMLWidget::~UnknownUMLWidget()
{
	hide();
	diagram()->update();
}
	

void UnknownUMLWidget::umlObjectModified()
{
	calculateSize();
	update();
	diagram()->update();
	
}
	

void UnknownUMLWidget::drawShape(QPainter &p)
{
	QPen textPen(Qt::black);
	QPen drawPen = (Qt::black);
	QBrush drawBrush(Qt::red);

	int currentX,  currentY;
	currentX = (int) x();
	currentY = (int) y();

	p.setPen(drawPen);
	p.setBrush(drawBrush);
	p.drawRect(currentX, currentY, width(), height());
	
	p.setPen(textPen);
	p.drawText(currentX, currentY, width(), height(), Qt::AlignCenter, m_name);
	
	if(isSelected())
	{
		p.setPen(Qt::blue);
		p.setBrush(Qt::blue);
		QRect selectionRect(0,0,4,4);
		selectionRect.moveTopLeft(QPoint((int)x(),(int) y()));
			p.drawRect(selectionRect);
		selectionRect.moveTopRight(QPoint((int) x()+width(),(int) y()));
			p.drawRect(selectionRect);
		selectionRect.moveBottomRight(QPoint((int) x()+width(),(int) y()+height()));
			p.drawRect(selectionRect);
		selectionRect.moveBottomLeft(QPoint((int) x(),(int) y()+height()));
			p.drawRect(selectionRect);
	}
	if( isShowHotSpots( ) )
		drawHotSpots( p );
}

void UnknownUMLWidget::calculateSize()
{
	
	m_name += m_umlObject->getName();

	QFont font; 	
	QFontMetrics fm(font);

	uint width  = fm.width(m_name) + (2 * hMargin);
	uint height = fm.lineSpacing() + (2 * vMargin);
	setSize(width,height);
}

} //end of namespace Umbrello





#include "unknownumlwidget.moc"
