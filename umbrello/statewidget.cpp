/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "statewidget.h"

#include <kdeversion.h>
#if !KDE_IS_VERSION(3,1,90)
# include <klineeditdlg.h>  //deprecated
# define KInputDialog       KLineEditDlg
#else
# include <kinputdialog.h>
#endif
#include <klocale.h>
#include <kdebug.h>
#include "umlwidget.h"
#include "umlview.h"
#include "dialogs/statedialog.h"
#include "listpopupmenu.h"

StateWidget::StateWidget(UMLView * view, StateType stateType) : UMLWidget(view) {
	UMLWidget::setBaseType(wt_State);
	m_StateType = stateType;
	m_Name = "State";
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
StateWidget::StateWidget(UMLView * view) : UMLWidget(view) {
	UMLWidget::setBaseType(wt_State);
	m_StateType = StateWidget::Normal;
	m_Name = "State";
	calculateSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
StateWidget::~StateWidget() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void StateWidget::draw(QPainter & p, int offsetX, int offsetY) {
	int w = width();
	int h = height();
	switch (m_StateType)
	{
		case Normal :
			p.setPen(UMLWidget::getLineColour());
			if(UMLWidget::getUseFillColour())
				p.setBrush(UMLWidget::getFillColour());
			{
				QFontMetrics &fm = getFontMetrics(FT_NORMAL);
				int fontHeight  = fm.lineSpacing();
				int textStartY = (h / 2) - (fontHeight / 2);
				int count = m_Activities.count();
				if( count == 0 ) {
					p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
					p.setPen(black);
					p.setFont(UMLWidget::getFont());
					p.drawText(offsetX + STATE_MARGIN, offsetY + textStartY, w - STATE_MARGIN * 2, fontHeight, AlignCenter, getName());
					p.setPen(UMLWidget::getLineColour());
				} else {
					p.drawRoundRect(offsetX, offsetY, w, h, (h*40)/w, (w*40)/h);
					textStartY = offsetY + STATE_MARGIN;
					p.setPen(black);
					QFont font = UMLWidget::getFont();
					font.setBold( true );
					p.setFont( font );
					p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2,
						   fontHeight, AlignCenter, getName());
					font.setBold( false );
					p.setFont( font );
					p.setPen(UMLWidget::getLineColour());
					int linePosY = textStartY + fontHeight;
					for( QStringList::Iterator it = m_Activities.begin(); it != m_Activities.end(); ++it ) {
						textStartY += fontHeight;
						p.drawLine( offsetX, linePosY, offsetX + w - 1, linePosY );
						p.setPen(black);
						p.drawText(offsetX + STATE_MARGIN, textStartY, w - STATE_MARGIN * 2 - 1,
							   fontHeight, AlignCenter, *it);
						p.setPen(UMLWidget::getLineColour());
						linePosY += fontHeight;
					}//end for
				}//end else
			}
			break;
		case Initial :
			p.setPen(UMLWidget::getLineColour());
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			break;
		default :
			p.setPen(UMLWidget::getLineColour());
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX, offsetY, w, h );
			p.setBrush( white );
			p.drawEllipse( offsetX + 1, offsetY + 1, w - 2, h - 2 );
			p.setBrush( UMLWidget::getLineColour() );
			p.drawEllipse( offsetX + 3, offsetY + 3, w - 6, h - 6 );
			break;
	}
	if(m_bSelected)
		drawSelected(&p, offsetX, offsetY);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void StateWidget::calculateSize() {
	int width = 10, height = 10;
	if ( m_StateType == Normal ) {
		QFontMetrics &fm = getFontMetrics(FT_BOLD);
		int fontHeight  = fm.lineSpacing();
		int textWidth = fm.width(getName());
		int count = m_Activities.count();
		height = fontHeight;
		if( count > 0 ) {
			height = fontHeight * ( count + 1);
			for( QStringList::Iterator it = m_Activities.begin(); it != m_Activities.end(); ++it ) {
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

void StateWidget::setName(QString strName) {
	m_Name = strName;
	calculateSize();
	adjustAssocs( getX(), getY() );
}

QString StateWidget::getName() const {
	return m_Name;
}

QString StateWidget::getDoc() const {
	return m_Doc;
}

void StateWidget::setDoc( QString doc ) {
	m_Doc = doc;
}

StateWidget::StateType StateWidget::getStateType() const {
	return m_StateType;
}

void StateWidget::setStateType( StateType stateType ) {
	m_StateType = stateType;
}

void StateWidget::slotMenuSelection(int sel) {
	bool done = false;
	bool ok = false;
	QString name = getName();

	switch( sel ) {
		case ListPopupMenu::mt_Rename:
			name = KInputDialog::getText( i18n("Enter State Name"), i18n("Enter the name of the new state:"), getName(), &ok );
			if( ok && name.length() > 0 )
				setName( name );
			done = true;
			break;

		case ListPopupMenu::mt_Properties:
			mouseDoubleClickEvent( 0 );
			done = true;
			break;
		case ListPopupMenu::mt_New_Activity:
			name = KInputDialog::getText( i18n("Enter Activity"), i18n("Enter the name of the new activity:"), i18n("new activity"), &ok );
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
	m_Activities.append( activity );
	calculateSize();
	return true;
}

bool StateWidget::removeActivity( QString activity ) {
	int index = - 1;
	if( ( index = m_Activities.findIndex( activity ) ) == -1 )
		return false;
	m_Activities.remove( m_Activities.at( index ) );
	calculateSize();
	return true;
}

void StateWidget::setActivities( QStringList & list ) {
	m_Activities = list;
	calculateSize();
}

QStringList & StateWidget::getActivityList() {
	return m_Activities;
}

bool StateWidget::renameActivity( QString activity, QString newName ) {
	int index = - 1;
	if( ( index = m_Activities.findIndex( activity ) ) == -1 )
		return false;
	m_Activities[ index ] = newName;
	return true;
}

bool StateWidget::isState(WorkToolBar::ToolBar_Buttons tbb, StateType& resultType)
{
	bool status = true;
	switch (tbb) {
		case WorkToolBar::tbb_Initial_State:
			resultType = Initial;
			break;
		case WorkToolBar::tbb_State:
			resultType = Normal;
			break;
		case WorkToolBar::tbb_End_State:
			resultType = End;
			break;
		default:
			status = false;
			break;
	}
	return status;
}

bool StateWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement stateElement = qDoc.createElement( "UML:StateWidget" );
	bool status = UMLWidget::saveToXMI( qDoc, stateElement );
	stateElement.setAttribute( "statename", m_Name );
	stateElement.setAttribute( "documentation", m_Doc );
	stateElement.setAttribute( "statetype", m_StateType );
	//save states activities
	QDomElement activitiesElement = qDoc.createElement( "Activities" );
	for( QStringList::Iterator it = m_Activities.begin(); it != m_Activities.end(); ++it ) {
		QDomElement tempElement = qDoc.createElement( "Activity" );
		tempElement.setAttribute( "name", *it );
		activitiesElement.appendChild( tempElement );
	}//end for
	stateElement.appendChild( activitiesElement );
	qElement.appendChild( stateElement );
	return status;
}

bool StateWidget::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidget::loadFromXMI( qElement ) )
		return false;
	m_Name = qElement.attribute( "statename", "" );
	m_Doc = qElement.attribute( "documentation", "" );
	QString type = qElement.attribute( "statetype", "1" );
	m_StateType = (StateType)type.toInt();
	//load states activities
	QDomNode node = qElement.firstChild();
	QDomElement tempElement = node.toElement();
	if( !tempElement.isNull() && tempElement.tagName() == "Activities" ) {
		QDomNode node = tempElement.firstChild();
		QDomElement activityElement = node.toElement();
		while( !activityElement.isNull() ) {
			if( activityElement.tagName() == "Activity" ) {
				QString name = activityElement.attribute( "name", "" );
				if( !name.isEmpty() )
					m_Activities.append( name );
			}//end if
			node = node.nextSibling();
			activityElement = node.toElement();
		}//end while
	}//end if
	return true;
}






#include "statewidget.moc"
