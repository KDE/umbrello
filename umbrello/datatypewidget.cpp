/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "datatypewidget.h"
#include "datatype.h"
#include "operation.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "listpopupmenu.h"

#include <kdebug.h>
#include <qpainter.h>

#define CIRCLE_SIZE 30

DatatypeWidget::DatatypeWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o) {
	init();
	setSize(100,30);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DatatypeWidget::DatatypeWidget(UMLView* view) : UMLWidget(view) {
	init();
	setSize(100,30);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DatatypeWidget::init() {
	UMLWidget::setBaseType(wt_Datatype);
	m_pMenu = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DatatypeWidget::~DatatypeWidget() {}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DatatypeWidget::draw(QPainter& p, int offsetX, int offsetY) {
	p.setPen(UMLWidget::getLineColour());
	if (UMLWidget::getUseFillColour())  {
		p.setBrush(UMLWidget::getFillColour());
	} else {
		p.setBrush(m_pView->viewport()->backgroundColor());
	}

	int w = width();
	int h = height();

	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name = this->getName();

	p.drawRect(offsetX, offsetY, w, h);
	p.setPen(QPen(black));

	QFont font = UMLWidget::getFont();
	font.setBold(true);
	p.setFont(font);
	p.drawText(offsetX + DATATYPE_MARGIN, offsetY,
		   w - DATATYPE_MARGIN* 2,fontHeight,
		   AlignCenter, "<< " + m_pObject->getStereotype() + " >>");

	font.setItalic( m_pObject->getAbstract() );
	p.setFont(font);
	p.drawText(offsetX + DATATYPE_MARGIN, offsetY + fontHeight,
		   w - DATATYPE_MARGIN * 2, fontHeight, AlignCenter, name);

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void DatatypeWidget::calculateSize() {
	if (!m_pObject)  {
		return;
	}
	int width, height;
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	lines++; //for the stereotype

	height = width = 0;
	height += lines * fontHeight;

	//now set the width of the concept
	//set width to name to start with
	//set width to name to start with
	width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->getPackage() + "::" + getName()).width();
	int w = getFontMetrics(FT_BOLD).boundingRect("<< " + m_pObject->getStereotype() + " >>").width();

	width = w > width?w:width;

	//allow for width margin
	width += DATATYPE_MARGIN * 2;

	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool DatatypeWidget::activate(IDChangeLog* ChangeLog /* = 0*/) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool DatatypeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement("datatypewidget");
	bool status = UMLWidget::saveToXMI(qDoc, conceptElement);
	qElement.appendChild(conceptElement);
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool DatatypeWidget::loadFromXMI( QDomElement & qElement ) {
	return UMLWidget::loadFromXMI(qElement);
}
