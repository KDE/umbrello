 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "interfacewidget.h"
#include "interface.h"
#include "operation.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"

#include <kdebug.h>
#include <qpainter.h>

#define CIRCLE_SIZE 30

InterfaceWidget::InterfaceWidget(UMLView * view, UMLInterface *i)
  : ClassifierWidget(view, i, Uml::wt_Interface) {
	init();
	setSize(100,30);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InterfaceWidget::~InterfaceWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::init() {
	m_bDrawAsCircle = false;

	updateSigs();
	initUMLObject( m_pObject );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::initUMLObject(UMLObject* object)
{
	object -> setAbstract( true ); // interfaces are by definition abstract
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setUMLObject(UMLObject* object)
{
	initUMLObject(object);
	UMLWidget::setUMLObject(object);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::draw(QPainter& p, int offsetX, int offsetY) {
	if ( getDrawAsCircle() ) {
		UMLWidget::draw(p, offsetX, offsetY);
		if (UMLWidget::getUseFillColour())
			p.setBrush(UMLWidget::getFillColour());
		else
			p.setBrush(m_pView -> viewport() -> backgroundColor());
		drawAsCircle(p, offsetX, offsetY);
	} else {
		drawAsConcept(p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::drawAsCircle(QPainter& p, int offsetX, int offsetY) {
	int w = width();

	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject -> getPackage() + "." + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
	p.setPen( QPen(black) );

	QFont font = UMLWidget::getFont();
	p.setFont(font);
	p.drawText(offsetX, offsetY + CIRCLE_SIZE,
		   w, fontHeight, AlignCenter, name);

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::drawAsConcept(QPainter& p, int offsetX, int offsetY) {
	ClassifierWidget::draw(p, offsetX, offsetY);

	QFont font = UMLWidget::getFont();
	font.setItalic(false);
	font.setUnderline(false);
	font.setBold(false);
	QFontMetrics fontMetrics(font);
	int fontHeight = fontMetrics.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject -> getPackage() + "." + this -> getName();
	} else {
		name = this -> getName();
	}

	p.setPen(QPen(black));

	font.setBold(true);
	p.setFont(font);
	p.drawText(offsetX + ClassifierWidget::MARGIN, m_bodyOffsetY,
		   m_w - ClassifierWidget::MARGIN * 2,fontHeight,
		   AlignCenter, m_pObject->getStereotype());

	font.setItalic( m_pObject -> getAbstract() );
	//FIXME why is underline sometimes true
	font.setUnderline( false );
	p.setFont(font);
	p.drawText(offsetX + ClassifierWidget::MARGIN, m_bodyOffsetY + fontHeight,
		   m_w - ClassifierWidget::MARGIN * 2, fontHeight, AlignCenter, name);
	font.setBold(false);
	font.setItalic(false);
	p.setFont(font);

	if ( m_bShowOperations ) {
		QFont font = UMLWidget::getFont();
		font.setItalic(false);
		font.setUnderline(false);
		font.setBold(false);

		UMLWidget::draw(p, offsetX, m_bodyOffsetY);

		const int operationsStart = fontHeight * 2;
		const int y = m_bodyOffsetY + operationsStart;
		p.drawLine(offsetX, y, offsetX + m_w - 1, y);
		p.setPen( QPen(black) );
		drawMembers(p, Uml::ot_Operation, m_ShowOpSigs,
			    offsetX + ClassifierWidget::MARGIN, y, fontHeight);
	}//end if op

	if (m_bSelected) {
		drawSelected(&p, offsetX, m_bodyOffsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::calculateSize() {
	if ( getDrawAsCircle() ) {
		calculateAsCircleSize();
	} else {
		calculateAsConceptSize();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::calculateAsCircleSize() {
	QFontMetrics &fm = getFontMetrics(FT_ITALIC_UNDERLINE);
	int fontHeight = fm.lineSpacing();

	int height = CIRCLE_SIZE + fontHeight;

	int width;
	if ( m_bShowPackage ) {
		width = fm.width(m_pObject->getPackage() + "." + getName());
	} else {
		width = fm.width(getName());
	}
	width = width<CIRCLE_SIZE ? CIRCLE_SIZE : width;

	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::calculateAsConceptSize() {
	if( !m_pObject)
		return;
	int width = 0, height = 0;
	ClassifierWidget::computeBasicSize(width, height);
	QSize templatesBoxSize = calculateTemplatesBoxSize();
	if (templatesBoxSize.width() != 0) {
		width += templatesBoxSize.width() / 2;
	}
	if (templatesBoxSize.height() != 0) {
		height += templatesBoxSize.height() - ClassifierWidget::MARGIN;
	}
	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::slotMenuSelection(int sel) {
	switch(sel) {
		case ListPopupMenu::mt_Operation:
			UMLApp::app()->getDocument()->createChildObject(m_pObject, Uml::ot_Operation);
			break;

		case ListPopupMenu::mt_Show_Operations:
			toggleShowOps();
			break;

		case ListPopupMenu::mt_Show_Operation_Signature:
			toggleShowOpSigs();
			break;

		case ListPopupMenu::mt_Scope:
			toggleShowScope();
			break;

		case ListPopupMenu::mt_DrawAsCircle:
			toggleDrawAsCircle();
			break;

		case ListPopupMenu::mt_Show_Packages:
			toggleShowPackage();
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::updateSigs() {
	ClassifierWidget::updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setDrawAsCircle(bool drawAsCircle) {
	m_bDrawAsCircle = drawAsCircle;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceWidget::getDrawAsCircle() const {
	return m_bDrawAsCircle;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement("interfacewidget");
	UMLWidget::saveToXMI(qDoc, conceptElement);

	conceptElement.setAttribute("showoperations", m_bShowOperations);
	conceptElement.setAttribute("showopsigs", m_ShowOpSigs);
	conceptElement.setAttribute("showpackage", m_bShowPackage);
	conceptElement.setAttribute("showscope", m_bShowScope);
	conceptElement.setAttribute("drawascircle", m_bDrawAsCircle);
	qElement.appendChild(conceptElement);
}

bool InterfaceWidget::loadFromXMI( QDomElement & qElement ) {
	if ( !UMLWidget::loadFromXMI(qElement) ) {
		return false;
	}
	QString showops = qElement.attribute("showoperations", "1");
	QString showopsigs = qElement.attribute("showopsigs", "600");
	QString showpackage = qElement.attribute("showpackage", "0");
	QString showscope = qElement.attribute("showscope", "0");
	QString drawascircle = qElement.attribute("drawascircle", "0");

	m_bShowOperations = (bool)showops.toInt();
	m_ShowOpSigs = (Uml::Signature_Type)showopsigs.toInt();
	m_bShowPackage = (bool)showpackage.toInt();
	m_bShowScope = (bool)showscope.toInt();
	m_bDrawAsCircle = (bool)drawascircle.toInt();

	return true;
}

void InterfaceWidget::toggleDrawAsCircle()
{
	m_bDrawAsCircle = !m_bDrawAsCircle;
	updateSigs();
	calculateSize();
	update();
}

