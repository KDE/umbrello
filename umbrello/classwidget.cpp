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

ClassWidget::ClassWidget(UMLView * view, UMLClass *o) : UMLWidget(view, o)
{
	init();
	setSize(100,30);
	calculateSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::init() {
	UMLWidget::setBaseType(wt_Class);
	m_pMenu = 0;
	m_ShowAttSigs = Uml::st_ShowSig;
	m_ShowOpSigs = Uml::st_ShowSig;
	//set defaults from m_pView
	if ( m_pView ) {
		const SettingsDlg::OptionState& ops = m_pView -> getOptionState();
		m_bShowAttributes = ops.classState.showAtts;
		m_bShowOperations = ops.classState.showOps;
		m_bShowPackage = ops.classState.showPackage;
		m_bShowStereotype = ops.classState.showStereoType;
		m_bShowScope = ops.classState.showScope;
		setShowAttSigs( ops.classState.showAttSig );
		setShowOpSigs( ops.classState.showOpSig );
		updateSigs();
	} else {
		m_bShowAttributes = true;
		m_bShowOperations = true;
		m_bShowPackage = false;
		m_bShowStereotype = false;
		m_bShowScope = true;
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
	if (!m_pObject) {
		kdDebug() << "ClassWidget::draw(): m_pObject is NULL" << endl;
		return;
	}
	p.setPen( UMLWidget::getLineColour() );
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
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
	if ( m_bShowPackage ) {
		name = m_pObject -> getPackage() + "::" + this -> getName();
	} else {
		name = this -> getName();
	}

	p.drawRect(offsetX, bodyOffsetY, w, h);
	p.setPen(QPen(black));
	if ( !m_bShowOperations && !m_bShowAttributes && !m_bShowStereotype ) {
		QFont font = UMLWidget::getFont();
		font.setBold( true );
		font.setItalic( m_pObject-> getAbstract() );
		p.setFont( font );
		p.drawText(offsetX + MARGIN, bodyOffsetY, w - MARGIN * 2, h, AlignCenter, name);
		font.setBold( false );
		font.setItalic( false );
		p.setFont( font );
	} else {
		/* if no stereotype is given, we don't want to show the empty << >> */
		if (m_bShowStereotype && m_pObject->getStereotype().isEmpty() == false ) {
			QFont f( UMLWidget::getFont() );
			f.setBold( true );
			//FIXME why is underline sometimes true
			f.setUnderline( false );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY, w-MARGIN * 2,fontHeight,AlignCenter, "«" + m_pObject -> getStereotype() + "»");
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + fontHeight,w-MARGIN * 2,fontHeight,AlignCenter, name);
			f.setItalic( false );
			f.setBold( false );
			p.setFont( f );
			p.setPen( UMLWidget::getLineColour() );
			p.drawLine(offsetX, bodyOffsetY + fontHeight * 2, offsetX + w - 1,
				   bodyOffsetY + fontHeight * 2);
		} else {
			QFont f( UMLWidget::getFont() );
			f.setBold( true );
			f.setUnderline( false );
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY, w-MARGIN * 2,fontHeight,AlignCenter, name);
			f.setItalic( false );
			f.setBold( false );
			p.setFont( f );
			p.setPen( UMLWidget::getLineColour() );
			p.drawLine(offsetX, bodyOffsetY + fontHeight, offsetX + w - 1, bodyOffsetY + fontHeight);
		}
	}
	int aStart = fontHeight;
	int oStart = na * fontHeight + aStart;
	if (m_bShowStereotype) {
		aStart += fontHeight;
		oStart += fontHeight;
	}


	if(na==0)
		oStart = aStart + fontHeight / 2;
	int y;
	//change so only going through list once
	p.setPen(QPen(black));
	if (m_bShowAttributes) {
		QFont f( p.font());
		y = aStart;
		UMLClassifierListItem* obj=0;
		QPtrList<UMLClassifierListItem>* list = ((UMLClass*)m_pObject)->getAttList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString att = obj -> toString( m_ShowAttSigs);
			f.setUnderline( obj -> getStatic() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + y, w-MARGIN * 2, fontHeight, AlignVCenter, att);
			f.setUnderline(false);
			p.setFont(f);
			y+=fontHeight;
		}

	}//end if att
	if (m_bShowOperations) {
		QFont f( p.font() );
		y = oStart;
		p.setPen( UMLWidget::getLineColour() );
		if (m_bShowAttributes)
			p.drawLine(offsetX, bodyOffsetY + y, offsetX + w - 1, bodyOffsetY + y);
		else
			y = aStart;
		p.setPen(QPen(black));
		UMLClassifierListItem* obj = 0;
		QPtrList<UMLClassifierListItem> *list = ((UMLClass*)m_pObject)->getOpList();
		for(obj=list->first();obj != 0;obj=list->next()) {
			QString op = obj -> toString( m_ShowOpSigs );
			f.setItalic( obj -> getAbstract() );
			f.setUnderline( obj -> getStatic() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + y, w-MARGIN * 2, fontHeight, AlignVCenter,op);
			f.setItalic( false );
			f.setUnderline(false);
			p.setFont( f );
			y+=fontHeight;
		}
	}//end if op

	//If there are any templates then draw them
	if ( ((UMLClass*)m_pObject)->getTemplateList()->count() > 0 ) {

		QFont font = UMLWidget::getFont();
		p.setPen( QPen(UMLWidget::getLineColour(), 0, Qt::DotLine) );
		p.drawRect( offsetX + width() - templatesBoxSize.width(), offsetY,
			    templatesBoxSize.width(), templatesBoxSize.height() );

		p.setPen( QPen(black) );
		font.setItalic(false);
		font.setUnderline(false);
		font.setBold(false);
		QPtrList<UMLClassifierListItem>* list = ((UMLClass*)m_pObject)->getTemplateList();
		UMLClassifierListItem* theTemplate = 0;
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

	QPtrList<UMLClassifierListItem>* list = ((UMLClass *)m_pObject)->getTemplateList();
	UMLClassifierListItem* theTemplate = 0;
	for ( theTemplate=list->first(); theTemplate != 0; theTemplate=list->next() ) {
		int textWidth = fm.boundingRect( theTemplate->toString() ).width();
		width = textWidth>width ? textWidth : width;
	}

	width += (MARGIN*2);
	return QSize(width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::calculateSize() {
	if( !m_pObject )
		return;
	int width, height;
	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
	int fontHeight  = fm.lineSpacing();

	int lines = 1;//always have one line - for name
	int numAtts = ((UMLClass *)m_pObject)->attributes();
	int numOps = ((UMLClass *)m_pObject)->operations();

	if (m_bShowStereotype)
		lines++;

	height = width = 0;
	//set the height of the concept
	if (m_bShowAttributes) {
		lines += numAtts;
		if(numAtts == 0)
			height += fontHeight / 2;//no atts, so just add a bit of space
	}

	if (m_bShowOperations) {
		lines += numOps;
		if(numOps == 0)

			height += fontHeight / 2;//no ops, so just add a but of space
	}
	height += lines * fontHeight;
	//now set the width of the concept
	//set width to name to start with
	if (m_bShowPackage)
		width = getFontMetrics(m_pObject && m_pObject-> getAbstract()
			? FT_BOLD_ITALIC
			: FT_BOLD).boundingRect(m_pObject -> getPackage() + "::" + getName()).width();
	else
		width = getFontMetrics(m_pObject && m_pObject-> getAbstract()
			? FT_BOLD_ITALIC
			: FT_BOLD).boundingRect(getName()).width();

	/* if no stereotype is given, this line has a width of 0 */
	int w = 0;
	if (m_pObject->getStereotype().isEmpty() == false ) {
		w = getFontMetrics(FT_BOLD).boundingRect("«" + m_pObject -> getStereotype() + "»").width();
	}

	width = w > width?w:width;

	/* calculate width of the attributes */
	if (m_bShowAttributes) {
		QPtrList<UMLClassifierListItem>* list = ((UMLClass *)m_pObject)->getAttList();
		UMLClassifierListItem* a = 0;
		for(a = list->first();a != 0; a = list->next()) {
			bool isStatic = a->getStatic();
			if (isStatic)
			{
				fm = getFontMetrics(FT_UNDERLINE);
			} else {
				fm = getFontMetrics(FT_NORMAL);
			}
			int w = fm.boundingRect(a -> toString( m_ShowAttSigs )).width();
			width = w > width?w:width;
		}
	}

	/* calculate width of the operations */
	if (m_bShowOperations) {
		QPtrList<UMLClassifierListItem> * list = ((UMLClass *)m_pObject)->getOpList();
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
	width += MARGIN * 2;

	if (!m_bShowOperations && !m_bShowAttributes && !m_bShowStereotype) {
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
//	adjustAssocs( getX(), getY() );//adjust assoc lines. Doesnt appear to be needed. -b.t.
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::slotMenuSelection(int sel) {
	switch(sel) {
		case ListPopupMenu::mt_Attribute:
		case ListPopupMenu::mt_Operation:
		{
			if ( m_pView->getDocument()->createUMLObject(m_pObject, ListPopupMenu::convert_MT_OT( (ListPopupMenu::Menu_Type)sel) ) )  {
				m_pView->getDocument()->setModified();
			}
			calculateSize();
			update();
			break;
		}
		case ListPopupMenu::mt_Show_Operations:
			toggleShowOps();
			break;

		case ListPopupMenu::mt_Show_Attributes:
			toggleShowAtts();
			break;

		case ListPopupMenu::mt_Show_Operation_Signature:
			toggleShowOpSigs();
			break;

		case ListPopupMenu::mt_Show_Attribute_Signature:
			toggleShowAttSigs();
			break;

		case ListPopupMenu::mt_Scope:
			toggleShowScope();
			break;

		case ListPopupMenu::mt_Show_Packages:
			toggleShowPackage();
			break;

		case ListPopupMenu::mt_Show_Stereotypes:
			toggleShowStereotype();
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::updateSigs() {
	if (m_bShowScope) {
		if (m_ShowOpSigs == Uml::st_NoSigNoScope)
			m_ShowOpSigs = Uml::st_NoSig;
		else if (m_ShowOpSigs == Uml::st_SigNoScope)
			m_ShowOpSigs = Uml::st_ShowSig;

		if (m_ShowAttSigs == Uml::st_NoSigNoScope)
			m_ShowAttSigs = Uml::st_NoSig;
		else if (m_ShowAttSigs == Uml::st_SigNoScope)
			m_ShowAttSigs = Uml::st_ShowSig;
	} else {
		if (m_ShowOpSigs == Uml::st_ShowSig)
			m_ShowOpSigs = Uml::st_SigNoScope;
		else if (m_ShowOpSigs == Uml::st_NoSig)
			m_ShowOpSigs = Uml::st_NoSigNoScope;

		if (m_ShowAttSigs == Uml::st_ShowSig)
			m_ShowAttSigs = Uml::st_SigNoScope;
		else if(m_ShowAttSigs == Uml::st_NoSig)
			m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowScope(bool _scope) {
	m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowAtts(bool _show) {
	m_bShowAttributes = _show;
	updateSigs();

	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowOps(bool _show) {
	m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setOpSignature(Signature_Type sig) {
	m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setAttSignature(Signature_Type sig) {
	m_ShowAttSigs = sig;
	updateSigs();
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowStereotype(bool _status) {
	m_bShowStereotype = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::setShowPackage(bool _status) {
	m_bShowPackage = _status;
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

void ClassWidget::setShowAttSigs(bool _status) {
	if( !_status ) {
		if (m_bShowScope)
			m_ShowAttSigs = Uml::st_NoSig;
		else
			m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
	else if (m_bShowScope)
		m_ShowAttSigs = Uml::st_ShowSig;
	else
		m_ShowAttSigs = Uml::st_SigNoScope;
	calculateSize();
	update();
}

void ClassWidget::setShowOpSigs(bool _status) {
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

bool ClassWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement( "UML:ClassWidget" );
	bool status = UMLWidget::saveToXMI( qDoc, conceptElement );
	conceptElement.setAttribute( "showattributes", m_bShowAttributes );
	conceptElement.setAttribute( "showoperations", m_bShowOperations );
	conceptElement.setAttribute( "showattsigs", m_ShowAttSigs );
	conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
	conceptElement.setAttribute( "showpackage", m_bShowPackage );
	conceptElement.setAttribute( "showstereotype", m_bShowStereotype );
	conceptElement.setAttribute( "showscope", m_bShowScope );
	qElement.appendChild( conceptElement );
	return status;
}

bool ClassWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	QString showatts = qElement.attribute( "showattributes", "1" );
	QString showops = qElement.attribute( "showoperations", "1" );
	QString showattsigs = qElement.attribute( "showattsigs", "600" );
	QString showopsigs = qElement.attribute( "showopsigs", "600" );
	QString showpackage = qElement.attribute( "showpackage", "0" );
	QString showstereo = qElement.attribute( "showstereotype", "0" );
	QString showscope = qElement.attribute( "showscope", "0" );

	m_bShowAttributes = (bool)showatts.toInt();
	m_bShowOperations = (bool)showops.toInt();
	m_ShowAttSigs = (Uml::Signature_Type)showattsigs.toInt();
	m_ShowOpSigs = (Uml::Signature_Type)showopsigs.toInt();
	m_bShowPackage = (bool)showpackage.toInt();
	m_bShowStereotype = (bool)showstereo.toInt();
	m_bShowScope = (bool)showscope.toInt();

	return true;
}

void ClassWidget::toggleShowAtts()
{
	m_bShowAttributes = !m_bShowAttributes;
	updateSigs();
	calculateSize();
	update();

	return;
}

void ClassWidget::toggleShowOps()
{
	m_bShowOperations = !m_bShowOperations;
	updateSigs();
	calculateSize();

	update();

	return;
}

void ClassWidget::toggleShowOpSigs()
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

void ClassWidget::toggleShowAttSigs()
{
	if (m_ShowAttSigs == Uml::st_ShowSig ||
        m_ShowAttSigs == Uml::st_SigNoScope) {
		if (m_bShowScope) {
			m_ShowAttSigs = Uml::st_NoSig;
		} else {
			m_ShowAttSigs = Uml::st_NoSigNoScope;
		}
	} else if (m_bShowScope) {
				m_ShowAttSigs = Uml::st_ShowSig;
	} else {
		m_ShowAttSigs = Uml::st_SigNoScope;
	}
	calculateSize();
	update();

	return;
}

void ClassWidget::toggleShowScope()
{
	m_bShowScope = !m_bShowScope;
	updateSigs();
	calculateSize();
	update();

	return;
}

void ClassWidget::toggleShowPackage()
{
	m_bShowPackage = !m_bShowPackage;
	updateSigs();
	calculateSize();
	update();

	return;
}

void ClassWidget::toggleShowStereotype()
{
	m_bShowStereotype = !m_bShowStereotype;
	updateSigs();
	calculateSize();
	update();

	return;
}
