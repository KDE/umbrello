/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "nodewidget.h"
#include "nodewidgetdata.h"
#include "node.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>

NodeWidget::NodeWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData) : UMLWidget(view, o, pData) {
	m_pMenu = 0;
	if (m_pObject) {
		calculateSize();
		update();
	}
}

NodeWidget::NodeWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o, new NodeWidgetData(view->getOptionState() )) {
	init();
	setSize(100, 30);
	calculateSize();
	m_pData->setType(wt_Node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeWidget::NodeWidget(UMLView * view) : UMLWidget(view, new NodeWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	m_pData->setType(wt_Node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView && m_pData) {
		//check to see if correct
		SettingsDlg::OptionState ops = m_pView->getOptionState();

		( (NodeWidgetData*)m_pData ) -> setShowStereotype( ops.classState.showStereoType );
	}
	//maybe loading and this may not be set.
	if (m_pObject) {
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeWidget::~NodeWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen( m_pData->getLineColour() );
	if ( m_pData->getUseFillColor() ) {
		p.setBrush( m_pData->getFillColour() );
	} else {
		p.setBrush( m_pView->viewport()->backgroundColor() );
	}

	int w = width();
	int h = height();
	int bodyOffsetY = offsetY + (h/3);
	int bodyWidth = w - (w/3);
	int bodyHeight = h - (h/3);
	QFont font = m_pData->getFont();
	font.setBold(true);
	QFontMetrics &fm = getFontMetrics(FT_BOLD);
	int fontHeight  = fm.lineSpacing();
	QString name = getName();
	QString stereotype = m_pObject->getStereotype();

	QPointArray pointArray(6);
	pointArray.setPoint(0, offsetX, bodyOffsetY);
	pointArray.setPoint(1, offsetX + (w/3), offsetY);
	pointArray.setPoint(2, offsetX + w - 1, offsetY);
	pointArray.setPoint(3, offsetX + w - 1, offsetY + ((h/3)*2) );
	pointArray.setPoint(4, offsetX + bodyWidth, offsetY + h - 1);
	pointArray.setPoint(5, offsetX, offsetY + h - 1);
	p.drawPolygon(pointArray);
	p.drawRect(offsetX, bodyOffsetY, bodyWidth, bodyHeight);
	p.drawLine(offsetX + w - 1, offsetY, offsetX + bodyWidth - 2, bodyOffsetY + 1);

	p.setPen( QPen(black) );
	p.setFont(font);

	if (stereotype != "") {
		p.drawText(offsetX, (int)(bodyOffsetY + (bodyHeight*0.5) - fontHeight),
			   bodyWidth, fontHeight, AlignCenter, "<< " + stereotype + " >>");
	}

	int lines;
	if (stereotype != "") {
		lines = 2;
	} else {
		lines = 1;
	}

	if ( getData()->getIsInstance() ) {
		font.setUnderline(true);
		p.setFont(font);
		name = getData()->getInstanceName() + " : " + name;
	}

	if (lines == 1) {
		p.drawText(offsetX, (int)(bodyOffsetY + (bodyHeight*0.5) - (fontHeight*0.5)),
			   bodyWidth, fontHeight, AlignCenter, name);
	} else {
		p.drawText(offsetX, (int)(bodyOffsetY + (bodyHeight*0.5)),
			   bodyWidth, fontHeight, AlignCenter, name);
	}

	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::calculateSize() {
	int width, height;

	QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
	int fontHeight  = fm.lineSpacing();

	QString name = m_pObject->getName();
	if ( getData()->getIsInstance() ) {
		name = getData()->getInstanceName() + " : " + name;
	}

	width = fm.width(name);

	int tempWidth = 0;
	if(m_pObject->getStereotype() != "") {
		tempWidth = fm.width("<< " + m_pObject->getStereotype() + " >>");
	}
	width = tempWidth>width ? tempWidth : width;
	width += NODE_MARGIN * 2;

	height = (2*fontHeight) + (NODE_MARGIN * 2);

	width = static_cast<int>(width * 1.5);
	height = static_cast<int>(height * 1.5);

	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::setShowStereotype(bool _status) {
	((NodeWidgetData*)m_pData)->setShowStereotype( _status );
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if(status) {
		calculateSize();
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeWidget::getShowStereotype() {
	return ((NodeWidgetData*)m_pData)->getShowStereotype();
}
