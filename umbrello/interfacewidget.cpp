/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "interfacewidget.h"
#include "interfacewidgetdata.h"
#include "interface.h"
#include "operation.h"
#include "umlview.h"
#include "listpopupmenu.h"

#include <kdebug.h>
#include <qpainter.h>

#define CIRCLE_SIZE 30

InterfaceWidget::InterfaceWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData) : UMLWidget(view, o, pData) {
	m_pMenu = 0;
	if( m_pObject ) {
		initUMLObject(m_pObject);
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InterfaceWidget::InterfaceWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o, new InterfaceWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	calculateSize();
	m_pData->setType(wt_Interface);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InterfaceWidget::InterfaceWidget(UMLView* view) : UMLWidget(view, new InterfaceWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	m_pData->setType(wt_Interface);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if(m_pView && m_pData) {
		//check to see if correct
		SettingsDlg::OptionState ops = m_pView->getOptionState();

		setShowOpSigs( ops.classState.showOpSig );

		( (InterfaceWidgetData*) m_pData ) -> m_bShowOperations = ops.classState.showOps;
		( (InterfaceWidgetData*) m_pData ) -> m_bShowPackage = ops.classState.showPackage ;
		( (InterfaceWidgetData*) m_pData ) -> m_bShowScope = ops.classState.showScope ;
		updateSigs();
	}
	//maybe loading and this may not be set.
	if( m_pObject ) {
		initUMLObject( m_pObject );
		calculateSize();
		update();
	}
	setDrawAsCircle(false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InterfaceWidget::~InterfaceWidget() {}
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
	p.setPen( m_pData->getLineColour() );

	if ( m_pData->getUseFillColor() ) {
		p.setBrush( m_pData->getFillColour() );
	} else {
		p.setBrush( m_pView->viewport()->backgroundColor() );
	}

	int w = width();

	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( ((InterfaceWidgetData*)m_pData)->m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
	p.setPen( QPen(black) );

	QFont font = m_pData->getFont();
	p.setFont(font);
	p.drawText(offsetX, offsetY + CIRCLE_SIZE,
		   w, fontHeight, AlignCenter, name);

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::drawAsConcept(QPainter& p, int offsetX, int offsetY) {
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	int w = width();
	int h = height();

	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( ((InterfaceWidgetData*)m_pData)->m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, offsetY, w, h);
	p.setPen(QPen(black));

	QFont font = m_pData->getFont();
	font.setBold(true);
	p.setFont(font);
	p.drawText(offsetX + INTERFACE_MARGIN, offsetY,
		   w - INTERFACE_MARGIN * 2,fontHeight,
		   AlignCenter, "<< " + m_pObject -> getStereotype() + " >>");

	font.setItalic( m_pObject -> getAbstract() );
	p.setFont(font);
	p.drawText(offsetX + INTERFACE_MARGIN, offsetY + fontHeight,
		   w - INTERFACE_MARGIN * 2, fontHeight, AlignCenter, name);
	font.setBold(false);
	font.setItalic(false);
	p.setFont(font);


	int operationsStart = fontHeight * 2;
	int y;

	if(((InterfaceWidgetData*)m_pData)->m_bShowOperations) {
		QFont font( p.font() );
		y = operationsStart;
		p.setPen( m_pData->getLineColour() );

		p.drawLine(offsetX, offsetY + y, offsetX + w - 1, offsetY + y);

		UMLOperation* obj = 0;
		QPtrList<UMLOperation>* list = ((UMLInterface*)m_pObject)->getOpList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString op = obj->toString(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs);
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
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight = fm.lineSpacing();

	int height = CIRCLE_SIZE + fontHeight;

	int width;
	if(((InterfaceWidgetData*)m_pData)->m_bShowPackage) {
		width = fm.width(m_pObject->getPackage() + "::" + getName());
	} else {
		width = fm.width(getName());
	}
	width = width<CIRCLE_SIZE ? CIRCLE_SIZE : width;

	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::calculateAsConceptSize() {
	if( !m_pData || !m_pObject)
		return;
	int width, height;
	QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	int fontHeight = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	int numOps = ((UMLInterface*)m_pObject)->operations();

	lines++; //for the stereotype

	height = width = 0;
	//set the height of the concept
	if(((InterfaceWidgetData*)m_pData)->m_bShowOperations) {
		lines += numOps;
		if(numOps == 0) {
			height += fontHeight / 2;//no ops, so just add a but of space
		}
	}
	height += lines * fontHeight;
	//now set the width of the concept
	//set width to name to start with
	if(((InterfaceWidgetData*)m_pData)->m_bShowPackage)
		width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->getPackage() + "::" + getName()).width();
	else
		width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(getName()).width();
	int w = getFontMetrics(FT_BOLD).boundingRect("<< " + m_pObject->getStereotype() + " >>").width();


	width = w > width?w:width;

	if(((InterfaceWidgetData*)m_pData)->m_bShowOperations) {
		QPtrList<UMLOperation>* list = ((UMLInterface*)m_pObject)->getOpList();
		UMLOperation * o = 0;
		for(o = list->first();o != 0; o = list->next()) {
			bool isAbstract = o -> getAbstract();
			bool isStatic = o -> getStatic();
			fm = getFontMetrics(isAbstract && isStatic
				? FT_ITALIC_UNDERLINE
				: isAbstract
					? FT_ITALIC
					: FT_UNDERLINE);
			int w = fm.boundingRect(o -> toString(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs)).width();
			width = w > width?w:width;
		}
	}
	//allow for width margin
	width += INTERFACE_MARGIN * 2;

	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::slotMenuSelection(int sel) {

	switch(sel) {
		case ListPopupMenu::mt_Operation:
			m_pView->getDocument()->createUMLObject(m_pObject, ListPopupMenu::convert_MT_OT( (ListPopupMenu::Menu_Type)sel) );
			break;

		case ListPopupMenu::mt_Show_Operations:
			((InterfaceWidgetData*)m_pData)->m_bShowOperations = ((InterfaceWidgetData*)m_pData)->m_bShowOperations?false:true;
			updateSigs();
			calculateSize();

			update();
			break;

		case ListPopupMenu::mt_Show_Operation_Signature:
			if(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_ShowSig || ((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_SigNoScope) {
				if(((InterfaceWidgetData*)m_pData)->m_bShowScope)
					((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
				else
					((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;
			} else if(((InterfaceWidgetData*)m_pData)->m_bShowScope)
				((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;

			else
				((InterfaceWidgetData*)m_pData)->m_ShowOpSigs =	Uml::st_SigNoScope;
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Scope:
			((InterfaceWidgetData*)m_pData)->m_bShowScope = ((InterfaceWidgetData*)m_pData)->m_bShowScope?false:true;
			updateSigs();
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_DrawAsCircle:
			((InterfaceWidgetData*)m_pData)->setDrawAsCircle( !((InterfaceWidgetData*)m_pData)->getDrawAsCircle() );
			updateSigs();
			calculateSize();
			update();
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::updateSigs() {
	//turn on scope
	if(((InterfaceWidgetData*)m_pData)->m_bShowScope) {
		if(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_NoSigNoScope) {
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
		} else if(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_SigNoScope) {
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;
		}
	} else { //turn off scope
		if(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_ShowSig) {
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_SigNoScope;
		} else if(((InterfaceWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_NoSig) {
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	}
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowScope(bool _scope) {
	((InterfaceWidgetData*)m_pData)->m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowOps(bool _show) {
	((InterfaceWidgetData*)m_pData)->m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setOpSignature(Signature_Type sig) {
	((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setShowPackage(bool _status) {
	((InterfaceWidgetData*)m_pData)->m_bShowPackage = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceWidget::getShowPackage() {
	return ((InterfaceWidgetData*)m_pData)->m_bShowPackage;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceWidget::setDrawAsCircle(bool drawAsCircle) {
	((InterfaceWidgetData*)m_pData)->m_bDrawAsCircle = drawAsCircle;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceWidget::getDrawAsCircle() {
	return ((InterfaceWidgetData*)m_pData)->m_bDrawAsCircle;
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
		if(((InterfaceWidgetData*)m_pData)->m_bShowScope)
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
		else
			((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if(((InterfaceWidgetData*)m_pData)->m_bShowScope)
		((InterfaceWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;
	else
		((InterfaceWidgetData*)m_pData)->m_ShowOpSigs =	Uml::st_SigNoScope;
	calculateSize();
	update();
}

bool InterfaceWidget::getShowOps() {
	return ((InterfaceWidgetData*)m_pData)->m_bShowOperations;
}

Uml::Signature_Type InterfaceWidget::getShowOpSigs() {
	return ((InterfaceWidgetData*)m_pData)->m_ShowOpSigs;
}

bool InterfaceWidget::getShowScope() {
	return ((InterfaceWidgetData*)m_pData)->m_bShowScope;
}
