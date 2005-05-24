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
#include "associationwidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"

ClassifierWidget::ClassifierWidget(UMLView * view, UMLClassifier *c)
  : UMLWidget(view, c) {
	init();
	if (c != NULL && c->isInterface()) {
		WidgetBase::setBaseType(Uml::wt_Interface);
		m_bShowStereotype = true;
		m_bShowAttributes = false;
		updateSigs();
	}
}

ClassifierWidget::~ClassifierWidget() {}

const int ClassifierWidget::MARGIN = 5;
const int ClassifierWidget::CIRCLE_SIZE = 30;

void ClassifierWidget::init() {
	WidgetBase::setBaseType(Uml::wt_Class);

	const Settings::OptionState& ops = m_pView->getOptionState();
	m_bShowAccess = ops.classState.showScope;
	m_bShowOperations = ops.classState.showOps;
	m_bShowPublicOnly = false;
	m_bShowPackage = ops.classState.showPackage;
	m_ShowAttSigs = Uml::st_ShowSig;
	/* setShowOpSigs( ops.classState.showOpSig );
	  Cannot do that because we get "pure virtual method called". Open code:
	 */
	if( !ops.classState.showOpSig ) {
		if (m_bShowAccess)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;

	} else if (m_bShowAccess)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;

	m_bShowAttributes = ops.classState.showAtts;
	m_bShowStereotype = ops.classState.showStereoType;
	setShowAttSigs( ops.classState.showAttSig );
	m_pAssocWidget = NULL;
	m_bDrawAsCircle = false;
}

void ClassifierWidget::updateSigs() {
	//turn on scope
	if (m_bShowAccess) {
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

bool ClassifierWidget::getShowStereotype() const {
	return m_bShowStereotype;
}

void ClassifierWidget::setShowStereotype(bool _status) {
	m_bShowStereotype = _status;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowStereotype()
{
	m_bShowStereotype = !m_bShowStereotype;
	updateSigs();
	calculateSize();
	update();
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
	return m_bShowAccess;
}

void ClassifierWidget::setShowScope(bool _scope) {
	m_bShowAccess = _scope;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowScope() {
	m_bShowAccess = !m_bShowAccess;
	updateSigs();
	calculateSize();
	update();
}

Uml::Signature_Type ClassifierWidget::getShowOpSigs() const {
	return m_ShowOpSigs;
}

void ClassifierWidget::setShowOpSigs(bool _status) {
	if( !_status ) {
		if (m_bShowAccess)
			m_ShowOpSigs = Uml::st_NoSig;
		else
			m_ShowOpSigs = Uml::st_NoSigNoScope;
	} else if (m_bShowAccess)
		m_ShowOpSigs = Uml::st_ShowSig;
	else
		m_ShowOpSigs = Uml::st_SigNoScope;
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowOpSigs() {
	if (m_ShowOpSigs == Uml::st_ShowSig || m_ShowOpSigs == Uml::st_SigNoScope) {
		if (m_bShowAccess) {
			m_ShowOpSigs = Uml::st_NoSig;
		} else {
			m_ShowOpSigs = Uml::st_NoSigNoScope;
		}
	} else if (m_bShowAccess) {
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

void ClassifierWidget::setShowAtts(bool _show) {
	m_bShowAttributes = _show;
	updateSigs();

	calculateSize();
	update();
}

void ClassifierWidget::setAttSignature(Uml::Signature_Type sig) {
	m_ShowAttSigs = sig;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::setShowAttSigs(bool _status) {
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

void ClassifierWidget::toggleShowAtts()
{
	m_bShowAttributes = !m_bShowAttributes;
	updateSigs();
	calculateSize();
	update();
}

void ClassifierWidget::toggleShowAttSigs()
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

int ClassifierWidget::displayedMembers(Uml::Object_Type ot) {
	int count = 0;
	UMLClassifierListItemList list = getClassifier()->getFilteredList(ot);
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

void ClassifierWidget::calculateSize() {
	if( !m_pObject )
		return;
	if (m_bDrawAsCircle) {
		calculateAsCircleSize();
		return;
	}
	int width = 0, height = 0;
	computeBasicSize(width, height, m_bShowStereotype);

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
		UMLClassifierListItemList list = getClassifier()->getFilteredList(Uml::ot_Attribute);
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

void ClassifierWidget::slotMenuSelection(int sel) {
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

QSize ClassifierWidget::calculateTemplatesBoxSize() {
	UMLTemplateList list = getClassifier()->getTemplateList();
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

int ClassifierWidget::displayedAttributes() {
	if (!m_bShowAttributes)
		return 0;
	return displayedMembers(Uml::ot_Attribute);
}

void ClassifierWidget::setClassAssocWidget(AssociationWidget *assocwidget) {
	m_pAssocWidget = assocwidget;
	UMLAssociation *umlassoc = NULL;
	if (assocwidget)
		umlassoc = assocwidget->getAssociation();
	getClassifier()->setClassAssoc(umlassoc);
}

AssociationWidget *ClassifierWidget::getClassAssocWidget() {
	return m_pAssocWidget;
}

UMLClassifier *ClassifierWidget::getClassifier() {
	return static_cast<UMLClassifier*>(m_pObject);
}

void ClassifierWidget::draw(QPainter & p, int offsetX, int offsetY) {
	UMLWidget::setPen(p);
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	else
		p.setBrush( m_pView->viewport()->backgroundColor() );

	if (m_bDrawAsCircle) {
		drawAsCircle(p, offsetX, offsetY);
		return;
	}

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

	QFont font = UMLWidget::getFont();
	font.setUnderline(false);
	font.setItalic(false);
	QFontMetrics fm = UMLWidget::getFontMetrics(UMLWidget::FT_NORMAL);
	const int fontHeight = fm.lineSpacing();

	//If there are any templates then draw them
	UMLTemplateList tlist = getClassifier()->getTemplateList();
	if ( tlist.count() > 0 ) {
		UMLWidget::setPen(p);
		QPen pen = p.pen();
		pen.setStyle(DotLine);
		p.setPen(pen);
		p.drawRect( offsetX + width() - templatesBoxSize.width(), offsetY,
			    templatesBoxSize.width(), templatesBoxSize.height() );
		p.setPen( QPen(black) );
		font.setBold(false);
		p.setFont(font);
		const int x = offsetX + width() - templatesBoxSize.width() + MARGIN;
		int y = offsetY + MARGIN;
		for ( UMLTemplate *t = tlist.first(); t; t = tlist.next() ) {
			QString text = t->toString();
			p.drawText(x, y, fm.width(text), fontHeight, AlignVCenter, text);
			y += fontHeight;
		}
	}

	const int textX = offsetX + MARGIN;
	const int textWidth = m_w - MARGIN * 2;

	p.setPen(QPen(black));

	// draw stereotype
	font.setBold(true);
	QString stereo = m_pObject->getStereotype();
	/* if no stereotype is given we don't want to show the empty << >> */
	const bool showStereotype = (m_bShowStereotype && !stereo.isEmpty());
	const bool showNameOnly = (!m_bShowOperations && !m_bShowAttributes && !showStereotype);
	int nameHeight = fontHeight;
	if (showNameOnly) {
		nameHeight = m_h;
	} else if (showStereotype) {
		p.setFont(font);
		p.drawText(textX, m_bodyOffsetY, textWidth, fontHeight, AlignCenter, stereo);
		m_bodyOffsetY += fontHeight;
	}

	// draw name
	QString name;
	if (m_bShowPackage) {
		name = m_pObject->getFullyQualifiedName();
	} else {
		name = this->getName();
	}
	font.setItalic( m_pObject->getAbstract() );
	p.setFont(font);
	p.drawText(textX, m_bodyOffsetY, textWidth, nameHeight, AlignCenter, name);
	if (!showNameOnly) {
		m_bodyOffsetY += fontHeight;
		UMLWidget::setPen(p);
		p.drawLine(offsetX, m_bodyOffsetY, offsetX + m_w - 1, m_bodyOffsetY);
		p.setPen(QPen(black));
	}
	font.setBold(false);
	font.setItalic(false);
	p.setFont(font);

	// draw attributes
	if (m_bShowAttributes) {
		drawMembers(p, Uml::ot_Attribute, m_ShowAttSigs, textX,
			    m_bodyOffsetY, fontHeight);
	}

	// draw operations
	int numAtts = displayedAttributes();
	if (m_bShowOperations) {
		int oStart = numAtts * fontHeight;
		m_bodyOffsetY += oStart;
		if (m_bShowAttributes) {
			UMLWidget::setPen(p);
			p.drawLine(offsetX, m_bodyOffsetY, offsetX + m_w - 1, m_bodyOffsetY);
			p.setPen(QPen(black));
		}
		drawMembers(p, Uml::ot_Operation, m_ShowOpSigs, textX,
			    m_bodyOffsetY, fontHeight);
	}

	if (m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}

void ClassifierWidget::drawAsCircle(QPainter& p, int offsetX, int offsetY) {
	int w = width();

	const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
	const int fontHeight  = fm.lineSpacing();
	QString name;
	if ( m_bShowPackage ) {
		name = m_pObject->getFullyQualifiedName();
	} else {
		name = this -> getName();
	}

	p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
	p.setPen( QPen(black) );

	QFont font = UMLWidget::getFont();
	p.setFont(font);
	p.drawText(offsetX, offsetY + CIRCLE_SIZE, w, fontHeight, AlignCenter, name);

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}

void ClassifierWidget::calculateAsCircleSize() {
	QFontMetrics &fm = getFontMetrics(FT_ITALIC_UNDERLINE);
	int fontHeight = fm.lineSpacing();

	int height = CIRCLE_SIZE + fontHeight;

	int width;
	if ( m_bShowPackage ) {
		width = fm.width(m_pObject->getPackage() + "." + getName());
	} else {
		width = fm.width(getName());
	}
	width = width<CIRCLE_SIZE ? CIRCLE_SIZE : width;

	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}

void ClassifierWidget::drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
				   int x, int y, int fontHeight) {
	QFont f = UMLWidget::getFont();
	f.setBold(false);
	UMLClassifierListItemList list = getClassifier()->getFilteredList(ot);
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

		UMLClassifier *uc = getClassifier();
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

void ClassifierWidget::setDrawAsCircle(bool drawAsCircle) {
	m_bDrawAsCircle = drawAsCircle;
	calculateSize();
	update();
}

bool ClassifierWidget::getDrawAsCircle() const {
	return m_bDrawAsCircle;
}

void ClassifierWidget::toggleDrawAsCircle() {
	m_bDrawAsCircle = !m_bDrawAsCircle;
	updateSigs();
	calculateSize();
	update();
}

bool ClassifierWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if (status) {
		calculateSize();
	}
	return status;
}

void ClassifierWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement) {
	QDomElement conceptElement;
	if (getClassifier()->isInterface())
		conceptElement = qDoc.createElement("interfacewidget");
	else
		conceptElement = qDoc.createElement("classwidget");
	UMLWidget::saveToXMI( qDoc, conceptElement );
	conceptElement.setAttribute( "showoperations", m_bShowOperations );
	conceptElement.setAttribute( "showpubliconly", m_bShowPublicOnly );
	conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
	conceptElement.setAttribute( "showpackage", m_bShowPackage );
	conceptElement.setAttribute( "showstereotype", m_bShowStereotype );
	conceptElement.setAttribute( "showscope", m_bShowAccess );
	if (! getClassifier()->isInterface()) {
		conceptElement.setAttribute("showattributes", m_bShowAttributes);
		conceptElement.setAttribute("showattsigs", m_ShowAttSigs);
	}
	qElement.appendChild( conceptElement );
}

bool ClassifierWidget::loadFromXMI(QDomElement & qElement) {
	if (!UMLWidget::loadFromXMI(qElement))
		return false;
	QString showatts = qElement.attribute( "showattributes", "0" );
	QString showops = qElement.attribute( "showoperations", "1" );
	QString showpubliconly = qElement.attribute( "showpubliconly", "0" );
	QString showattsigs = qElement.attribute( "showattsigs", "600" );
	QString showopsigs = qElement.attribute( "showopsigs", "600" );
	QString showpackage = qElement.attribute( "showpackage", "0" );
	QString showstereo = qElement.attribute( "showstereotype", "1" );
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

