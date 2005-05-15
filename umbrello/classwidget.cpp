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
	if (m_bShowAccess) {
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
		if (m_bShowAccess)
			m_ShowAttSigs = Uml::st_NoSig;
		else
			m_ShowAttSigs = Uml::st_NoSigNoScope;
	}
	else if (m_bShowAccess)
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
		if (m_bShowAccess) {
			m_ShowAttSigs = Uml::st_NoSig;
		} else {
			m_ShowAttSigs = Uml::st_NoSigNoScope;
		}
	} else if (m_bShowAccess) {
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
	UMLClass *umlclass = static_cast<UMLClass*>(m_pObject);
	UMLAssociation *umlassoc = NULL;
	if (assocwidget)
		umlassoc = assocwidget->getAssociation();
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
	conceptElement.setAttribute( "showscope", m_bShowAccess );
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
	m_bShowAccess = (bool)showscope.toInt();

	return true;
}

