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

PackageWidget::PackageWidget(UMLView * view, UMLObject * o) : UMLWidget(view, o) {
	init();
	setSize(100, 30);
	calculateSize();
	UMLWidget::setBaseType(wt_Package);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
PackageWidget::PackageWidget(UMLView * view) : UMLWidget(view) {
	init();
	setSize(100,30);
	UMLWidget::setBaseType(wt_Package);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView) {
		//check to see if correct
		const SettingsDlg::OptionState& ops = m_pView->getOptionState();
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
	p.setPen( UMLWidget::getLineColour() );
	if ( UMLWidget::getUseFillColour() )
		p.setBrush( UMLWidget::getFillColour() );
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	int w = width();
	int h = height();
	QFont font = UMLWidget::getFont();
	font.setBold(true);
	QFontMetrics &fm = getFontMetrics(FT_BOLD);
	int fontHeight  = fm.lineSpacing();
	QString name = getName();
	QString stereotype = m_pObject->getStereotype();

	p.drawRect(offsetX, offsetY, 50, fontHeight);
	p.drawRect(offsetX, offsetY + fontHeight - 1, w, h - fontHeight);

	p.setPen( QPen(black) );
	p.setFont(font);

	if (stereotype != "") {
		p.drawText(offsetX, offsetY + fontHeight + PACKAGE_MARGIN,
			   w, fontHeight, AlignCenter, "<< " + stereotype + " >>");
	}

	int lines;
	if (stereotype != "") {
		lines = 2;
	} else {
		lines = 1;
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
	if (m_pObject->getStereotype() != "") {
		lines = 2;
	} else {
		lines = 1;
	}

	width = fm.width( m_pObject->getName() );

	int tempWidth = 0;
	if(m_pObject->getStereotype() != "") {
		tempWidth = fm.width("<< " + m_pObject->getStereotype() + " >>");
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
bool PackageWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("packagewidget");
	bool status = UMLWidget::saveToXMI(qDoc, conceptElement);
	conceptElement.setAttribute("showstereotype", m_bShowStereotype);
	qElement.appendChild(conceptElement);
	return status;
}

bool PackageWidget::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidget::loadFromXMI(qElement) ) {
		return false;
	}
	QString showstereo = qElement.attribute("showstereotype", "0");
	m_bShowStereotype = (bool)showstereo.toInt();
	return true;
}

