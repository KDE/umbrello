/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classwidget.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "listpopupmenu.h"
#include "template.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>

ClassWidget::ClassWidget(UMLView * view, UMLObject *o, UMLWidgetData *pData) : UMLWidget(view,  o, pData) {
	m_pMenu = 0;
	if( m_pObject ) {
		calculateSize();
		update();
	}
}

ClassWidget::ClassWidget(UMLView * view, UMLObject *o) : UMLWidget(view,o, new ClassWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	calculateSize();
	m_pData->setType(wt_Class);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassWidget::ClassWidget(UMLView * view) : UMLWidget(view, new ClassWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	m_pData->setType(wt_Class);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if(m_pView && m_pData) {
		//check to see if correct
		SettingsDlg::OptionState ops = m_pView -> getOptionState();

		setShowAttSigs( ops.classState.showAttSig );
		setShowOpSigs( ops.classState.showOpSig );

		( (ClassWidgetData *) m_pData ) -> m_bShowAttributes = ops.classState.showAtts;
		( (ClassWidgetData *) m_pData ) -> m_bShowOperations = ops.classState.showOps;
		( (ClassWidgetData *) m_pData ) -> m_bShowPackage = ops.classState.showPackage ;
		( (ClassWidgetData *) m_pData ) -> m_bShowStereotype = ops.classState.showStereoType;
		( (ClassWidgetData *) m_pData ) -> m_bShowScope = ops.classState.showScope ;
		updateSigs();
	}
	//maybe loading and this may not be set.
	if( m_pObject ) {
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassWidget::~ClassWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());
	int na = ((UMLClass *)m_pObject)->attributes();

	QSize templatesBoxSize = calculateTemplatesBoxSize();
	int bodyOffsetY;
	if (templatesBoxSize.height() == 0) {
		bodyOffsetY = offsetY;
	} else {
		bodyOffsetY = offsetY + templatesBoxSize.height() - MARGIN;
	}
	int w;
	if (templatesBoxSize.width() == 0) {
		w = width();
	} else {
		w = width() - (templatesBoxSize.width() / 2);
	}
	int h;
	if (templatesBoxSize.height() == 0) {
		h = height();
	} else {
		h = height() - (templatesBoxSize.height() - MARGIN);
	}
	QFontMetrics fm = QFontMetrics( m_pData -> getFont() );
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( ((ClassWidgetData*)m_pData)->m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, bodyOffsetY, w, h);
	p.setPen(QPen(black));
	if(!((ClassWidgetData*)m_pData)->m_bShowOperations &&
	        ! ((ClassWidgetData*)m_pData)->m_bShowAttributes &&
	        ! ((ClassWidgetData*)m_pData)->m_bShowStereotype) {
		QFont font = m_pData -> getFont();
		font.setBold( true );
		font.setItalic( m_pObject-> getAbstract() );
		p.setFont( font );
		p.drawText(offsetX + MARGIN, bodyOffsetY, w - MARGIN * 2, h, AlignCenter, name);
		font.setBold( false );
		font.setItalic( false );
		p.setFont( font );
	} else {
		if(((ClassWidgetData*)m_pData)->m_bShowStereotype) {
			QFont f( m_pData -> getFont() );
			f.setBold( true );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY, w-MARGIN * 2,fontHeight,AlignCenter, "<< " + m_pObject -> getStereotype() + " >>");
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + fontHeight,w-MARGIN * 2,fontHeight,AlignCenter, name);
			f.setItalic( false );
			f.setBold( false );
			p.setFont( f );
			p.setPen(m_pData->getLineColour());
			p.drawLine(offsetX, bodyOffsetY + fontHeight * 2, offsetX + w - 1,
				   bodyOffsetY + fontHeight * 2);
		} else {
			QFont f( m_pData -> getFont() );
			f.setBold( true );
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY, w-MARGIN * 2,fontHeight,AlignCenter, name);
			f.setItalic( false );
			f.setBold( false );
			p.setFont( f );
			p.setPen(m_pData->getLineColour());
			p.drawLine(offsetX, bodyOffsetY + fontHeight, offsetX + w - 1, bodyOffsetY + fontHeight);
		}
	}
	int aStart = fontHeight;
	int oStart = na * fontHeight + aStart;
	if(((ClassWidgetData*)m_pData)->m_bShowStereotype) {
		aStart += fontHeight;
		oStart += fontHeight;
	}


	if(na==0)
		oStart = aStart + fontHeight / 2;
	int y;
	//change so only going through list once
	p.setPen(QPen(black));
	if(((ClassWidgetData*)m_pData)->m_bShowAttributes) {
		QFont f( p.font());
		y = aStart;
		UMLAttribute * obj=0;
		QPtrList<UMLAttribute> *list = ((UMLClass*)m_pObject)->getAttList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			f.setUnderline( obj -> getStatic() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + y, w-MARGIN * 2, fontHeight, AlignVCenter,
				   obj->toString(((ClassWidgetData*)m_pData)->m_ShowAttSigs));
			f.setUnderline(false);
			p.setFont(f);
			y+=fontHeight;
		}

	}//end if att
	if(((ClassWidgetData*)m_pData)->m_bShowOperations) {
		QFont f( p.font() );
		y = oStart;
		p.setPen(m_pData->getLineColour());
		if(((ClassWidgetData*)m_pData)->m_bShowAttributes)
			p.drawLine(offsetX, bodyOffsetY + y, offsetX + w - 1, bodyOffsetY + y);
		else
			y = aStart;
		UMLOperation * obj = 0;
		QPtrList<UMLOperation> *list = ((UMLClass*)m_pObject)->getOpList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString op = obj -> toString(((ClassWidgetData*)m_pData)->m_ShowOpSigs);
			p.setPen(QPen(black));
			f.setItalic( obj -> getAbstract() );
			f.setUnderline( obj -> getStatic() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + y, w-MARGIN * 2,fontHeight,AlignVCenter,op);
			f.setItalic( false );
			f.setUnderline(false);
			p.setFont( f );
			y+=fontHeight;
		}
	}//end if op

	//If there are any templates the draw them
	//this should really be controlled by a property in classwidgetdata
	if ( ((UMLClass*)m_pObject)->getTemplateList()->count() > 0 ) {

		QFont font = m_pData->getFont();
		p.setPen( QPen(m_pData->getLineColour(), 0, Qt::DotLine) );
		p.drawRect( offsetX + width() - templatesBoxSize.width(), offsetY,
			    templatesBoxSize.width(), templatesBoxSize.height() );

		p.setPen( QPen(black) );
		font.setItalic(false);
		font.setUnderline(false);
		font.setBold(false);
		QPtrList<UMLTemplate>* list = ((UMLClass*)m_pObject)->getTemplateList();
		UMLTemplate* theTemplate = 0;
		int y = offsetY + MARGIN;
		for ( theTemplate=list->first(); theTemplate != 0; theTemplate=list->next() ) {
			QString text = theTemplate->toString();
			p.drawText(offsetX + width() - templatesBoxSize.width() + MARGIN, y,
				   width()-MARGIN, fontHeight, AlignVCenter, text);
			y += fontHeight;
		}
	}
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QSize ClassWidget::calculateTemplatesBoxSize() {
	int count = ((UMLClass *)m_pObject)->templates();
	if (count == 0) {
		return QSize(0, 0);
	}

	int width, height;
	height = width = 0;

	QFont font = m_pData->getFont();
	font.setBold(true);//use bold for all calculations
	font.setItalic(true);
	QFontMetrics fm = QFontMetrics(font);

	height = count * fm.lineSpacing() + (MARGIN*2);

	QPtrList<UMLTemplate>* list = ((UMLClass *)m_pObject)->getTemplateList();
	UMLTemplate* theTemplate = 0;
	for ( theTemplate=list->first(); theTemplate != 0; theTemplate=list->next() ) {
		int textWidth = fm.width( theTemplate->toString() );
		width = textWidth>width ? textWidth : width;
	}

	width += (MARGIN*2);
	return QSize(width, height);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::calculateSize() {
	if( !m_pData || !m_pObject)
		return;
	int width, height;
	QFont font = m_pData -> getFont();
	font.setBold( true );//use bold for all calculations
	font.setItalic( true );
	QFontMetrics fm = QFontMetrics( font );
	int fontHeight  = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	int numAtts = ((UMLClass *)m_pObject)->attributes();
	int numOps = ((UMLClass *)m_pObject)->operations();

	if(((ClassWidgetData*)m_pData)->m_bShowStereotype)
		lines++;

	height = width = 0;
	//set the height of the concept
	if(((ClassWidgetData*)m_pData)->m_bShowAttributes) {
		lines += numAtts;
		if(numAtts == 0)
			height += fontHeight / 2;//no atts, so just add a bit of space
	}

	if(((ClassWidgetData*)m_pData)->m_bShowOperations) {
		lines += numOps;
		if(numOps == 0)

			height += fontHeight / 2;//no ops, so just add a but of space
	}
	height += lines * fontHeight;
	//now set the width of the concept
	//set width to name to start with
	if(((ClassWidgetData*)m_pData)->m_bShowPackage)
		width = fm.width(m_pObject -> getPackage() + "::" + getName());
	else
		width = fm.width(getName());
	int w = fm.width("<< " + m_pObject -> getStereotype() + " >>");


	width = w > width?w:width;

	if(((ClassWidgetData*)m_pData)->m_bShowAttributes) {
		QPtrList<UMLAttribute> * list = ((UMLClass *)m_pObject)->getAttList();
		UMLAttribute * a = 0;
		for(a = list->first();a != 0; a = list->next()) {
			int w = fm.width(a -> toString(((ClassWidgetData*)m_pData)->m_ShowAttSigs));
			width = w > width?w:width;
		}
	}
	if(((ClassWidgetData*)m_pData)->m_bShowOperations) {
		QPtrList<UMLOperation> * list = ((UMLClass *)m_pObject)->getOpList();
		UMLOperation * o = 0;
		for(o = list->first();o != 0; o = list->next()) {
			int w = fm.width(o -> toString(((ClassWidgetData*)m_pData)->m_ShowOpSigs));
			width = w > width?w:width;
		}
	}
	//allow for width margin
	width += MARGIN * 2;
	//add some more for luck
	//this prevents the edge pixels not being displayed when using some font/fontservers
	width += 4;
	if(!((ClassWidgetData*)m_pData)->m_bShowOperations &&
	        !((ClassWidgetData*)m_pData)->m_bShowAttributes &&
	        !((ClassWidgetData*)m_pData)->m_bShowStereotype) {
		height += MARGIN * 2;
	}

	QSize templatesBoxSize = calculateTemplatesBoxSize();
	if (templatesBoxSize.width() != 0) {
		width += templatesBoxSize.width() / 2;
	}
	if (templatesBoxSize.height() != 0) {
		height += templatesBoxSize.height() - MARGIN;
	}

	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::slotMenuSelection(int sel) {

	switch(sel) {
		case ListPopupMenu::mt_Attribute:
		case ListPopupMenu::mt_Operation:
			m_pView->getDocument()->createUMLObject(m_pObject, ListPopupMenu::convert_MT_OT( (ListPopupMenu::Menu_Type)sel) );
			break;

		case ListPopupMenu::mt_Show_Operations:
			((ClassWidgetData*)m_pData)->m_bShowOperations = ((ClassWidgetData*)m_pData)->m_bShowOperations?false:true;
			updateSigs();
			calculateSize();

			update();
			break;

		case ListPopupMenu::mt_Show_Attributes:
			((ClassWidgetData*)m_pData)->m_bShowAttributes = ((ClassWidgetData*)m_pData)->m_bShowAttributes?false:true;
			updateSigs();
			calculateSize();
			update();

			break;

		case ListPopupMenu::mt_Show_Operation_Signature:
			if(((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_ShowSig || ((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_SigNoScope) {
				if(((ClassWidgetData*)m_pData)->m_bShowScope)
					((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
				else
					((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;
			} else if(((ClassWidgetData*)m_pData)->m_bShowScope)
				((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;

			else
				((ClassWidgetData*)m_pData)->m_ShowOpSigs =	Uml::st_SigNoScope;
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Show_Attribute_Signature:
			if(((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_ShowSig ||
			        ((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_SigNoScope) {
				if(((ClassWidgetData*)m_pData)->m_bShowScope)


					((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSig;
				else

					((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSigNoScope;
			}
			else if(((ClassWidgetData*)m_pData)->m_bShowScope)
				((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_ShowSig;
			else
				((ClassWidgetData*)m_pData)->m_ShowAttSigs =	Uml::st_SigNoScope;
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Scope:
			((ClassWidgetData*)m_pData)->m_bShowScope = ((ClassWidgetData*)m_pData)->m_bShowScope?false:true;
			updateSigs();
			calculateSize();
			update();
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::updateSigs() {
	if(((ClassWidgetData*)m_pData)->m_bShowScope) {
		if(((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_NoSigNoScope)
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
		else if(((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_SigNoScope)
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;

		if(((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_NoSigNoScope)
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSig;
		else if(((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_SigNoScope)
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_ShowSig;
	} else {
		if(((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_ShowSig)
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_SigNoScope;
		else if(((ClassWidgetData*)m_pData)->m_ShowOpSigs == Uml::st_NoSig)
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;

		if(((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_ShowSig)
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_SigNoScope;
		else if(((ClassWidgetData*)m_pData)->m_ShowAttSigs == Uml::st_NoSig)
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowScope(bool _scope) {
	((ClassWidgetData*)m_pData)->m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowAtts(bool _show) {
	((ClassWidgetData*)m_pData)->m_bShowAttributes = _show;
	updateSigs();

	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowOps(bool _show) {
	((ClassWidgetData*)m_pData)->m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setOpSignature(Signature_Type sig) {
	((ClassWidgetData*)m_pData)->m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setAttSignature(Signature_Type sig) {
	((ClassWidgetData*)m_pData)->m_ShowAttSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowStereotype(bool _status) {
	((ClassWidgetData*)m_pData)->m_bShowStereotype = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowPackage(bool _status) {
	((ClassWidgetData*)m_pData)->m_bShowPackage = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ClassWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if(status) {
		calculateSize();
	}


	return status;
}

/** Synchronizes the Widget's m_pData member with its display properties, for example:

 the X and Y positions of the widget, etc */
void ClassWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}

void ClassWidget::setShowAttSigs(bool _status) {
	if( !_status ) {
		if(((ClassWidgetData*)m_pData)->m_bShowScope)
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSig;
		else
			((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
	else if(((ClassWidgetData*)m_pData)->m_bShowScope)
		((ClassWidgetData*)m_pData)->m_ShowAttSigs = Uml::st_ShowSig;
	else
		((ClassWidgetData*)m_pData)->m_ShowAttSigs =	Uml::st_SigNoScope;
	calculateSize();
	update();
}

void ClassWidget::setShowOpSigs(bool _status) {
	if( !_status ) {
		if(((ClassWidgetData*)m_pData)->m_bShowScope)
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSig;
		else
			((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if(((ClassWidgetData*)m_pData)->m_bShowScope)
		((ClassWidgetData*)m_pData)->m_ShowOpSigs = Uml::st_ShowSig;
	else
		((ClassWidgetData*)m_pData)->m_ShowOpSigs =	Uml::st_SigNoScope;
	calculateSize();
	update();
}
