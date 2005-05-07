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
#include "associationwidget.h"
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

void ClassWidget::init() {
	m_pAssocWidget = NULL;
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

ClassWidget::~ClassWidget() {
	if (m_pAssocWidget)
		m_pAssocWidget->removeAssocClassLine();
}

int ClassWidget::displayedAttributes() {
	if (!m_bShowAttributes)
		return 0;
	return ClassifierWidget::displayedMembers(Uml::ot_Attribute);
}

void ClassWidget::draw(QPainter & p, int offsetX, int offsetY) {
	if (!m_pObject) {
		kdDebug() << "ClassWidget::draw(): m_pObject is NULL" << endl;
		return;
	}
	ClassifierWidget::draw(p, offsetX, offsetY);
	QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
	int fontHeight  = fm.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject->getFullyQualifiedName();
	} else {
		name = this -> getName();
	}

	p.setPen(QPen(black));
	const int textX = offsetX + MARGIN;
	const int textWidth = m_w - MARGIN * 2;
	if ( !m_bShowOperations && !m_bShowAttributes && !m_bShowStereotype ) {
		QFont font = UMLWidget::getFont();
		font.setBold( true );
		font.setItalic( m_pObject-> getAbstract() );
		p.setFont( font );
		p.drawText(textX, m_bodyOffsetY, textWidth, m_h, AlignCenter, name);
		font.setBold( false );
		font.setItalic( false );
		p.setFont( font );
	} else {
		QFont f( UMLWidget::getFont() );
		f.setBold( true );
		//FIXME why is underline sometimes true
		f.setUnderline( false );
		/* if no stereotype is given, we don't want to show the empty << >> */
		QString firstLine = m_pObject->getStereotype();
		bool showStereotype = (m_bShowStereotype && !firstLine.isEmpty() );
		if (!showStereotype) {
			firstLine = name;
			f.setItalic( m_pObject->getAbstract() );
		}
		p.setFont( f );
		p.drawText(textX, m_bodyOffsetY, textWidth, fontHeight, AlignCenter, firstLine);
		if (showStereotype) {
			f.setItalic( m_pObject -> getAbstract() );
			p.setFont( f );
			m_bodyOffsetY += fontHeight;
			p.drawText(textX, m_bodyOffsetY, textWidth, fontHeight, AlignCenter, name);
		}
		m_bodyOffsetY += fontHeight;
		f.setItalic( false );
		f.setBold( false );
		p.setFont( f );
		UMLWidget::draw(p, offsetX, offsetY);
		p.drawLine(offsetX, m_bodyOffsetY, offsetX + m_w - 1, m_bodyOffsetY);
	}

	p.setPen(QPen(black));

	if (m_bShowAttributes) {
		drawMembers(p, Uml::ot_Attribute, m_ShowAttSigs, textX,
			    m_bodyOffsetY, fontHeight);
	}//end if att

	int numAtts = displayedAttributes();
	if (m_bShowOperations) {
		QFont f = UMLWidget::getFont();
		int oStart = numAtts * fontHeight;
		m_bodyOffsetY += oStart;
		if (m_bShowAttributes) {
			UMLWidget::draw(p, offsetX, offsetY);
			p.drawLine(offsetX, m_bodyOffsetY, offsetX + m_w - 1, m_bodyOffsetY);
			p.setPen(QPen(black));
		}
		drawMembers(p, Uml::ot_Operation, m_ShowOpSigs, textX,
			    m_bodyOffsetY, fontHeight);
	}//end if op

	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}

void ClassWidget::calculateSize() {
	if( !m_pObject )
		return;
	int width = 0, height = 0;
	ClassifierWidget::computeBasicSize(width, height, m_bShowStereotype);

	if (m_bShowAttributes) {
		// calculate height of the attributes
		const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
		const int fontHeight = fm.lineSpacing();
		int numAtts = displayedAttributes();
		if (numAtts == 0)
			height += fontHeight / 2;  // no atts, so just add a bit of space
		else
			height += fontHeight * numAtts;

		// calculate width of the attributes
		UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);
		UMLClassifierListItemList list = umlclass->getFilteredList(Uml::ot_Attribute);
		for (UMLClassifierListItem *a = list.first(); a; a = list.next()) {
			if (m_bShowPublicOnly && a->getScope() != Uml::Public)
				continue;
			QFont font = UMLWidget::getFont();
			font.setUnderline(a->getStatic());
			font.setItalic(false);
			const QFontMetrics fontMetrics(font);
			const int w = fontMetrics.width(a->toString(m_ShowAttSigs));
			if (w > width)
				width = w;
		}
	}

	if (!m_bShowOperations && !m_bShowAttributes && !m_bShowStereotype) {
		height += ClassifierWidget::MARGIN * 2;
	}

	QSize templatesBoxSize = calculateTemplatesBoxSize();
	if (templatesBoxSize.width() != 0) {
		width += templatesBoxSize.width() / 2;
	}
	if (templatesBoxSize.height() != 0) {
		height += templatesBoxSize.height() - ClassifierWidget::MARGIN;
	}

	setSize(width, height);
	adjustUnselectedAssocs( getX(), getY() );
}

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

void ClassWidget::setShowAtts(bool _show) {
	m_bShowAttributes = _show;
	updateSigs();

	calculateSize();
	update();
}

void ClassWidget::setAttSignature(Uml::Signature_Type sig) {
	m_ShowAttSigs = sig;
	updateSigs();
	calculateSize();
	update();
}

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

void ClassWidget::setClassAssocWidget(AssociationWidget *assocwidget) {
	m_pAssocWidget = assocwidget;
	if (m_pAssocWidget == NULL)
		return;
	UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);
	UMLAssociation *umlassoc = assocwidget->getAssociation();
	if (umlassoc == NULL) {
		kdError() << "ClassWidget::setClassAssocWidget: "
			  << "cannot setClassAssoc() because UMLAssociation is NULL"
			  << endl;
		return;
	}
	umlclass->setClassAssoc(umlassoc);
}

AssociationWidget *ClassWidget::getClassAssocWidget() {
	return m_pAssocWidget;
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

