/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// own header file
#include "usecasewidget.h"
// system includes
#include <qpainter.h>
#include <kdebug.h>
// local includes
#include "usecase.h"
#include "umlview.h"

UseCaseWidget::UseCaseWidget(UMLView * view, UMLUseCase *o) : UMLWidget(view, o) {
	UMLWidget::setBaseType(wt_UseCase);
	//calculateSize();  Doing this during loadFromXMI() gives futile updates.
	//                  Instead, it is done afterwards by UMLWidget::activate()
}

UseCaseWidget::~UseCaseWidget() {}

void UseCaseWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen( QPen( UMLWidget::getLineColour(), UMLWidget::getLineWidth() ) );
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
	p.setPen( QPen( UMLWidget::getLineColour(), UMLWidget::getLineWidth() ) );
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}

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

void UseCaseWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement usecaseElement = qDoc.createElement( "usecasewidget" );
	UMLWidget::saveToXMI( qDoc, usecaseElement );
	qElement.appendChild( usecaseElement );
}

