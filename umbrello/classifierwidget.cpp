 /*
  *  copyright (C) 2004
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

// own header
#include "classifierwidget.h"
// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>
// app includes
#include "classifier.h"
#include "operation.h"
#include "template.h"
#include "umlview.h"

ClassifierWidget::ClassifierWidget(UMLView * view, UMLClassifier *c, Uml::Widget_Type wt)
  : UMLWidget(view, c) {
	init(wt);
}

ClassifierWidget::~ClassifierWidget() {}

const int ClassifierWidget::MARGIN = 5;

void ClassifierWidget::init(Uml::Widget_Type wt) {
	UMLWidget::setBaseType(wt);

	const Settings::OptionState& ops = m_pView->getOptionState();
	m_bShowScope = ops.classState.showScope;
	m_bShowOperations = ops.classState.showOps;
	m_bShowPublicOnly = false;
	m_bShowPackage = ops.classState.showPackage;
	/* setShowOpSigs( ops.classState.showOpSig );
	  Cannot do that because we get "pure virtual method called". Open code:
	 */
	if( !ops.classState.showOpSig ) {
		if (m_bShowScope)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if (m_bShowScope)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;
}

void ClassifierWidget::updateSigs() {
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
	//To be done by inheriting classes:
	// calculateSize();
	// update();
}

bool ClassifierWidget::getShowOps() const {
	return m_bShowOperations;
}

void ClassifierWidget::setShowOps(bool _show) {
	m_bShowOperations = _show;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowOps() {
	m_bShowOperations = !m_bShowOperations;
	updateSigs();
	calculateSize();
	update();
}

bool ClassifierWidget::getShowPublicOnly() const {
	return m_bShowPublicOnly;
}

void ClassifierWidget::setShowPublicOnly(bool _status) {
	m_bShowPublicOnly = _status;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowPublicOnly() {
	m_bShowPublicOnly = !m_bShowPublicOnly;
	calculateSize();
	update();
}

bool ClassifierWidget::getShowScope() const {
	return m_bShowScope;
}

void ClassifierWidget::setShowScope(bool _scope) {
	m_bShowScope = _scope;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowScope() {
	m_bShowScope = !m_bShowScope;
	updateSigs();
	calculateSize();
	update();
}

Uml::Signature_Type ClassifierWidget::getShowOpSigs() const {
	return m_ShowOpSigs;
}

void ClassifierWidget::setShowOpSigs(bool _status) {
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

void ClassifierWidget::toggleShowOpSigs() {
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
}

bool ClassifierWidget::getShowPackage() const {
	return m_bShowPackage;
}

void ClassifierWidget::setShowPackage(bool _status) {
	m_bShowPackage = _status;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowPackage() {
	m_bShowPackage = !m_bShowPackage;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::setOpSignature(Uml::Signature_Type sig) {
	m_ShowOpSigs = sig;
	updateSigs();
	calculateSize();
	update();
}

int ClassifierWidget::displayedMembers(Uml::Object_Type ot) {
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	int count = 0;
	UMLClassifierListItemList list = c->getFilteredList(ot);
	for (UMLClassifierListItem *m = list.first(); m; m = list.next()) {
		if (!(m_bShowPublicOnly && m->getScope() != Uml::Public))
			count++;
	}
	return count;
}

int ClassifierWidget::displayedOperations() {
	if (!m_bShowOperations)
		return 0;
	return displayedMembers(Uml::ot_Operation);
}

QSize ClassifierWidget::calculateTemplatesBoxSize() {
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	UMLTemplateList list = c->getTemplateList();
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

	for (UMLTemplate *t = list.first(); t; t = list.next()) {
		int textWidth = fm.width( t->toString() );
		if (textWidth > width)
			width = textWidth;
	}

	width += (MARGIN*2);
	return QSize(width, height);
}

void ClassifierWidget::draw(QPainter & p, int offsetX, int offsetY) {
	UMLWidget::setPen(p);
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	QSize templatesBoxSize = calculateTemplatesBoxSize();
	m_bodyOffsetY = offsetY;
	if (templatesBoxSize.height() > 0)
		m_bodyOffsetY += templatesBoxSize.height() - MARGIN;
	m_w = width();
	if (templatesBoxSize.width() > 0)
		m_w -= templatesBoxSize.width() / 2;
	m_h = height();
	if (templatesBoxSize.height() > 0)
		m_h -= templatesBoxSize.height() - MARGIN;
	p.drawRect(offsetX, m_bodyOffsetY, m_w, m_h);

	//If there are any templates then draw them
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	UMLTemplateList tlist = c->getTemplateList();
	if ( tlist.count() > 0 ) {
		QFont font = UMLWidget::getFont();
		QFontMetrics fm(font);
		int fontHeight = fm.lineSpacing();
		UMLWidget::setPen(p);
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
		const int x = offsetX + width() - templatesBoxSize.width() + MARGIN;
		int y = offsetY + MARGIN;
		for ( UMLTemplate *t = tlist.first(); t; t = tlist.next() ) {
			QString text = t->toString();
			p.drawText(x, y, fontMetrics.width(text), fontHeight, AlignVCenter, text);
			y += fontHeight;
		}
	}
}

void ClassifierWidget::drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
				   int x, int y, int fontHeight) {
	QFont f = UMLWidget::getFont();
	f.setBold(false);
	UMLClassifier *c = static_cast<UMLClassifier*>(m_pObject);
	UMLClassifierListItemList list = c->getFilteredList(ot);
	for (UMLClassifierListItem *obj = list.first(); obj; obj = list.next()) {
		if (m_bShowPublicOnly && obj->getScope() != Uml::Public)
			continue;
		QString text = obj->toString(sigType);
		f.setItalic( obj->getAbstract() );
		f.setUnderline( obj->getStatic() );
		p.setFont( f );
		QFontMetrics fontMetrics(f);
		p.drawText(x, y, fontMetrics.width(text), fontHeight, AlignVCenter, text);
		f.setItalic(false);
		f.setUnderline(false);
		p.setFont(f);
		y += fontHeight;
	}
}

void ClassifierWidget::computeBasicSize(int &width, int &height,
					bool showStereotype /* =true */) {

	// set the height of the concept
	{
		height = 0;
		const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
		const int fontHeight = fm.lineSpacing();
		int lines = 1;  // always have one line - for name
		if (showStereotype)
			lines++;
		const int numOps = displayedOperations();
		if (numOps == 0)
			height += fontHeight / 2;  // no ops, so just add a bit of space
		else
			lines += numOps;
		height += lines * fontHeight;
	}

	// set the width of the concept
	{
		// set width to name to start with
		QString displayName = getName();
		if (m_bShowPackage)
			displayName.prepend( m_pObject->getPackage() + "." );

		UMLClassifier *uc = static_cast<UMLClassifier*>(m_pObject);
		const UMLWidget::FontType f(uc->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
		width = getFontMetrics(f).boundingRect(displayName).width();

		// adjust width to stereotype if given
		if (showStereotype) {
			const QString& stereo = m_pObject->getStereotype();
			const int w = getFontMetrics(FT_BOLD).boundingRect(stereo).width();
			if (w > width)
				width = w;
		}

		// calculate width of the operations
		if (m_bShowOperations) {
			UMLOperationList list(uc->getOpList());
			for (UMLOperation* op = list.first(); op; op = list.next()) {
				if (m_bShowPublicOnly && op->getScope() != Uml::Public)
					continue;
				QFont font = UMLWidget::getFont();
				font.setUnderline( op->getStatic() );
				font.setItalic( op->getAbstract() );
				const QFontMetrics fontMetrics(font);
				const int w = fontMetrics.width( op->toString(m_ShowOpSigs) );
				if (w > width)
					width = w;
			}
		}

		//allow for width margin
		width += MARGIN * 2;
	}
}

bool ClassifierWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	return status;
}

