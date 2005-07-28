  /***************************************************************************
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tool.h"
#include "diagramview.h"
#include "diagram.h"
#include "diagramelement.h"
#include "diagramwidget.h"
#include "path.h"


#include <qevent.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <kdebug.h>
#include <klocale.h>


namespace Umbrello
{

Tool::Tool( DiagramView  *view, const QString& name ):
	m_view(view), m_diagram(view->diagram())
{
	m_name = name;
	m_toolTip = i18n("This tool has no tooltip");
	m_category = "Miscelaneous";
	m_icon = "unknown";
	m_shiftPressed = false;
	m_ctrlPressed = false;
	m_altPressed = false;
}

Tool::~Tool()
{ }

void Tool::activate()
{
}

void Tool::deactivate()
{
}

bool Tool::mouseEvent( QMouseEvent *event, const QPoint& diagramPos)
{
	if(!view() || !diagram())
		return false;
	m_savedPos = m_currentPos;
	m_currentPos = diagramPos;

	m_altPressed = event->state() & Qt::AltModifier;
	m_shiftPressed = event->state() & Qt::ShiftModifier;
	m_ctrlPressed = event->state() & Qt::ControlModifier;

	bool consumed = false;
	switch( event->type() )
	{
		case QEvent::MouseButtonDblClick:
			m_buttonPressed = event->button();
			consumed =  mouseDblClickEvent();
			return consumed;
		case QEvent::MouseButtonPress:
			m_buttonPressed = event->button();
			consumed = mousePressEvent();
			return consumed;
		case QEvent::MouseButtonRelease:
			if( m_isDragging )
			{
				consumed = mouseDragReleaseEvent( );
				m_isDragging = false;
			}
			else
			{
				consumed = mouseReleaseEvent();
			}
			m_buttonPressed = Qt::NoButton;
			return consumed;
		case QEvent::MouseMove:
			if( m_buttonPressed == Qt::LeftButton )
			{
				consumed = mouseDragEvent( );
				m_isDragging = true;
			}
			else
			{
				consumed = mouseMoveEvent( );
			}
			return consumed;
		default: return false;
	}
	return consumed;
}

bool Tool::keyEvent( QEvent *event )
{
	bool consumed = false;
	QKeyEvent *keyEvent;
	switch( event->type() )
	{
		case QEvent::KeyPress:
			keyEvent = static_cast<QKeyEvent*>(event);
			switch( keyEvent->key() )
			{
				case Qt::Key_Shift:
					m_shiftPressed = true;
					break;
				case Qt::Key_Alt:
					m_altPressed = true;
					break;
				case Qt::Key_Control:
					m_ctrlPressed = true;
					break;
				default:break;
			}
			consumed = keyPressed( keyEvent->key() );
			return consumed;
		case QEvent::KeyRelease:
			keyEvent = static_cast<QKeyEvent*>(event);
			switch( keyEvent->key() )
			{
				case Qt::Key_Shift:
					m_shiftPressed = false;
					break;
				case Qt::Key_Alt:
					m_altPressed = false;
					break;
				case Qt::Key_Control:
					m_ctrlPressed = false;
					break;
				default:break;
			}
			consumed = keyReleased( keyEvent->key());
			return consumed;
		default:
			return false;
	}
	return consumed;
}





void Tool::setCursor( ) {}

bool Tool::mousePressEvent( )      { return false;}
bool Tool::mouseReleaseEvent( )    { return false;}
bool Tool::mouseMoveEvent( )       { return false;}
bool Tool::mouseDragEvent( )       { return false;}
bool Tool::mouseDragReleaseEvent( ){ return false;}
bool Tool::mouseDblClickEvent( )   { return false;}

bool Tool::keyPressed( int ) {return false;}
bool Tool::keyReleased( int ){ return false;}


void Tool::setName( const QString &n ) { m_name = n;}
void Tool::setCategory( const QString &c ) { m_category = c;}
void Tool::setIcon( const QString &i ){ m_icon = i;}
void Tool::setToolTip( const QString &t ){ m_toolTip = t;}

}
