/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usecasewidget.h"
#include "usecasewidgetdata.h"
#include "umlwidget.h"
#include "umlview.h"

#include <qpainter.h>

UseCaseWidget::UseCaseWidget(UMLView * view, UMLObject *o, UMLWidgetData* pData) : UMLWidget(view, o, pData) {}

UseCaseWidget::UseCaseWidget(UMLView * view, UMLObject *o) : UMLWidget(view, o, new UseCaseWidgetData(view->getOptionState() )) {
	m_pData->setType(wt_UseCase);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UseCaseWidget::UseCaseWidget(UMLView * view) : UMLWidget(view, new UseCaseWidgetData(view->getOptionState())) {
	m_pData->setType(wt_UseCase);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UseCaseWidget::~UseCaseWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UseCaseWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	p.setFont( m_pData -> getFont() );
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	int w = width();
	int h = height();
	//int middleX = w / 2;
	int textStartY = (h / 2) - (fontHeight / 2);

	p.drawEllipse(offsetX, offsetY, w, h);
	p.setPen(black);
	p.drawText(offsetX + UC_MARGIN, offsetY + textStartY, w - UC_MARGIN * 2, fontHeight, AlignCenter, getName());
	p.setPen(m_pData->getLineColour());
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

void UseCaseWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}
