/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "activitywidget.h"
#include "activitywidgetdata.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"

#include <klineeditdlg.h>
#include <klocale.h>
#include <qpainter.h>

ActivityWidget::ActivityWidget(UMLView * view, UMLWidgetData* pData)
  : UMLWidget(view, pData) {}

ActivityWidget::ActivityWidget(UMLView * view, ActivityType activityType )
  : UMLWidget(view, new ActivityWidgetData(view->getOptionState() )) {
	static_cast<ActivityWidgetData *>( m_pData ) -> setActivityType( activityType );
	m_pData->setType(wt_Activity);
	calculateSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ActivityWidget::ActivityWidget(UMLView * view) : UMLWidget(view, new ActivityWidgetData(view->getOptionState() )) {
	m_pData->setType(wt_Activity);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ActivityWidget::~ActivityWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActivityWidget::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	ActivityType type = static_cast<ActivityWidgetData *>( m_pData )->getActivityType();
	switch ( type )
	{
		case Normal :
			p.setPen(m_pData->getLineColour());
			if(m_pData->getUseFillColor()) {
				p.setBrush(m_pData->getFillColour());
			}
			{
				QFontMetrics &fm = getFontMetrics(FT_NORMAL);
				int fontHeight  = fm.lineSpacing();
				//int middleX = w / 2;
				int textStartY = (h / 2) - (fontHeight / 2);
				p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
				p.setPen(black);
				p.setFont(m_pData->getFont());
				p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY, w - ACTIVITY_MARGIN * 2, fontHeight, AlignCenter, getName());
			}
			p.setPen(m_pData->getLineColour());
			break;
		case Initial :
			p.setPen(m_pData->getLineColour());
			p.setBrush(m_pData->getLineColour());
			p.drawEllipse( offsetX, offsetY, w, h );
			break;
		case End :
			p.setPen(m_pData->getLineColour());
			p.setBrush( m_pData->getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			p.setBrush( white );
			p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
			p.setBrush( m_pData->getLineColour() );
			p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
			break;
		case Branch :
			p.setPen(m_pData->getLineColour());
			p.setBrush(m_pData->getFillColour());
			{
				QPointArray array( 4 );
				array[ 0 ] = QPoint( offsetX + w / 2, offsetY );
				array[ 1 ] = QPoint( offsetX + w, offsetY  + h / 2 );
				array[ 2 ] = QPoint( offsetX + w / 2, offsetY + h );
				array[ 3 ] = QPoint( offsetX, offsetY + h / 2 );
				p.drawPolygon( array );
				p.drawPolyline( array );
			}
			break;
		case Fork :
			p.fillRect( offsetX, offsetY, width(), height(), QBrush( black ));
			break;
	}
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActivityWidget::calculateSize() {
	int width = 10, height = 10;
	ActivityType type = static_cast<ActivityWidgetData *>( m_pData ) -> getActivityType();
	if( type == Normal ) {
		QFontMetrics &fm = getFontMetrics(FT_NORMAL);
		int fontHeight  = fm.lineSpacing();
		int textWidth = fm.width(getName());
		height = fontHeight;
		width = textWidth > ACTIVITY_WIDTH?textWidth:ACTIVITY_WIDTH;
		height = height > ACTIVITY_HEIGHT?height:ACTIVITY_HEIGHT;
		width += ACTIVITY_MARGIN * 2;
		height += ACTIVITY_MARGIN * 2;
	} else if( type == Branch ) {
		width = height = 20;
	} else if( type == Fork ) {
		width = 40;
		height = 4;
	}
	setSize(width, height);
}

void ActivityWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}

void ActivityWidget::setName(QString strName) {
	static_cast<ActivityWidgetData*>(m_pData)->setName(strName);
	calculateSize();
	adjustAssocs( (int)x(), (int)y() );
}

QString ActivityWidget::getName() {
	return static_cast<ActivityWidgetData*>(m_pData)->getName();
}

QString ActivityWidget::getDoc() {
	return static_cast<ActivityWidgetData*>(m_pData)->getDoc();
}

void ActivityWidget::setDoc( QString doc ) {
	static_cast<ActivityWidgetData*>(m_pData)->setDoc(doc);
}

ActivityWidget::ActivityType ActivityWidget::getActivityType() {
	return static_cast<ActivityWidgetData*>(m_pData)->getActivityType();
}

void ActivityWidget::setActivityType( ActivityType activityType ) {
	static_cast<ActivityWidgetData*>(m_pData)->setActivityType(activityType);
}

void ActivityWidget::slotMenuSelection(int sel) {
	bool done = false;

	bool ok = false;
	QString name = getName();

	switch( sel ) {
		case ListPopupMenu::mt_Rename:
			name = KLineEditDlg::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), getName(), &ok );
			if( ok && name.length() > 0 )
				setName( name );
			done = true;
			break;

		case ListPopupMenu::mt_Properties:
			mouseDoubleClickEvent( 0 );
			done = true;
			break;
	}

	if( !done )
		UMLWidget::slotMenuSelection( sel );
}


void ActivityWidget::mouseDoubleClickEvent(QMouseEvent * /*me*/) {
	m_pView -> updateDocumentation( false );
	ActivityDialog dialog( m_pView, this );
	if( dialog.exec() && dialog.getChangesMade() ) {
		//put here anything that needs to be done for changes
		//nothing at the moment.
	}
	m_pView -> showDocumentation( this, true );
}

#include "activitywidget.moc"
