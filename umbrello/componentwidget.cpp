/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "componentwidget.h"
#include "componentwidgetdata.h"
#include "component.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>

ComponentWidget::ComponentWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData) : UMLWidget(view, o, pData) {
	m_pMenu = 0;
	if (m_pObject) {
		calculateSize();
		update();
	}
}

ComponentWidget::ComponentWidget(UMLView* view, UMLObject* o) : UMLWidget(view, o, new ComponentWidgetData(view->getOptionState() )) {
	init();
	setSize(100, 30);
	calculateSize();
	m_pData->setType(wt_Component);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentWidget::ComponentWidget(UMLView * view) : UMLWidget(view, new ComponentWidgetData(view->getOptionState() )) {
	init();
	setSize(100,30);
	m_pData->setType(wt_Component);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::init() {
	m_pMenu = 0;
	//set defaults from m_pView
	if (m_pView && m_pData) {
		//check to see if correct
		SettingsDlg::OptionState ops = m_pView->getOptionState();

		( (ComponentWidgetData*)m_pData ) -> m_bShowStereotype = ops.classState.showStereoType;
	}
	//maybe loading and this may not be set.
	if (m_pObject) {
		calculateSize();
		update();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentWidget::~ComponentWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::draw(QPainter & p, int offsetX, int offsetY) {
	p.setPen( m_pData->getLineColour() );
	if ( (static_cast<UMLComponent*>(m_pObject))->getExecutable() ) {
		QPen thickerPen = p.pen();
		thickerPen.setWidth(2);
		p.setPen(thickerPen);
	}
	if ( m_pData->getUseFillColor() ) {
		p.setBrush( m_pData->getFillColour() );
	} else {
		p.setBrush( m_pView->viewport()->backgroundColor() );
	}

	int w = width();
	int h = height();
	QFont font = m_pData->getFont();
	font.setBold(true);
	QFontMetrics fm = QFontMetrics(font);
	int fontHeight  = fm.lineSpacing();
	QString name = getName();
	QString stereotype = m_pObject->getStereotype();

	p.drawRect(offsetX + 2*COMPONENT_MARGIN, offsetY,
		   w - 2*COMPONENT_MARGIN, 3*COMPONENT_MARGIN + 2*fontHeight);
	p.drawRect(offsetX, offsetY + COMPONENT_MARGIN, COMPONENT_MARGIN*4, fontHeight);
	p.drawRect(offsetX, offsetY + COMPONENT_MARGIN*2 + fontHeight, COMPONENT_MARGIN*4, fontHeight);

	p.setPen( QPen(black) );
	p.setFont(font);

	if (stereotype != "") {
		p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h*0.5) - fontHeight,
			   w - (COMPONENT_MARGIN*4), fontHeight, AlignCenter, "<< " + stereotype + " >>");
	}

	int lines;
	if (stereotype != "") {
		lines = 2;
	} else {
		lines = 1;
	}

	if ( getData()->getIsInstance() ) {
		font.setUnderline(true);
		p.setFont(font);
		name = getData()->getInstanceName() + " : " + name;
	}

	if (lines == 1) {
		p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h*0.5) - (fontHeight*0.5),
			   w - (COMPONENT_MARGIN*4), fontHeight, AlignCenter, name );
	} else {
		p.drawText(offsetX + (COMPONENT_MARGIN*4), offsetY + (h*0.5),
			   w - (COMPONENT_MARGIN*4), fontHeight, AlignCenter, name );
	}

	if(m_bSelected) {
		drawSelected(&p, offsetX, offsetY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::calculateSize() {
	if ( !m_pData || !m_pObject) {
		return;
	}
	int width, height;

	QFont font = m_pData->getFont();
	font.setBold(true);   //use bold for all calculations
	font.setItalic(true);
	QFontMetrics fm = QFontMetrics( font );
	int fontHeight  = fm.lineSpacing();

	QString name = m_pObject->getName();
	if ( getData()->getIsInstance() ) {
		name = getData()->getInstanceName() + " : " + name;
	}

	width = fm.width(name);

	int tempWidth = 0;
	if(m_pObject->getStereotype() != "") {
		tempWidth = fm.width("<< " + m_pObject->getStereotype() + " >>");
	}
	width = tempWidth>width ? tempWidth : width;
	width += COMPONENT_MARGIN * 6;
	width = 70>width ? 70 : width; //minumin width of 70

	height = (2*fontHeight) + (COMPONENT_MARGIN * 3);

	setSize(width, height);
	adjustAssocs( (int)x(), (int)y() );//adjust assoc lines
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentWidget::setShowStereotype(bool _status) {
	((ComponentWidgetData*)m_pData)->m_bShowStereotype = _status;
	calculateSize();
	update();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ComponentWidget::activate(IDChangeLog* ChangeLog /* = 0 */) {
	bool status = UMLWidget::activate(ChangeLog);
	if(status) {
		calculateSize();
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ComponentWidget::getShowStereotype() {
	return ((ComponentWidgetData*)m_pData)->m_bShowStereotype;
}
