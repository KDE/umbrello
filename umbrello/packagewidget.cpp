/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "packagewidget.h"
#include "packagewidgetdata.h"
#include "package.h"
#include "umlview.h"
#include "umlobject.h"
#include "umlwidgetdata.h"
#include <kdebug.h>
#include <qpainter.h>

PackageWidget::PackageWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData) : UMLWidget(view, o, pData) {
	m_pMenu = 0;
	if (m_pObject) {
		calculateSize();
		update();
	}
}

PackageWidget::PackageWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o, new PackageWidgetData(view->getOptionState() )) {
	init();
	setSize(100, 30);
	calculateSize();
	m_pData->setType(wt_Package);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
PackageWidget::PackageWidget(UMLView * view) : UMLWidget(view, new PackageWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	m_pData->setType(wt_Package);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView && m_pData) {
		//check to see if correct
		SettingsDlg::OptionState ops = m_pView->getOptionState();

		( (PackageWidgetData*)m_pData ) -> setShowStereotype( ops.classState.showStereoType );
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
	p.setPen(m_pData->getLineColour());
	if(m_pData->getUseFillColor())
		p.setBrush(m_pData->getFillColour());
	else
		p.setBrush(m_pView -> viewport() -> backgroundColor());

	int w = width();
	int h = height();
	QFont font = m_pData->getFont();
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
	if ( !m_pData || !m_pObject) {
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
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void PackageWidget::setShowStereotype(bool _status) {
	((PackageWidgetData*)m_pData)->setShowStereotype( _status );
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
	return ((PackageWidgetData*)m_pData)->getShowStereotype();
}
