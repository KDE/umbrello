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

#include "packagewidget.h"
#include "package.h"
#include "umlview.h"
#include "umlobject.h"
#include <kdebug.h>
#include <qpainter.h>

PackageWidget::PackageWidget(UMLView * view, UMLPackage *o) : UMLWidget(view, o) {
	init();
	setSize(100, 30);
	calculateSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::init() {
	UMLWidget::setBaseType(Uml::wt_Package);
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView) {
		//check to see if correct
		const Settings::OptionState& ops = m_pView->getOptionState();
		m_bShowStereotype = ops.classState.showStereoType;
	}
	//maybe loading and this may not be set.
	if (m_pObject) {
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
PackageWidget::~PackageWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::draw(QPainter & p, int offsetX, int offsetY) {
	UMLWidget::draw(p, offsetX, offsetY);
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	int w = width();
	int h = height();
	QFont font = UMLWidget::getFont();
	font.setBold(true);
	//FIXME italic is true when a package is first created until you click elsewhere, not sure why
	font.setItalic(false);
	QFontMetrics &fm = getFontMetrics(FT_BOLD);
	int fontHeight  = fm.lineSpacing();
	QString name = getName();

	p.drawRect(offsetX, offsetY, 50, fontHeight);
	p.drawRect(offsetX, offsetY + fontHeight - 1, w, h - fontHeight);

	p.setPen( QPen(black) );
	p.setFont(font);

	int lines = 1;
	if (m_pObject != NULL) {
		QString stereotype = m_pObject->getStereotype();
		if (!stereotype.isEmpty()) {
			p.drawText(offsetX, offsetY + fontHeight + PACKAGE_MARGIN,
				   w, fontHeight, AlignCenter, stereotype);
			lines = 2;
		}
	}

	p.drawText(offsetX, offsetY + (fontHeight*lines) + PACKAGE_MARGIN,
		   w, fontHeight, AlignCenter, name );

	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::calculateSize() {
	if ( !m_pObject ) {
		return;
	}
	int width, height;

	QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
	int fontHeight  = fm.lineSpacing();

	int lines;
	if (!m_pObject->getStereotype().isEmpty()) {
		lines = 2;
	} else {
		lines = 1;
	}

	width = fm.width( m_pObject->getName() );

	int tempWidth = 0;
	if(!m_pObject->getStereotype().isEmpty()) {
		tempWidth = fm.width(m_pObject->getStereotype());
	}
	width = tempWidth>width ? tempWidth : width;
	width += PACKAGE_MARGIN * 2;
	width = 70>width ? 70 : width; //minumin width of 70

	height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

	setSize(width, height);
	adjustAssocs( getX(), getY() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::setShowStereotype(bool _status) {
	m_bShowStereotype = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool PackageWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if(status) {
		calculateSize();
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool PackageWidget::getShowStereotype() {
	return m_bShowStereotype;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("packagewidget");
	UMLWidget::saveToXMI(qDoc, conceptElement);
	conceptElement.setAttribute("showstereotype", m_bShowStereotype);
	qElement.appendChild(conceptElement);
}

bool PackageWidget::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidget::loadFromXMI(qElement) ) {
		return false;
	}
	QString showstereo = qElement.attribute("showstereotype", "0");
	m_bShowStereotype = (bool)showstereo.toInt();
	return true;
}

