/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "nodewidget.h"
#include "node.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>

NodeWidget::NodeWidget(UMLView * view, UMLObject * o) : UMLWidget(view, o) {
	init();
	setSize(100, 30);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
NodeWidget::NodeWidget(UMLView * view) : UMLWidget(view) {
	init();
	setSize(100,30);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::init() {
	UMLWidget::setBaseType(wt_Node);
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView) {
		//check to see if correct
		const SettingsDlg::OptionState& ops = m_pView->getOptionState();
		m_bShowStereotype = ops.classState.showStereoType;
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
	p.setPen( UMLWidget::getLineColour() );
	if ( UMLWidget::getUseFillColour() ) {
		p.setBrush( UMLWidget::getFillColour() );
	} else {
		p.setBrush( m_pView->viewport()->backgroundColor() );
	}

	int w = width();
	int h = height();
	int bodyOffsetY = offsetY + (h/3);
	int bodyWidth = w - (w/3);
	int bodyHeight = h - (h/3);
	QFont font = UMLWidget::getFont();
	font.setBold(true);
	QFontMetrics &fm = getFontMetrics(FT_BOLD);
	int fontHeight  = fm.lineSpacing();
	QString name = getName();

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

	int lines = 1;
	if (m_pObject) {
		QString stereotype = m_pObject->getStereotype();
		if (stereotype != "") {
			p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - fontHeight,
				   bodyWidth, fontHeight, AlignCenter, "«" + stereotype + "»");
			lines = 2;
		}
	}

	if ( UMLWidget::getIsInstance() ) {
		font.setUnderline(true);
		p.setFont(font);
		name = UMLWidget::getInstanceName() + " : " + name;
	}

	if (lines == 1) {
		p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - (fontHeight/2),
			   bodyWidth, fontHeight, AlignCenter, name);
	} else {
		p.drawText(offsetX, bodyOffsetY + (bodyHeight/2),
			   bodyWidth, fontHeight, AlignCenter, name);
	}

	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::calculateSize() {
	if (m_pObject == NULL)
		return;

	QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
	int fontHeight  = fm.lineSpacing();

	QString name = m_pObject->getName();
	if ( UMLWidget::getIsInstance() ) {
		name = UMLWidget::getInstanceName() + " : " + name;
	}

	int width = fm.width(name);

	int tempWidth = 0;
	if(m_pObject->getStereotype() != "") {
		tempWidth = fm.width("«" + m_pObject->getStereotype() + "»");
	}
	width = tempWidth>width ? tempWidth : width;
	width += NODE_MARGIN * 2;

	int height = (2*fontHeight) + (NODE_MARGIN * 2);

	width = static_cast<int>(width * 1.5);
	height = static_cast<int>(height * 1.5);

	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void NodeWidget::setShowStereotype(bool _status) {
	m_bShowStereotype = _status;
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
bool NodeWidget::getShowStereotype() const {
	return m_bShowStereotype;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("nodewidget");
	bool status = UMLWidget::saveToXMI(qDoc, conceptElement);
	conceptElement.setAttribute("showstereotype", m_bShowStereotype);
	qElement.appendChild(conceptElement);
	return status;
}

