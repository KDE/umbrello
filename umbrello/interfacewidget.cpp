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
#include "listpopupmenu.h"

#include <kdebug.h>
#include <qpainter.h>

#define CIRCLE_SIZE 30

InterfaceWidget::InterfaceWidget(UMLView * view, UMLInterface *i) : UMLWidget(view, i) {
	init();
	setSize(100,30);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InterfaceWidget::~InterfaceWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::init() {
	UMLWidget::setBaseType(wt_Interface);
	m_pMenu = 0;
	m_bDrawAsCircle = false;

	const SettingsDlg::OptionState& ops = m_pView->getOptionState();
	m_bShowScope = ops.classState.showScope;
	setShowOpSigs( ops.classState.showOpSig );
	m_bShowOperations = ops.classState.showOps;
	m_bShowPackage = ops.classState.showPackage;

	updateSigs();
	initUMLObject( m_pObject );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

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
		drawAsCircle(p, offsetX, offsetY);
	} else {
		drawAsConcept(p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::drawAsCircle(QPainter& p, int offsetX, int offsetY) {
	p.setPen( UMLWidget::getLineColour() );

	if ( UMLWidget::getUseFillColour() ) {
		p.setBrush( UMLWidget::getFillColour() );
	} else {
		p.setBrush( m_pView->viewport()->backgroundColor() );
	}

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
	p.setPen(UMLWidget::getLineColour());
	if(UMLWidget::getUseFillColour())
		p.setBrush(UMLWidget::getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	int w = width();
	int h = height();

	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, offsetY, w, h);
	p.setPen(QPen(black));

	QFont font = UMLWidget::getFont();
	font.setBold(true);
	p.setFont(font);
	p.drawText(offsetX + INTERFACE_MARGIN, offsetY,
		   w - INTERFACE_MARGIN * 2,fontHeight,
		   AlignCenter, "«" + m_pObject -> getStereotype() + "»");

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
		QFont font( p.font() );
		y = operationsStart;
		p.setPen( UMLWidget::getLineColour() );

		p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

		UMLClassifierListItem* obj = 0;
		QPtrList<UMLClassifierListItem>* list = ((UMLInterface*)m_pObject)->getOpList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString op = obj->toString( m_ShowOpSigs );
			p.setPen( QPen(black) );
			font.setUnderline( obj->getStatic() );
			font.setItalic( obj->getAbstract() );
			p.setFont(font);
			p.drawText(offsetX + INTERFACE_MARGIN, offsetY + y,
				   w - INTERFACE_MARGIN * 2, fontHeight, AlignVCenter, op);
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
	int numOps = ((UMLInterface*)m_pObject)->operations();

	lines++; //for the stereotype

	height = width = 0;
	//set the height of the concept
	if (m_bShowOperations) {
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
	int w = getFontMetrics(FT_BOLD).boundingRect("«" + m_pObject->getStereotype() + "»").width();


	width = w > width?w:width;
	width++;

	if (m_bShowOperations) {
		QPtrList<UMLClassifierListItem>* list = ((UMLInterface*)m_pObject)->getOpList();
		UMLClassifierListItem* listItem = 0;
		for(listItem = list->first();listItem != 0; listItem = list->next()) {
			/* we don't make a difference if the text is underlined or not, because
			 * if we do so, we will get the wrong width;
			 */
			fm = getFontMetrics(FT_ITALIC_UNDERLINE);

			/* it should be the following thing, but then the width will be too
			 * small for FT_NORMAL text (only some pixels) */
//			bool isAbstract = listItem->getAbstract();
//			bool isStatic = listItem->getStatic();
//			if (isAbstract && isStatic)
//			{
//				fm = getFontMetrics(FT_ITALIC_UNDERLINE);
//			} else if (isAbstract) {
//				fm = getFontMetrics(FT_ITALIC);
//			} else if (isStatic) {
//				fm = getFontMetrics(FT_UNDERLINE);
//			} else {
//				fm = getFontMetrics(FT_NORMAL);
//			}
			int w = fm.boundingRect( listItem->toString(m_ShowOpSigs) ).width();
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
			m_pView->getDocument()->createUMLObject(m_pObject, ListPopupMenu::convert_MT_OT( (ListPopupMenu::Menu_Type)sel) );
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
	//turn on scope
	if (m_bShowScope) {
		if (m_ShowOpSigs == Uml::st_NoSigNoScope) {
			m_ShowOpSigs = Uml::st_NoSig;
		} else if (m_ShowOpSigs == Uml::st_SigNoScope) {
			m_ShowOpSigs = Uml::st_ShowSig;
		}
	} else { //turn off scope
		if (m_ShowOpSigs == Uml::st_ShowSig) {
			m_ShowOpSigs = Uml::st_SigNoScope;
		} else if (m_ShowOpSigs == Uml::st_NoSig) {
			m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	}
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowScope(bool _scope) {
	m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowOps(bool _show) {
	m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setOpSignature(Signature_Type sig) {
	m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowPackage(bool _status) {
	m_bShowPackage = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceWidget::getShowPackage() const {
	return m_bShowPackage;
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
bool InterfaceWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowOpSigs(bool _status) {
	if( !_status ) {
		if (m_bShowScope)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if (m_bShowScope)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;
	calculateSize();
	update();
}

bool InterfaceWidget::getShowOps() const {
	return m_bShowOperations;
}

Uml::Signature_Type InterfaceWidget::getShowOpSigs() const {
	return m_ShowOpSigs;
}

bool InterfaceWidget::getShowScope() const {
	return m_bShowScope;
}

bool InterfaceWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement("interfacewidget");
	bool status = UMLWidget::saveToXMI(qDoc, conceptElement);

	conceptElement.setAttribute("showoperations", m_bShowOperations);
	conceptElement.setAttribute("showopsigs", m_ShowOpSigs);
	conceptElement.setAttribute("showpackage", m_bShowPackage);
	conceptElement.setAttribute("showscope", m_bShowScope);
	conceptElement.setAttribute("drawascircle", m_bDrawAsCircle);
	qElement.appendChild(conceptElement);

	return status;
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

void InterfaceWidget::toggleShowOps()
{
	m_bShowOperations = !m_bShowOperations;
	updateSigs();
	calculateSize();

	update();

	return;
}

void InterfaceWidget::toggleShowScope()
{
	m_bShowScope = !m_bShowScope;
	updateSigs();
	calculateSize();
	update();

	return;
}

void InterfaceWidget::toggleShowOpSigs()
{
	if (m_ShowOpSigs == Uml::st_ShowSig || m_ShowOpSigs == Uml::st_SigNoScope) {
		if (m_bShowScope) {
			m_ShowOpSigs = Uml::st_NoSig;
		} else {
			m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	} else if (m_bShowScope) {
		m_ShowOpSigs = Uml::st_ShowSig;
	} else {
		m_ShowOpSigs = Uml::st_SigNoScope;
	}

	calculateSize();
	update();

	return;
}

void InterfaceWidget::toggleDrawAsCircle()
{
	m_bDrawAsCircle = !m_bDrawAsCircle;
	updateSigs();
	calculateSize();
	update();

	return;
}

void InterfaceWidget::toggleShowPackage()
{
	m_bShowPackage = !m_bShowPackage;
	updateSigs();
	calculateSize();
	update();

	return;
}
