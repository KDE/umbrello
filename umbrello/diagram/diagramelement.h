/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIAGRAMELEMENT_H
#define DIAGRAMELEMENT_H

#include <qobject.h>
#include <qstring.h>
#include <qpoint.h>
#include <kdebug.h>


#include "diagram.h"
class QPopupMenu;

namespace Umbrello{


class DiagramElement :  public QObject, public QCanvasPolygonalItem 
{
	Q_OBJECT

public:

	DiagramElement( Diagram *diagram, int id);
	
	virtual ~DiagramElement();
	
	int getID() const;
	
	virtual void moveAbs( int x, int y );
	
	virtual void moveAbs( const QPoint & );
	
	virtual void moveBy( int dx, int dy);
	
	virtual void fillContextMenu(QPopupMenu &menu);

	virtual void setSelected(bool);
	
	
	/** Execute some action when double click...
	some subclasses may choose to call properties, other to do
	other things*/
	virtual void doubleClick();
	
	Diagram* diagram() const;
	
public slots:
	void moveToForeground();
	void moveToBackground();
	virtual void editProperties();
	

signals:
	void moved();
	
protected:
	virtual void drawShape(QPainter& ) = 0;

	int m_id;
	
};


} // end of namespace Umbrello

#endif  //  DIAGRAMELEMENT_H
