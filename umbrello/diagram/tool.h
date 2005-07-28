  /***************************************************************************
                               tool.h
                             -------------------
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

#ifndef UMBRELLO_TOOL_H
#define UMBRELLO_TOOL_H

#include <qstring.h>
#include <qpoint.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>

class QMouseEvent;
class Q3CanvasItemList;

namespace Umbrello
{
class DiagramView;
class Diagram;
class DiagramElement;
class DiagramWidget;

class Tool{
public:
	Tool( DiagramView  *view, const QString& name );
	virtual ~Tool();
	
	virtual void activate();
	virtual void deactivate();
	
	inline QString name() const;
	inline QString category() const;
	inline QString icon() const;
	inline QString toolTip() const;
	
	bool mouseEvent( QMouseEvent*, const QPoint& );
	bool keyEvent( QEvent* );
	
protected:

	void setName( const QString& );
	void setCategory( const QString & );
	void setIcon( const QString& );
	void setToolTip( const QString& );
	
	virtual void setCursor( );
	
	/** return true if the tool consumed the event*/
	virtual bool mousePressEvent( );
	virtual bool mouseReleaseEvent( );
	virtual bool mouseMoveEvent( );
	virtual bool mouseDblClickEvent( );
	virtual bool mouseDragEvent( );
	virtual bool mouseDragReleaseEvent( );
	
	virtual bool keyPressed( int );
	virtual bool keyReleased( int );
	
	inline int buttonPressed( ) const;
	inline bool shiftPressed( ) const;
	inline bool ctrlPressed( ) const;
	inline bool altPressed( ) const;
	
	inline QPoint currentPos( ) const;
	inline QPoint savedPos( ) const;
	
	
	inline DiagramView* view() const;
	inline Diagram* diagram() const;
	
	
private:
	QString m_name;
	QString m_category;
	QString m_icon;
	QString m_toolTip;
	
	int m_buttonPressed;
	bool m_shiftPressed;
	bool m_ctrlPressed;
	bool m_altPressed;
	bool m_isDragging;
	
	QPoint m_savedPos;
	QPoint m_currentPos;
	
	DiagramView *m_view;
	Diagram* m_diagram;
};

QString Tool::name() const {return m_name;}
QString Tool::category() const {return m_category;}
QString Tool::icon() const {return m_icon;}
QString Tool::toolTip() const {return m_toolTip;}

int Tool::buttonPressed( ) const {return m_buttonPressed;}
bool Tool::shiftPressed( ) const {return m_shiftPressed;}
bool Tool::ctrlPressed( ) const {return m_ctrlPressed;}
bool Tool::altPressed( ) const {return m_altPressed;}

QPoint Tool::currentPos( ) const {return m_currentPos;}
QPoint Tool::savedPos( ) const {return m_savedPos;}

DiagramView* Tool::view() const {return m_view;}
Diagram* Tool::diagram() const {return m_diagram;}

}


#endif //UMBRELLO_TOOL_H
