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

#ifndef UMBRELLO_SELECTTOOL_H
#define UMBRELLO_SELECTTOOL_H

#include "tool.h"

class QCanvasRectangle;

namespace Umbrello
{
class DiagramView;

class SelectTool : public Tool
{
public:
	SelectTool( DiagramView  *view );
	virtual ~SelectTool();
	
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
	
	virtual bool keyPressed( Qt::Key );
	virtual bool keyReleased( Qt::Key );
	
	QPoint m_selectFrom;
	QCanvasRectangle *m_selectionRect;
};


}


#endif //UMBRELLO_SELECTTOOL_H
