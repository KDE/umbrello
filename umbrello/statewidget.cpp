/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klineeditdlg.h>
#include <klocale.h>
#include <kdebug.h>
#include "statewidgetdata.h"
#include "umlwidget.h"
#include "umlview.h"
#include "dialogs/statedialog.h"
#include "statewidget.h"
#include "listpopupmenu.h"

StateWidget::StateWidget(UMLView * view, UMLWidgetData* pData) : UMLWidget(view, pData) {}

StateWidget::StateWidget(UMLView * view, StateType stateType ) : UMLWidget(view, new StateWidgetData(view->getOptionState() )) {
	static_cast<StateWidgetData *>( m_pData ) -> setStateType( stateType );
	m_pData->setType(wt_State);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
StateWidget::StateWidget(UMLView * view) : UMLWidget(view, new StateWidgetData(view->getOptionState() )) {
	m_pData->setType(wt_State);
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
StateWidget::~StateWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void StateWidget::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	StateType type = static_cast<StateWidgetData *>( m_pData ) -> getStateType();
	switch (type)
	{
		case Normal :
			p.setPen(m_pData->getLineColour());
			if(m_pData->getUseFillColor())
				p.setBrush(m_pData->getFillColour());
			{
				QFontMetrics fm = QFontMetrics( m_pData -> getFont() );
				int fontHeight  = fm.lineSpacing();
				int textStartY = (h / 2) - (fontHeight / 2);
				QStringList list = static_cast<StateWidgetData *>( m_pData ) -> getActivityList();
				int count = list.count();
				if( count == 0 ) {
					p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
					p.setPen(black);
					p.setFont(m_pData -> getFont());
					p.drawText(offsetX + STATE_MARGIN, offsetY + textStartY, w - STATE_MARGIN * 2, fontHeight, AlignCenter, getName());
					p.setPen(m_pData->getLineColour());
				} else {
					p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
					textStartY = offsetY + STATE_MARGIN;
					p.setPen(black);
					QFont font = m_pData -> getFont();
					font.setBold( true );
					p.setFont( font );
					p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2,
						   fontHeight, AlignCenter, getName());
					font.setBold( false );
					p.setFont( font );
					p.setPen(m_pData->getLineColour());
					int linePosY = textStartY + fontHeight;
					for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
						textStartY += fontHeight;
						p.drawLine( offsetX, linePosY, offsetX + w - 1, linePosY );
						p.setPen(black);
						p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2 - 1,
							   fontHeight, AlignCenter, *it);
						p.setPen(m_pData->getLineColour());
						linePosY += fontHeight;
					}//end for
				}//end else
			}
			break;
		case Initial :
			p.setPen(m_pData->getLineColour());
			p.setBrush( m_pData->getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			break;
		default :
			p.setPen(m_pData->getLineColour());
			p.setBrush( m_pData->getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			p.setBrush( white );
			p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
			p.setBrush( m_pData->getLineColour() );
			p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
			break;
	}
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void StateWidget::calculateSize() {
	int width = 10, height = 10;
	if( static_cast<StateWidgetData*>(m_pData)->getStateType() == Normal ) {
		QFont font = m_pData -> getFont();
		font.setBold( true );//use bold for all calculations
		QFontMetrics fm = QFontMetrics( font );
		int fontHeight  = fm.lineSpacing();
		int textWidth = fm.width(getName());
		QStringList list = static_cast<StateWidgetData *>( m_pData ) -> getActivityList();
		int count = list.count();
		height = fontHeight;
		if( count > 0 ) {
			height = fontHeight * ( count + 1);
			for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
				int w = fm.width( *it );
				if( w > textWidth )
					textWidth = w;
			}//end for
		}//end if
		width = textWidth > STATE_WIDTH?textWidth:STATE_WIDTH;
		height = height > STATE_HEIGHT?height:STATE_HEIGHT;
		width += STATE_MARGIN * 2;
		height += STATE_MARGIN * 2;
	}
	setSize(width, height);
}

void StateWidget::synchronizeData() {
	//Nothing to synchronize
	UMLWidget::synchronizeData();
}

void StateWidget::setName(QString strName) {
	static_cast<StateWidgetData *>( m_pData ) -> setName( strName );
	calculateSize();
	adjustAssocs( (int)x(), (int)y() );
}

QString StateWidget::getName() {
	return static_cast<StateWidgetData *>( m_pData ) -> getName();
}

QString StateWidget::getDoc() {
	return static_cast<StateWidgetData *>( m_pData ) -> getDoc();
}

void StateWidget::setDoc( QString doc ) {
	static_cast<StateWidgetData *>( m_pData ) -> setDoc( doc );
}

StateWidget::StateType StateWidget::getStateType() {
	return static_cast<StateWidgetData *>( m_pData ) -> getStateType();
}

void StateWidget::setStateType( StateType stateType ) {
	static_cast<StateWidgetData *>( m_pData ) -> setStateType( stateType );
}

void StateWidget::slotMenuSelection(int sel) {
	bool done = false;
	bool ok = false;
	QString name = getName();

	switch( sel ) {
		case ListPopupMenu::mt_Rename:
			name = KLineEditDlg::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), getName(), &ok );
			if( ok && name.length() > 0 )
				setName( name );
			done = true;
			break;

		case ListPopupMenu::mt_Properties:
			mouseDoubleClickEvent( 0 );
			done = true;
			break;
		case ListPopupMenu::mt_New_Activity:
			name = KLineEditDlg::getText( i18n("Enter Activity"), i18n("Enter the name of the new activity:"), i18n("new activity"), &ok );
			if( ok && name.length() > 0 )
				addActivity( name );
			done = true;
			break;
	}

	if( !done )
		UMLWidget::slotMenuSelection( sel );
}

void StateWidget::mouseDoubleClickEvent(QMouseEvent * /*me*/) {
	m_pView -> updateDocumentation( false );
	StateDialog dialog( m_pView, this );
	if( dialog.exec() && dialog.getChangesMade() ) {
		//put here anything that needs to be done for changes
		//nothing at the moment.
	}
	m_pView -> showDocumentation( this, true );
}

bool StateWidget::addActivity( QString activity ) {
	bool result = static_cast<StateWidgetData *>( m_pData ) -> addActivity( activity );
	if( result ) {
		calculateSize();
	}
	return result;
}

bool StateWidget::removeActivity( QString activity ) {
	bool result = static_cast<StateWidgetData *>( m_pData ) -> removeActivity( activity );
	if( result ) {
		calculateSize();
	}
	return result;
}

void StateWidget::setActivities( QStringList & list ) {
	static_cast<StateWidgetData *>( m_pData ) -> setActivities( list );
	calculateSize();
}

QStringList & StateWidget::getActivityList() {
	return static_cast<StateWidgetData *>( m_pData ) -> getActivityList();
}

bool StateWidget::renameActivity( QString activity, QString newName ) {
	return static_cast<StateWidgetData *>( m_pData ) -> renameActivity( activity, newName );
}






#include "statewidget.moc"
