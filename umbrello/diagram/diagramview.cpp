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
#include "umlwidget.h"
#include "diagramelement.h"

#include "../concept.h"
#include "../package.h"
#include "../interface.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"

#include "../worktoolbar.h"
#include "../umldoc.h"
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>

using namespace Umbrello;
using std::map;

typedef UMLConcept UMLClass;

namespace
{	const long None = 0;
	const long Selecting = 0x01;
	const long CreatingAssociation = 0x01<<1;
	int min(int a, int b) { return (a<b?a:b);}
	int max(int a, int b) { return (a>b?a:b);}
}


namespace NewDiagram{

DiagramView::DiagramView( Diagram *diagram, WorkToolBar *toolbar,
			QWidget *parent, const char *name, WFlags f) :
             QCanvasView( diagram, parent, name, f ),toolBar(toolbar),m_selectionRect(0L)
{
	if(toolMap.empty())
	{
		initToolMap();
	}
	
	m_contextMenu = new QPopupMenu(this);
	m_linePath = new QPtrList<QCanvasLine>();
	m_linePath->setAutoDelete(true);
	
	viewport()->setAcceptDrops(true);
	connect(toolbar,SIGNAL(toolSelected(WorkToolBar::EditTool)),this,SLOT(setTool(WorkToolBar::EditTool)));
	setTool(toolbar->currentTool());

}

DiagramView::~DiagramView()
{
	delete m_linePath;
}

Diagram* DiagramView::diagram() const
{
	return dynamic_cast<Diagram*>(canvas());
}

void DiagramView::setTool( WorkToolBar::EditTool tool )
{
kdDebug()<<"setting current tool to : "<<tool<<endl;
	m_tool = tool;
	
	QCursor cursor;	
	switch(m_tool)
	{
		case WorkToolBar::Select:
			setCursor(Qt::ArrowCursor);
			break;
		case WorkToolBar::Class:
		case WorkToolBar::Actor:
		case WorkToolBar::Generalization:
 			setCursor(Qt::CrossCursor);
			break;
//FIXME add others
		
		default:
			setCursor(Qt::ArrowCursor);

	}
}

void DiagramView::contentsMousePressEvent( QMouseEvent *e )
{
if(e->button() == RightButton && m_currentAction == CreatingAssociation )
{	kdDebug()<<"aborting association"<<endl;
	
	viewport()->setMouseTracking(false);
	m_currentAction = None;
	m_linePath->clear();
	diagram()->update();
	e->accept();
	return;
}
if(e->button() != LeftButton)
{
	kdDebug()<<"button not handled in pressevent: "<<e->button()<<endl;
	e->ignore();
	return;
}

m_savedPosition = e->pos();
QPoint diagramPos = e->pos(); //translate FIXME
QCanvasItemList list = diagram()->collisions(diagramPos);


	switch( m_tool )
	{
	case WorkToolBar::Select:
		if(list.isEmpty())
		{
			diagram()->deselectAll();
			m_currentAction = Selecting;
			m_selectFrom = m_savedPosition;
			m_selectionRect = new QCanvasRectangle(m_selectFrom.x(),m_selectFrom.y(),1,1,canvas());
			m_selectionRect->setPen(QPen(QColor("grey"), 0, DotLine));
			m_selectionRect->show();
			return;
		}
		else
		{
			DiagramElement *el = dynamic_cast<DiagramElement*>(list.first());
			
			bool state = el->isSelected();
			if((e->state() & ShiftButton) == NoButton)
			{
				diagram()->deselectAll();
				el->setSelected(true);
			}
			else
			{
				el->setSelected(!state);
			}
			return;
		}
		break;
	case WorkToolBar::Class:
	case WorkToolBar::UseCase:
	case WorkToolBar::Actor:
		if(!list.isEmpty())
		{//cannot create an object on top of other object
		//maybe create the UMLObject but not the Widget??
			return;
		}
		if( diagram()->acceptType(*(toolMap[m_tool])))
		{
			UMLObject *obj = diagram()->document()->createUMLObject(*(toolMap[m_tool]));
			diagram()->createUMLWidget(obj, diagramPos);
			return;
		}
		break;
////////////////////////////////////////////////////////
//FIXME this should be similar to the "UMLObjects" above	
	case WorkToolBar::InitialActivity:
	case WorkToolBar::EndActivity:
	case WorkToolBar::Branch:
	case WorkToolBar::Fork:
	case WorkToolBar::Activity:
	case WorkToolBar::State:
	case WorkToolBar::InitialState:
			break;
/////////////////////////////////////////////////			
	case WorkToolBar::Generalization:
	case WorkToolBar::Aggregation:
	case WorkToolBar::Association:
		kdDebug()<<"association: tool is "<<m_tool<<endl;
		list.remove(m_linePath->last());
		if( m_currentAction == CreatingAssociation )
		{
		
			if(list.isEmpty())
			{//make an anchor point
				kdDebug()<<"anchor point here.."<<endl;
				kdDebug()<<"check for shift/ctl to make orthogonal lines"<<endl;
				QCanvasLine *line = m_linePath->last();
				line->setPoints(line->startPoint().x(),line->startPoint().y(),diagramPos.x(),diagramPos.y());
				line = new QCanvasLine(canvas());
				line->setPoints(diagramPos.x(),diagramPos.y(),diagramPos.x(),diagramPos.y());
				line->show();
				m_linePath->append(line);
				canvas()->update();
				return;
			}
			else
			{//check if assoc ok, create association and widget
			kdDebug()<<"create association here, then create widget, and then set widget's path to linePath."<<endl;
				viewport()->setMouseTracking(false);
				m_linePath->clear();
				canvas()->update();
				m_currentAction = None;
				return;
			}
		}
		//else, start creating association
		kdDebug()<<"start creating association"<<endl;
		if(list.isEmpty())
		{
			
			kdDebug()<<"cannot create an association without a object/widget"<<endl;
			return;
		}
		else
		{
			m_currentAction = CreatingAssociation;
			kdDebug()<<"starting association. first ask if object can accept this association"<<endl;
			QCanvasLine *line = new QCanvasLine(canvas());
			line->setPoints(diagramPos.x(),diagramPos.y(),diagramPos.x(),diagramPos.y());
			m_linePath->append(line);
			line->show();	
			canvas()->update();
			viewport()->setMouseTracking(true);
			return;
		}
		break;
	
	//////
	case WorkToolBar::Note:
	case WorkToolBar::Text:
			//FIXME - place holder only!!
		diagram()->createCustomWidget( 1, diagramPos);
		break;
	default:
		kdDebug()<<"tool "<<m_tool<<" not handled in MousePressEvent"<<endl;
	}

}

	

void DiagramView::contentsContextMenuEvent(QContextMenuEvent *e)
{
m_contextMenu->clear();
QPoint diagramPos = e->pos(); //translate FIXME
QCanvasItemList list = diagram()->collisions(diagramPos);
	if(list.isEmpty())
	{
		diagram()->fillContextMenu(*m_contextMenu);
	}
	else
	{
		dynamic_cast<DiagramElement*>(list.first())->fillContextMenu(*m_contextMenu);
	}
	
m_contextMenu->exec(QCursor::pos());
canvas()->update();
e->accept();
}


void DiagramView::contentsMouseReleaseEvent(QMouseEvent *e )
{
m_currentAction &= ~Selecting;
if(m_selectionRect) 
{
	delete m_selectionRect;
	m_selectionRect = 0L;
	canvas()->update();
 }
}

void DiagramView::contentsMouseMoveEvent( QMouseEvent *e )
{

QPoint diff = e->pos() - m_savedPosition;

if( m_tool == WorkToolBar::Select && m_currentAction == Selecting )
{
	m_selectionRect->setSize(diff.x(),diff.y());
	QCanvasItemList list = diagram()->collisions(m_selectionRect->rect());
	if(!list.isEmpty())
	{
		diagram()->deselectAll();
		QCanvasItemList::iterator it = list.begin();
		for(; it != list.end() ; ++it )
		{
			(*it)->setSelected(true);
		}
		
	}
	canvas()->update();
	return;
}
m_savedPosition = e->pos();
QPoint diagramPos = e->pos(); //FIXME transform coordinates
QCanvasItemList list = diagram()->collisions(diagramPos);
switch( m_tool )
{
	case WorkToolBar::Select:
		if(list.isEmpty())
		{
			return;
		}
		diagram()->moveSelectedBy(diff.x(),diff.y());
		break;
	case WorkToolBar::Generalization:
	case WorkToolBar::Aggregation:
	case WorkToolBar::Association:
		if( m_currentAction == CreatingAssociation )
		{
		QCanvasLine *line = m_linePath->last();
		line->setPoints(line->startPoint().x(),line->startPoint().y(),diagramPos.x(),diagramPos.y());
		canvas()->update();
		}
		break;
	default:
		kdDebug()<<"tool "<<m_tool<<" not handled in mousemove"<<endl;
}
}

void DiagramView::contentsDoubleClickEvent( QMouseEvent *e )
{
QPoint diagramPos = e->pos(); //translate FIXME
QCanvasItemList list = diagram()->collisions(diagramPos);
if(!list.isEmpty())
{
	dynamic_cast<DiagramElement*>(list.first())->doubleClick();
}


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

void DiagramView::focusInEvent(QFocusEvent *e)
{
kdDebug()<<"telling toolbar to change icons"<<endl;
	toolBar->slotCheckToolBar((Uml::Diagram_Type)diagram()->diagramType());
	QCanvasView::focusInEvent(e);
}

//////////////////////////////////////////////////////////
map<WorkToolBar::EditTool, const std::type_info*> DiagramView::toolMap;
void DiagramView::initToolMap()
{

/*	toolMap[WorkToolBar::Generalization] = typeid( ;
	toolMap[WorkToolBar::Aggregation] = rtti::UMLAssociation  ;
	toolMap[WorkToolBar::Dependency] = rtti::UMLAssociation  ;
	toolMap[WorkToolBar::Association] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::CollMessage] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::SeqMessage] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::Composition] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::UniAssociation] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::StateTransition] = rtti::UMLAssociation ;
	toolMap[WorkToolBar::ActivityTransition] = rtti::UMLAssociation ;
//	toolMap[WorkToolBar::Anchor] = AnchorWidget ;
*/	//toolMap[WorkToolBar::Text] = TextWidget ;
	toolMap[WorkToolBar::Actor] = &typeid(UMLActor);
	toolMap[WorkToolBar::UseCase] = &typeid(UMLUseCase);
	
	toolMap[WorkToolBar::Class] = &typeid(UMLClass);
	toolMap[WorkToolBar::Package] = &typeid(UMLPackage);
	toolMap[WorkToolBar::Interface] = &typeid(UMLInterface);
	//toolMap[WorkToolBar::Template] = &typeid(UMLTemplate);
/*	toolMap[WorkToolBar::InitialState] = rtti::UMLState ;
	toolMap[WorkToolBar::EndState] = rtti::UMLState ;
	toolMap[WorkToolBar::InitialActivity] = rtti::UMLActivity ;
	toolMap[WorkToolBar::EndActivity] = rtti::UMLActivity ;
	toolMap[WorkToolBar::Branch] = BranchWidget ;
	toolMap[WorkToolBar::Fork] = ForkWidget ;
*/
}

}

#include "diagramview.moc"
