/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diagramview.h"
#include "diagram.h"
#include "tool.h"
#include "toolbar.h"
#include "diagramelement.h"


#include "../uml.h"
// #include "umlwidget.h"
// #include "diagramelement.h"


#include <qpoint.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>



namespace Umbrello{

DiagramView::DiagramView( Diagram *diagram, QWidget *parent, const char *name, WFlags f) :
             QCanvasView( diagram, parent, name, f ),m_tool(0L)
{
	if( parent )
		m_toolBar = new ToolBar( UMLApp::app(), this, QMainWindow::Top );
	else
		m_toolBar = new ToolBar( UMLApp::app(), this );
	m_contextMenu = new QPopupMenu( this, "diagram context menu");
	m_toolBar->showTools( "selecttool" );
	m_toolBar->showTools( "pathtool" );
	m_toolBar->showTools( "classtool" );
	m_toolBar->showTools( "interfacetool" );
	m_toolBar->showTools( "packagetool" );
	m_toolBar->showTools( "associationtool" );
	m_toolBar->showTools( "generalizationtool" );
	m_toolBar->showTools( "compositiontool" );
	m_toolBar->showTools( "boxtool" );
	m_toolBar->showTools( "notetool" );
	m_toolBar->showTools( "texttool" );
	m_toolBar->show();

	viewport()->setAcceptDrops(true);
	connect(m_toolBar,SIGNAL(toolChanged(Tool*)),this,SLOT(setTool(Tool*)));
	
	m_toolBar->setCurrentTool( "selecttool" );
	viewport()->setFocusPolicy(ClickFocus);
}

DiagramView::~DiagramView()
{
}

Diagram* DiagramView::diagram() const
{
	return dynamic_cast<Diagram*>(canvas());
}

void DiagramView::setTool( Tool *tool )
{
	if( m_tool )
		m_tool->deactivate( );
	m_tool = tool;
	if( m_tool )
		m_tool->activate( );
}

void DiagramView::contentsMousePressEvent( QMouseEvent *e )
{
	QPoint diagramPos = e->pos(); // FIXME translate to diagram (canvas) coordinates
	if( !m_tool )
		return;
	bool accepted = m_tool->mouseEvent(e,diagramPos);
	if( ! accepted && e->button() == RightButton )
	{kdDebug()<<"menu"<<endl;
		m_contextMenu->clear( );
		DiagramElement *element = diagram()->firstDiagramElement( diagramPos );
		if( element )
			element->fillContextMenu(*m_contextMenu);
		else
 			diagram()->fillContextMenu(*m_contextMenu);
		m_contextMenu->exec(QCursor::pos());
		diagram()->update();
	}
}


void DiagramView::contentsContextMenuEvent(QContextMenuEvent *e)
{
}


void DiagramView::contentsMouseReleaseEvent(QMouseEvent *e )
{
	QPoint diagramPos = e->pos(); //FIXME translate to diagram(canvas) coordinates
	if( !m_tool )
		return;
	bool accepted = m_tool->mouseEvent(e,diagramPos);
}

void DiagramView::contentsMouseMoveEvent( QMouseEvent *e )
{
	QPoint diagramPos = e->pos(); //FIXME translate to diagram(canvas) coordinates
	if( !m_tool )
		return;
	bool accepted = m_tool->mouseEvent(e,diagramPos);
}

void DiagramView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
	QPoint diagramPos = e->pos(); //FIXME translate to diagram(canvas) coordinates
	if(!m_tool)
		return;
	bool accepted = m_tool->mouseEvent(e,diagramPos);
}


void DiagramView::contentsDragEnterEvent(QDragEnterEvent *e)
{
//we let the diagram decide if it can accept this drop
	diagram()->dragEnterEvent(e);
}

void DiagramView::contentsDropEvent(QDropEvent *e)
{
//translate coordinates with e->setPos(diagramposition);
	diagram()->dropEvent(e);
}

void DiagramView::keyPressEvent( QKeyEvent *e )
{kdDebug()<<"key event"<<endl;
	bool consumed = false;
	if(m_tool)
		consumed = m_tool->keyEvent(e);
	if( consumed )
		e->accept();
	else
		e->ignore();

}
void DiagramView::keyReleaseEvent( QKeyEvent *e )
{
	kdDebug()<<"key event"<<endl;
	bool consumed = false;
	if(m_tool)
		consumed = m_tool->keyEvent(e);
	if( consumed )
		e->accept();
	else
		e->ignore();

}

void DiagramView::focusInEvent(QFocusEvent *e)
{
	QCanvasView::focusInEvent(e);
}

void DiagramView::hideEvent(QHideEvent *e)
{
	m_toolBar->hide();
	QCanvasView::hideEvent(e);
}

void DiagramView::showEvent(QShowEvent *e)
{
	m_toolBar->show();
	QCanvasView::showEvent(e);
}

}

#include "diagramview.moc"
