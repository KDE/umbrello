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
#include "umldoc.h"
#include <kdebug.h>
#include <qpainter.h>

ClassWidget::ClassWidget(UMLView * view, UMLObject *o, UMLWidgetData *pData)
	: UMLWidget(view,  o, pData)
{
	if( m_pObject ) {
		calculateSize();
		update();
	}
}

ClassWidget::ClassWidget(UMLView * view, UMLObject *o) 
	: UMLWidget(view,o, new ClassWidgetData(view->getOptionState() )) 
{
	init();
	setSize(100,30);
	calculateSize();
	m_pData->setType(wt_Class);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassWidget::ClassWidget(UMLView * view) 
	: UMLWidget(view, new ClassWidgetData(view->getOptionState() )) 
{
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

		ClassWidgetData *data = (ClassWidgetData *) m_pData;
		data -> setShowAttributes( ops.classState.showAtts );
		data -> setShowOperations( ops.classState.showOps );
		data -> setShowPackage( ops.classState.showPackage );
		data -> setShowStereotype( ops.classState.showStereoType );
		data -> setShowScope( ops.classState.showScope );
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
	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	ClassWidgetData *data = (ClassWidgetData *) m_pData;
	if (data->getShowPackage()) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, bodyOffsetY, w, h);
	p.setPen(QPen(black));
	if ( !data->getShowOperations() &&
	     !data->getShowAttributes() &&
	     !data->getShowStereotype() ) {
		QFont font = m_pData -> getFont();
		font.setBold( true );
		font.setItalic( m_pObject-> getAbstract() );
		p.setFont( font );
		p.drawText(offsetX + MARGIN, bodyOffsetY, w - MARGIN * 2, h, AlignCenter, name);
		font.setBold( false );
		font.setItalic( false );
		p.setFont( font );
	} else {
		if (data->getShowStereotype()) {
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
	if (data->getShowStereotype()) {
		aStart += fontHeight;
		oStart += fontHeight;
	}


	if(na==0)
		oStart = aStart + fontHeight / 2;
	int y;
	//change so only going through list once
	p.setPen(QPen(black));
	if (data->getShowAttributes()) {
		QFont f( p.font());
		y = aStart;
		UMLAttribute * obj=0;
		QPtrList<UMLAttribute> *list = ((UMLClass*)m_pObject)->getAttList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			f.setUnderline( obj -> getStatic() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + y, w-MARGIN * 2, fontHeight, AlignVCenter,
				   obj->toString(data->getShowAttSigs()));
			f.setUnderline(false);
			p.setFont(f);
			y+=fontHeight;
		}

	}//end if att
	if (data->getShowOperations()) {
		QFont f( p.font() );
		y = oStart;
		p.setPen(m_pData->getLineColour());
		if (data->getShowAttributes())
			p.drawLine(offsetX, bodyOffsetY + y, offsetX + w - 1, bodyOffsetY + y);
		else
			y = aStart;
		UMLOperation * obj = 0;
		QPtrList<UMLOperation> *list = ((UMLClass*)m_pObject)->getOpList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString op = obj -> toString( data->getShowOpSigs() );
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

	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);

	height = count * fm.lineSpacing() + (MARGIN*2);

	QPtrList<UMLTemplate>* list = ((UMLClass *)m_pObject)->getTemplateList();
	UMLTemplate* theTemplate = 0;
	for ( theTemplate=list->first(); theTemplate != 0; theTemplate=list->next() ) {
		int textWidth = fm.boundingRect( theTemplate->toString() ).width();
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
	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
	int fontHeight  = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	int numAtts = ((UMLClass *)m_pObject)->attributes();
	int numOps = ((UMLClass *)m_pObject)->operations();

	ClassWidgetData* data = (ClassWidgetData*)m_pData;
	if (data->getShowStereotype())
		lines++;

	height = width = 0;
	//set the height of the concept
	if (data->getShowAttributes()) {
		lines += numAtts;
		if(numAtts == 0)
			height += fontHeight / 2;//no atts, so just add a bit of space
	}

	if (data->getShowOperations()) {
		lines += numOps;
		if(numOps == 0)

			height += fontHeight / 2;//no ops, so just add a but of space
	}
	height += lines * fontHeight;
	//now set the width of the concept
	//set width to name to start with
	if (data->getShowPackage())
		width = getFontMetrics(m_pObject && m_pObject-> getAbstract()
			? FT_BOLD_ITALIC
			: FT_BOLD).boundingRect(m_pObject -> getPackage() + "::" + getName()).width();
	else
		width = getFontMetrics(m_pObject && m_pObject-> getAbstract()
			? FT_BOLD_ITALIC
			: FT_BOLD).boundingRect(getName()).width();
	int w = getFontMetrics(FT_BOLD).boundingRect("<< " + m_pObject -> getStereotype() + " >>").width();


	width = w > width?w:width;

	if (data->getShowAttributes()) {
		QPtrList<UMLAttribute> * list = ((UMLClass *)m_pObject)->getAttList();
		UMLAttribute * a = 0;
		for(a = list->first();a != 0; a = list->next()) {
			int w = getFontMetrics(a->getStatic() ? FT_UNDERLINE : FT_NORMAL).boundingRect(a -> toString(data->getShowAttSigs())).width();
			width = w > width?w:width;
		}
	}
	if (data->getShowOperations()) {
		QPtrList<UMLOperation> * list = ((UMLClass *)m_pObject)->getOpList();
		UMLOperation * o = 0;
		for(o = list->first();o != 0; o = list->next()) {
			bool isAbstract = o -> getAbstract();
			bool isStatic = o -> getStatic();
			fm = getFontMetrics(isAbstract && isStatic
				? FT_ITALIC_UNDERLINE
				: isAbstract
					? FT_ITALIC
					: FT_UNDERLINE);

			int w = fm.boundingRect(o -> toString(data->getShowOpSigs())).width();
			width = w > width?w:width;
		}
	}
	//allow for width margin
	width += MARGIN * 2;

	if (!data->getShowOperations() &&
	        !data->getShowAttributes() &&
	        !data->getShowStereotype()) {
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
	ClassWidgetData* data = (ClassWidgetData*)m_pData;
	switch(sel) {
		case ListPopupMenu::mt_Attribute:
		case ListPopupMenu::mt_Operation:
			m_pView->getDocument()->createUMLObject(m_pObject, ListPopupMenu::convert_MT_OT( (ListPopupMenu::Menu_Type)sel) );
			break;

		case ListPopupMenu::mt_Show_Operations:
			data->setShowOperations( !data->getShowOperations() );
			updateSigs();
			calculateSize();

			update();
			break;

		case ListPopupMenu::mt_Show_Attributes:
			data->setShowAttributes( !data->getShowAttributes() );
			updateSigs();
			calculateSize();
			update();

			break;

		case ListPopupMenu::mt_Show_Operation_Signature:
			if (data->getShowOpSigs() == Uml::st_ShowSig ||
			    data->getShowOpSigs() == Uml::st_SigNoScope) {
				if (data->getShowScope())
					data->setShowOpSigs( Uml::st_NoSig );
				else
					data->setShowOpSigs( Uml::st_NoSigNoScope );
			} else if (data->getShowScope())
				data->setShowOpSigs( Uml::st_ShowSig );

			else
				data->setShowOpSigs( Uml::st_SigNoScope );
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Show_Attribute_Signature:
			if (data->getShowAttSigs() == Uml::st_ShowSig ||
			        data->getShowAttSigs() == Uml::st_SigNoScope) {
				if (data->getShowScope())
					data->setShowAttSigs( Uml::st_NoSig );
				else
					data->setShowAttSigs( Uml::st_NoSigNoScope );
			}
			else if (data->getShowScope())
				data->setShowAttSigs( Uml::st_ShowSig );
			else
				data->setShowAttSigs( Uml::st_SigNoScope );
			calculateSize();
			update();
			break;

		case ListPopupMenu::mt_Scope:
			data->setShowScope( !data->getShowScope() );
			updateSigs();
			calculateSize();
			update();
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::updateSigs() {
	ClassWidgetData* data = (ClassWidgetData*)m_pData;
	if (data->getShowScope()) {
		if (data->getShowOpSigs() == Uml::st_NoSigNoScope)
			data->setShowOpSigs( Uml::st_NoSig );
		else if (data->getShowOpSigs() == Uml::st_SigNoScope)
			data->setShowOpSigs( Uml::st_ShowSig );

		if (data->getShowAttSigs() == Uml::st_NoSigNoScope)
			data->setShowAttSigs( Uml::st_NoSig );
		else if (data->getShowAttSigs() == Uml::st_SigNoScope)
			data->setShowAttSigs( Uml::st_ShowSig );
	} else {
		if (data->getShowOpSigs() == Uml::st_ShowSig)
			data->setShowOpSigs( Uml::st_SigNoScope );
		else if (data->getShowOpSigs() == Uml::st_NoSig)
			data->setShowOpSigs( Uml::st_NoSigNoScope );

		if (data->getShowAttSigs() == Uml::st_ShowSig)
			data->setShowAttSigs( Uml::st_SigNoScope );
		else if(data->getShowAttSigs() == Uml::st_NoSig)
			data->setShowAttSigs( Uml::st_NoSigNoScope );
	}
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowScope(bool _scope) {
	((ClassWidgetData*)m_pData)->setShowScope( _scope );
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowAtts(bool _show) {
	((ClassWidgetData*)m_pData)->setShowAttributes( _show );
	updateSigs();

	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowOps(bool _show) {
	((ClassWidgetData*)m_pData)->setShowOperations( _show );
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setOpSignature(Signature_Type sig) {
	((ClassWidgetData*)m_pData)->setShowOpSigs( sig );
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setAttSignature(Signature_Type sig) {
	((ClassWidgetData*)m_pData)->setShowAttSigs( sig );
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowStereotype(bool _status) {
	((ClassWidgetData*)m_pData)->setShowStereotype( _status );
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowPackage(bool _status) {
	((ClassWidgetData*)m_pData)->setShowPackage( _status );
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
	ClassWidgetData* data = (ClassWidgetData*)m_pData;
	if( !_status ) {
		if (data->getShowScope())
			data->setShowAttSigs( Uml::st_NoSig );
		else
			data->setShowAttSigs( Uml::st_NoSigNoScope );
	}
	else if (data->getShowScope())
		data->setShowAttSigs( Uml::st_ShowSig );
	else
		data->setShowAttSigs( Uml::st_SigNoScope );
	calculateSize();
	update();
}

void ClassWidget::setShowOpSigs(bool _status) {
	ClassWidgetData* data = (ClassWidgetData*)m_pData;
	if( !_status ) {
		if(data->getShowScope())
			data->setShowOpSigs( Uml::st_NoSig );
		else
			data->setShowOpSigs( Uml::st_NoSigNoScope );

	} else if (data->getShowScope())
		data->setShowOpSigs( Uml::st_ShowSig );
	else
		data->setShowOpSigs( Uml::st_SigNoScope );
	calculateSize();
	update();
}
