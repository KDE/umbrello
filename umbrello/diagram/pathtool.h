 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_PATHTTOOL_H
#define UMBRELLO_PATHTTOOL_H

#include "tool.h"
#include <qptrlist.h>
#include <qpoint.h>


class QCanvasLine;

namespace Umbrello
{
class DiagramView;

class PathTool : public Tool
{
public:
	PathTool( DiagramView  *view );
	virtual ~PathTool();
	
	virtual void activate();
	virtual void deactivate();
protected:
	virtual void setCursor( );
	
	virtual bool mousePressEvent( );
	virtual bool mouseReleaseEvent( );
	virtual bool mouseMoveEvent( );
	virtual bool mouseDblClickEvent( );
	virtual bool mouseDragEvent( );
	virtual bool mouseDragReleaseEvent( );
	
	virtual bool keyPressed( int );
	virtual bool keyReleased( int );
	
	virtual void cancelPathCreation( );
	virtual bool createNextPathPoint( );
	virtual void createPath( );
	
	virtual QPoint findNextPathPoint( );
	
	
	QPtrList<QCanvasLine> m_linePath;
};


}


#endif //#define UMBRELLO_PATHTTOOL_H
