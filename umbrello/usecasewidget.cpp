/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usecasewidget.h"
#include "umlwidget.h"
#include "umlview.h"

#include <qpainter.h>

UseCaseWidget::UseCaseWidget(UMLView * view, UMLObject *o) : UMLWidget(view, o) {
	UMLWidget::setBaseType(wt_UseCase);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UseCaseWidget::UseCaseWidget(UMLView * view) : UMLWidget(view) {
	UMLWidget::setBaseType(wt_UseCase);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UseCaseWidget::~UseCaseWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UseCaseWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen( UMLWidget::getLineColour() );
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	p.setFont( UMLWidget::getFont() );
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	int w = width();
	int h = height();
	//int middleX = w / 2;
	int textStartY = (h / 2) - (fontHeight / 2);

	p.drawEllipse(offsetX, offsetY, w, h);
	p.setPen(black);
	p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, w - UC_MARGIN * 2, fontHeight, AlignCenter, getName());
	p.setPen( UMLWidget::getLineColour() );
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);




}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UseCaseWidget::calculateSize()
{
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	int textWidth = fm.width(getName());
	int width = textWidth > UC_WIDTH?textWidth:UC_WIDTH;
	int height = UC_HEIGHT + fontHeight + UC_MARGIN;

	width += UC_MARGIN * 2;
	setSize(width, height);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement usecaseElement = qDoc.createElement( "UML:UseCaseWidget" );
	bool status = UMLWidget::saveToXMI( qDoc, usecaseElement );
	qElement.appendChild( usecaseElement );
	return status;
}

