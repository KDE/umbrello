/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef UMBRELLO_DIAGRAMVIEW_H
#define UMBRELLO_DIAGRAMVIEW_H

#include <qcanvas.h>
//#include <map>
//#include <typeinfo>

//#include "../umlnamespace.h"
#include "toolbar.h"

class QPopupMenu;


namespace Umbrello {

class Diagram;
class Tool;
class ToolBar;

class DiagramView : public QCanvasView
{
Q_OBJECT

public:
	DiagramView( Diagram *diagram, QWidget *parent = 0, const char *name = 0, WFlags f = 0);
	virtual ~DiagramView();
	
	Diagram* diagram() const;
	
public slots:
	virtual void setTool( Tool* );
	
protected:

	virtual void keyPressEvent( QKeyEvent *e );
	virtual void keyReleaseEvent( QKeyEvent *e );
	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void contentsMouseReleaseEvent(QMouseEvent *e );
	virtual void contentsMouseMoveEvent( QMouseEvent *e );
	virtual void contentsMouseDoubleClickEvent( QMouseEvent *e );
	virtual void contentsContextMenuEvent(QContextMenuEvent *e);
	virtual void contentsDragEnterEvent(QDragEnterEvent *e);
	virtual void contentsDropEvent(QDropEvent *e);
	
	virtual void focusInEvent(QFocusEvent *);
	virtual void hideEvent(QHideEvent *);
	virtual void showEvent(QShowEvent *);
	
private:
	ToolBar *m_toolBar;
	Tool *m_tool;
	QPopupMenu *m_contextMenu;
	
};


}

#endif // UMBRELLO_DIAGRAMVIEW_H

