/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef DIA_DIAGRAMVIEW_H
#define DIA_DIAGRAMVIEW_H

#include <qcanvas.h>
#include <map>
#include <typeinfo>

#include "../umlnamespace.h"
#include "../worktoolbar.h"

class QPopupMenu;


namespace Umbrello {

class Diagram;
class DiagramElement;

class DiagramView : public QCanvasView
{
Q_OBJECT

public:
	DiagramView( Diagram *diagram, WorkToolBar *toolbar, 
		QWidget *parent = 0, const char *name= 0, WFlags f = 0);
		
	virtual ~DiagramView();
	
	Diagram* diagram() const;
	
public slots:
	virtual void setTool(WorkToolBar::EditTool tool );
	
protected:

	virtual void contentsMousePressEvent( QMouseEvent *e );
	virtual void contentsMouseReleaseEvent(QMouseEvent *e );
	virtual void contentsMouseMoveEvent( QMouseEvent *e );
	virtual void contentsMouseDoubleClickEvent( QMouseEvent *e );
	virtual void contentsDragEnterEvent(QDragEnterEvent *e);
	virtual void contentsContextMenuEvent(QContextMenuEvent *e);
	virtual void contentsDropEvent(QDropEvent *e);
	
	virtual void focusInEvent(QFocusEvent *);
	

	WorkToolBar *toolBar;
	WorkToolBar::EditTool m_tool;
	QPopupMenu *m_contextMenu;
	
	QPoint m_savedPosition;
	QPoint m_selectFrom, m_selectTo;
	QCanvasRectangle *m_selectionRect;
	QPtrList<QCanvasLine> *m_linePath;
	QPtrList<DiagramElement> *m_widgetStack;
	
	long m_currentAction;
	
	/** Map WorkToolBar::EditTool to a UML type - which can then be inserted
	  * in a diagram */
	static std::map<WorkToolBar::EditTool, const std::type_info*> umlTypeMap;
	
	/** Map WorkToolBar::EditTool to Association types - used for creating associations
	 * between the UMLObjects*/
	static std::map<WorkToolBar::EditTool, Uml::Association_Type> associationTypeMap;
	
	/** Map WorkToolBar::EditTool to generic/custome widgets -> that is, widgets wich
	 * have no real value in the model and used only in the diagram for visual purposes
	 * examples are boxes, lines ( line != AssociationWidget ), texts, etc*/
	static std::map<WorkToolBar::EditTool, int> customWidgetMap;
private:
	void initMaps();
};


};

#endif

