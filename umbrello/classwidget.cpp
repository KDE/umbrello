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

#include "classwidget.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "listpopupmenu.h"
#include "template.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include <kdebug.h>
#include <qpainter.h>

ClassWidget::ClassWidget(UMLView * view, UMLClass *o)
  : ClassifierWidget(view, o, Uml::wt_Class)
{
	init();
	setSize(100,30);
	calculateSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::init() {
	m_ShowAttSigs = Uml::st_ShowSig;
	//set defaults from m_pView
	const Settings::OptionState& ops = m_pView -> getOptionState();
	m_bShowAttributes = ops.classState.showAtts;
	m_bShowStereotype = ops.classState.showStereoType;
	setShowAttSigs( ops.classState.showAttSig );
	updateSigs();
	//maybe loading and this may not be set.
	if( m_pObject ) {
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassWidget::~ClassWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////

int ClassWidget::displayedAttributes() {
	if (!m_bShowAttributes)
		return 0;
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	int count = 0;
	UMLClassifierListItemList list = c->getFilteredList(Uml::ot_Attribute);
	for (UMLClassifierListItem *m = list.first(); m; m = list.next()) {
		if (!(m_bShowPublicOnly && m->getScope() != Uml::Public))
			count++;
	}
	return count;
}

void ClassWidget::drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
			      int x, int bodyOffsetY, int y, int fontHeight) {
	QFont f = UMLWidget::getFont();
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	UMLClassifierListItemList list = c->getFilteredList(ot);
	for (UMLClassifierListItem *obj = list.first(); obj; obj = list.next()) {
		if (m_bShowPublicOnly && obj->getScope() != Uml::Public)
			continue;
		QString text = obj->toString(sigType);
		f.setItalic( obj->getAbstract() );
		f.setBold(false);
		f.setUnderline( obj->getStatic() );
		p.setFont( f );
		QFontMetrics fontMetrics(f);
		p.drawText(x, bodyOffsetY + y, fontMetrics.width(text), fontHeight, AlignVCenter, text);
		f.setItalic(false);
		f.setUnderline(false);
		p.setFont(f);
		y += fontHeight;
	}
}

void ClassWidget::draw(QPainter & p, int offsetX, int offsetY) {
	if (!m_pObject) {
		kdDebug() << "ClassWidget::draw(): m_pObject is NULL" << endl;
		return;
	}
	UMLWidget::draw(p, offsetX, offsetY);
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	QSize templatesBoxSize = calculateTemplatesBoxSize();
	int bodyOffsetY = offsetY;
	if (templatesBoxSize.height() > 0)
		bodyOffsetY += templatesBoxSize.height() - MARGIN;
	int w = width();
	if (templatesBoxSize.width() > 0)
		w -= templatesBoxSize.width() / 2;
	int h = height();
	if (templatesBoxSize.height() > 0)
		h -= templatesBoxSize.height() - MARGIN;
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
			p.drawText(offsetX + MARGIN, bodyOffsetY, w-MARGIN * 2,fontHeight,AlignCenter, m_pObject -> getStereotype());
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			p.drawText(offsetX + MARGIN, bodyOffsetY + fontHeight,w-MARGIN * 2,fontHeight,AlignCenter, name);
			f.setItalic( false );
			f.setBold( false );
			p.setFont( f );
			UMLWidget::draw(p, offsetX, offsetY);
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
			UMLWidget::draw(p, offsetX, offsetY);
			p.drawLine(offsetX, bodyOffsetY + fontHeight, offsetX + w - 1, bodyOffsetY + fontHeight);
		}
	}

	int aStart = fontHeight;
	int numAtts = displayedAttributes();
	if (m_bShowStereotype) {
		aStart += fontHeight;
	}

	p.setPen(QPen(black));
	UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);

	if (m_bShowAttributes) {
		drawMembers(p, Uml::ot_Attribute, m_ShowAttSigs, offsetX + MARGIN,
			    bodyOffsetY, aStart, fontHeight);
	}//end if att

	if (m_bShowOperations) {
		QFont f = UMLWidget::getFont();
		int oStart = numAtts * fontHeight + aStart;
		if (m_bShowStereotype) {
			oStart += fontHeight;
		}
		if (numAtts == 0)
			oStart = aStart + fontHeight / 2;
		int y = oStart;
		if (m_bShowAttributes) {
			UMLWidget::draw(p, offsetX, offsetY);
			p.drawLine(offsetX, bodyOffsetY + y, offsetX + w - 1, bodyOffsetY + y);
			p.setPen(QPen(black));
		} else
			y = aStart;
		drawMembers(p, Uml::ot_Operation, m_ShowOpSigs, offsetX + MARGIN,
			    bodyOffsetY, y, fontHeight);
	}//end if op

	//If there are any templates then draw them
	UMLClassifierListItemList tlist = umlclass->getFilteredList(Uml::ot_Template);
	if ( tlist.count() > 0 ) {

		QFont font = UMLWidget::getFont();
		UMLWidget::draw(p, offsetX, offsetY);
		QPen pen = p.pen();
		pen.setStyle(DotLine);
		p.setPen(pen);
		p.drawRect( offsetX + width() - templatesBoxSize.width(), offsetY,
			    templatesBoxSize.width(), templatesBoxSize.height() );

		p.setPen( QPen(black) );
		font.setItalic(false);
		font.setUnderline(false);
		font.setBold(false);
		p.setFont(font);
		QFontMetrics fontMetrics(font);
		UMLClassifierListItem* theTemplate = 0;
		const int x = offsetX + width() - templatesBoxSize.width() + MARGIN;
		int y = offsetY + MARGIN;
		for ( theTemplate = tlist.first(); theTemplate; theTemplate = tlist.next() ) {
			QString text = theTemplate->toString();
			p.drawText(x, y, fontMetrics.width(text), fontHeight, AlignVCenter, text);
			y += fontHeight;
		}
	}
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QSize ClassWidget::calculateTemplatesBoxSize() {
	UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);
	UMLClassifierListItemList list = umlclass->getFilteredList(Uml::ot_Template);
	int count = list.count();
	if (count == 0) {
		return QSize(0, 0);
	}

	int width, height;
	height = width = 0;

	QFont font = UMLWidget::getFont();
	font.setItalic(false);
	font.setUnderline(false);
	font.setBold(false);
	QFontMetrics fm(font);

	height = count * fm.lineSpacing() + (MARGIN*2);

	UMLClassifierListItem* theTemplate = 0;
	for ( theTemplate=list.first(); theTemplate != 0; theTemplate=list.next() ) {
		int textWidth = fm.width( theTemplate->toString() );
		width = textWidth>width ? textWidth : width;
	}

	width += (MARGIN*2);
	return QSize(width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::calculateSize() {
	if( !m_pObject )
		return;
	UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);

	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
	int fontHeight  = fm.lineSpacing();

	int lines = 1;//always have one line - for name

	if (m_bShowStereotype)
		lines++;

	int height = 0;
	//set the height of the concept
	if (m_bShowAttributes) {
		int numAtts = displayedAttributes();
		lines += numAtts;
		if(numAtts == 0)
			height += fontHeight / 2;  // no atts, so just add a bit of space
	}

	if (m_bShowOperations) {
		int numOps = displayedOperations();
		lines += numOps;
		if(numOps == 0)
			height += fontHeight / 2;  // no ops, so just add a bit of space
	}
	height += lines * fontHeight;

	//now set the width of the concept
	//set width to name to start with
	QString displayedName = getName();
	if (m_bShowPackage)
		displayedName.prepend( m_pObject->getPackage() + "::" );

	UMLWidget::FontType nameFont = (m_pObject->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
	int width = getFontMetrics(nameFont).boundingRect(displayedName).width();

	/* adjust width to stereotype if given */
	QString displayedStereotype = m_pObject->getStereotype();
	if (! displayedStereotype.isEmpty() ) {
		int w = getFontMetrics(FT_BOLD).boundingRect(displayedStereotype).width();
		if (w > width)
			width = w;
	}

	/* calculate width of the attributes */
	if (m_bShowAttributes) {
		UMLClassifierListItemList list = umlclass->getFilteredList(Uml::ot_Attribute);
		UMLClassifierListItem* a = 0;
		for (a = list.first(); a; a = list.next()) {
			if (m_bShowPublicOnly && a->getScope() != Uml::Public)
				continue;
			bool isStatic = a->getStatic();
			QFont font = UMLWidget::getFont();
			font.setUnderline(isStatic);
			font.setItalic(false);
			QFontMetrics fontMetrics(font);
			int w = fm.width(a->toString(m_ShowAttSigs));
			width = w > width?w:width;
		}
	}

	/* calculate width of the operations */
	if (m_bShowOperations) {
		UMLOperationList list(umlclass->getOpList());
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
	adjustUnselectedAssocs( getX(), getY() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::slotMenuSelection(int sel) {
	ListPopupMenu::Menu_Type mt = (ListPopupMenu::Menu_Type)sel;
	switch (mt) {
		case ListPopupMenu::mt_Attribute:
		case ListPopupMenu::mt_Operation:
		{
			UMLDoc *doc = UMLApp::app()->getDocument();
			Uml::Object_Type ot = ListPopupMenu::convert_MT_OT(mt);
			if (doc->createChildObject(m_pObject, ot))
				doc->setModified();
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

		case ListPopupMenu::mt_Show_Public_Only:
			toggleShowPublicOnly();
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
		default:
			break;
	}
	UMLWidget::slotMenuSelection(sel);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassWidget::updateSigs() {
	ClassifierWidget::updateSigs();
	if (m_bShowScope) {
		if (m_ShowAttSigs == Uml::st_NoSigNoScope)
			m_ShowAttSigs = Uml::st_NoSig;
		else if (m_ShowAttSigs == Uml::st_SigNoScope)
			m_ShowAttSigs = Uml::st_ShowSig;
	} else {
		if (m_ShowAttSigs == Uml::st_ShowSig)
			m_ShowAttSigs = Uml::st_SigNoScope;
		else if(m_ShowAttSigs == Uml::st_NoSig)
			m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
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
void ClassWidget::setAttSignature(Uml::Signature_Type sig) {
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

void ClassWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement( "classwidget" );
	UMLWidget::saveToXMI( qDoc, conceptElement );
	conceptElement.setAttribute( "showattributes", m_bShowAttributes );
	conceptElement.setAttribute( "showoperations", m_bShowOperations );
	conceptElement.setAttribute( "showpubliconly", m_bShowPublicOnly );
	conceptElement.setAttribute( "showattsigs", m_ShowAttSigs );
	conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
	conceptElement.setAttribute( "showpackage", m_bShowPackage );
	conceptElement.setAttribute( "showstereotype", m_bShowStereotype );
	conceptElement.setAttribute( "showscope", m_bShowScope );
	qElement.appendChild( conceptElement );
}

bool ClassWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	QString showatts = qElement.attribute( "showattributes", "1" );
	QString showops = qElement.attribute( "showoperations", "1" );
	QString showpubliconly = qElement.attribute( "showpubliconly", "0" );
	QString showattsigs = qElement.attribute( "showattsigs", "600" );
	QString showopsigs = qElement.attribute( "showopsigs", "600" );
	QString showpackage = qElement.attribute( "showpackage", "0" );
	QString showstereo = qElement.attribute( "showstereotype", "0" );
	QString showscope = qElement.attribute( "showscope", "0" );

	m_bShowAttributes = (bool)showatts.toInt();
	m_bShowOperations = (bool)showops.toInt();
	m_bShowPublicOnly = (bool)showpubliconly.toInt();
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
}

void ClassWidget::toggleShowStereotype()
{
	m_bShowStereotype = !m_bShowStereotype;
	updateSigs();
	calculateSize();
	update();
}
