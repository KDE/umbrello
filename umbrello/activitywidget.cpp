/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "activitywidget.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"

#include "inputdialog.h"
#include <klocale.h>
#include <qpainter.h>

ActivityWidget::ActivityWidget(UMLView * view, ActivityType activityType ) : UMLWidget(view, -1)
{
	m_ActivityType = activityType;
	UMLWidget::setBaseType( wt_Activity );
	calculateSize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ActivityWidget::~ActivityWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActivityWidget::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	switch ( m_ActivityType )
	{
		case Normal :
			p.setPen( UMLWidget::getLineColour() );
			if ( UMLWidget::getUseFillColour() ) {
				p.setBrush( UMLWidget::getFillColour() );
			}
			{
				QFontMetrics &fm = getFontMetrics(FT_NORMAL);
				int fontHeight  = fm.lineSpacing();
				//int middleX = w / 2;
				int textStartY = (h / 2) - (fontHeight / 2);
				p.drawRoundRect(offsetX, offsetY, w, h, (h * 60) / w, 60);
				p.setPen(black);
				p.setFont( UMLWidget::getFont() );
				p.drawText(offsetX + ACTIVITY_MARGIN, offsetY + textStartY, w - ACTIVITY_MARGIN * 2, fontHeight, AlignCenter, getName());
			}
			p.setPen( UMLWidget::getLineColour() );
			break;
		case Initial :
			p.setPen( UMLWidget::getLineColour() );
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			break;
		case End :
			p.setPen( UMLWidget::getLineColour() );
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			p.setBrush( white );
			p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
			break;
		case Branch :
			p.setPen( UMLWidget::getLineColour() );
			p.setBrush( UMLWidget::getFillColour() );
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
	if ( m_ActivityType == Normal ) {
		QFontMetrics &fm = getFontMetrics(FT_NORMAL);
		int fontHeight  = fm.lineSpacing();
		int textWidth = fm.width(getName());
		height = fontHeight;
		width = textWidth > ACTIVITY_WIDTH?textWidth:ACTIVITY_WIDTH;
		height = height > ACTIVITY_HEIGHT?height:ACTIVITY_HEIGHT;
		width += ACTIVITY_MARGIN * 2;
		height += ACTIVITY_MARGIN * 2;
	} else if ( m_ActivityType == Branch ) {
		width = height = 20;
	} else if ( m_ActivityType == Fork ) {
		width = 40;
		height = 4;
	}
	setSize(width, height);
}

void ActivityWidget::setName(QString strName) {
	m_Name = strName;
	calculateSize();
	adjustAssocs( getX(), getY() );
}

QString ActivityWidget::getName() const {
	return m_Name;
}

QString ActivityWidget::getDoc() const {
	return m_Doc;
}

void ActivityWidget::setDoc( QString doc ) {
	m_Doc = doc;
}

ActivityWidget::ActivityType ActivityWidget::getActivityType() const {
	return m_ActivityType;
}

void ActivityWidget::setActivityType( ActivityType activityType ) {
	m_ActivityType = activityType;
}

void ActivityWidget::slotMenuSelection(int sel) {
	bool done = false;

	bool ok = false;
	QString name = m_Name;

	switch( sel ) {
		case ListPopupMenu::mt_Rename:
			name = KInputDialog::getText( i18n("Enter Activity Name"), i18n("Enter the name of the new activity:"), m_Name, &ok );
			if( ok && name.length() > 0 )
				m_Name = name;
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

bool ActivityWidget::isActivity(WorkToolBar::ToolBar_Buttons tbb,
				ActivityType& resultType)
{
	bool status = true;
	switch (tbb) {
		case WorkToolBar::tbb_Initial_Activity:
			resultType = Initial;
			break;
		case WorkToolBar::tbb_Activity:
			resultType = Normal;
			break;
		case WorkToolBar::tbb_End_Activity:
			resultType = End;
			break;
		case WorkToolBar::tbb_Branch:
			resultType = Branch;
			break;
		case WorkToolBar::tbb_Fork:
			resultType = Fork;
			break;
		default:
			status = false;
			break;
	}
	return status;
}

void ActivityWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement activityElement = qDoc.createElement( "UML:ActivityWidget" );
	UMLWidget::saveToXMI( qDoc, activityElement );
	activityElement.setAttribute( "activityname", m_Name );
	activityElement.setAttribute( "documentation", m_Doc );
	activityElement.setAttribute( "activitytype", m_ActivityType );
	qElement.appendChild( activityElement );
}

bool ActivityWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	m_Name = qElement.attribute( "activityname", "" );
	m_Doc = qElement.attribute( "documentation", "" );
	QString type = qElement.attribute( "activitytype", "1" );
	m_ActivityType = (ActivityType)type.toInt();
	return true;
}


#include "activitywidget.moc"
