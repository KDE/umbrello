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
	UMLWidget::draw(p, offsetX, offsetY);
	if (UMLWidget::getUseFillColour())
		p.setBrush(UMLWidget::getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	if ( getDrawAsCircle() ) {
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
		name = m_pObject -> getPackage() + "::" + this -> getName();
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
	int w = width();
	int h = height();

	QFont font = UMLWidget::getFont();
	font.setItalic(false);
	font.setUnderline(false);
	font.setBold(false);
	QFontMetrics fontMetrics(font);
	int fontHeight = fontMetrics.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, offsetY, w, h);
	p.setPen(QPen(black));

	font.setBold(true);
	p.setFont(font);
	p.drawText(offsetX + INTERFACE_MARGIN, offsetY,
		   w - INTERFACE_MARGIN * 2,fontHeight,
		   AlignCenter, m_pObject->getStereotype());

	font.setItalic( m_pObject -> getAbstract() );
	//FIXME why is underline sometimes true
	font.setUnderline( false );
	p.setFont(font);
	p.drawText(offsetX + INTERFACE_MARGIN, offsetY + fontHeight,
		   w - INTERFACE_MARGIN * 2, fontHeight, AlignCenter, name);
	font.setBold(false);
	font.setItalic(false);
	p.setFont(font);


	int operationsStart = fontHeight * 2;
	int y;

	if ( m_bShowOperations ) {
		QFont font = UMLWidget::getFont();
		font.setItalic(false);
		font.setUnderline(false);
		font.setBold(false);

		y = operationsStart;
		UMLWidget::draw(p, offsetX, offsetY);

		p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

		UMLOperation* obj = 0;
		UMLOperationList list(((UMLInterface*)m_pObject)->getOpList());
		for(obj=list.first();obj != 0;obj=list.next()) {
			if (m_bShowPublicOnly && obj->getScope() != Uml::Public)
				continue;
			QString op = obj->toString( m_ShowOpSigs );
			p.setPen( QPen(black) );
			font.setUnderline( obj->getStatic() );
			font.setItalic( obj->getAbstract() );
			p.setFont(font);
			QFontMetrics fontMetrics(font);
			p.drawText(offsetX + INTERFACE_MARGIN, offsetY + y,
				   fontMetrics.width(op), fontHeight, AlignVCenter, op);
			font.setUnderline(false);
			font.setItalic(false);
			p.setFont(font);
			y+=fontHeight;
		}
	}//end if op

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
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
		width = fm.width(m_pObject->getPackage() + "::" + getName());
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
	int width, height;
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	int numOps = 0;

	lines++; //for the stereotype

	height = width = 0;
	//set the height of the concept
	if (m_bShowOperations) {
		UMLOperationList list((static_cast<UMLClassifier*>(m_pObject))->getOpList());
		for (UMLOperation *obj = list.first(); obj; obj = list.next()) {
			if (!(m_bShowPublicOnly && obj->getScope() != Uml::Public))
				numOps++;
		}
		lines += numOps;
		if(numOps == 0) {
			height += fontHeight / 2;//no ops, so just add a but of space
		}
	}
	height += lines * fontHeight;
	//now set the width of the concept
	//set width to name to start with
	if(m_bShowPackage)
		width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->getPackage() + "::" + getName()).width();
	else
		width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(getName()).width();
	int w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->getStereotype()).width();


	width = w > width?w:width;
	width++;

	if (m_bShowOperations) {
		UMLOperationList list(((UMLInterface*)m_pObject)->getOpList());
		UMLOperation* listItem = 0;
		for(listItem = list.first();listItem != 0; listItem = list.next()) {
			if (m_bShowPublicOnly && listItem->getScope() != Uml::Public)
				continue;
			QFont font = UMLWidget::getFont();
			font.setUnderline( listItem->getStatic() );
			font.setItalic( listItem->getAbstract() );
			QFontMetrics fontMetrics(font);

			int w = fontMetrics.width(listItem->toString(m_ShowOpSigs));
			width = w > width?w:width;
		}
	}
	//allow for width margin
	width += INTERFACE_MARGIN * 2;

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

