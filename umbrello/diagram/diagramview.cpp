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

#include "../class.h"
#include "../package.h"
#include "../interface.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"
#include "../association.h"

#include "../worktoolbar.h"
#include "../umldoc.h"
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>

using namespace Umbrello;
using std::map;

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
	if(umlTypeMap.empty())
	{
		initMaps();
	}

	m_contextMenu = new QPopupMenu(this);
	m_linePath = new QPtrList<QCanvasLine>();
	m_linePath->setAutoDelete(true);
	m_widgetStack = new QPtrList<DiagramElement>();
	m_widgetStack->setAutoDelete(false);

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
//kdDebug()<<"setting current tool to : "<<tool<<endl;
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
	m_currentAction = None;
	m_linePath->clear();
	if(m_selectionRect)
	{
		delete m_selectionRect;
		m_selectionRect = 0L;
		canvas()->update();
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
	//Try to add an UML Object to the diagram
	case WorkToolBar::Class:
	case WorkToolBar::Interface:
	case WorkToolBar::Package:
	case WorkToolBar::Component:
	case WorkToolBar::Object:
	case WorkToolBar::State:
	case WorkToolBar::Activity:
	case WorkToolBar::UseCase:
	case WorkToolBar::Actor:
		if(!list.isEmpty())
		{//cannot create an object on top of other object
			return;
		}
		if( diagram()->acceptType(*(umlTypeMap[m_tool])))
		{
			UMLObject *obj = diagram()->document()->createUMLObject(*(umlTypeMap[m_tool]));
			if( obj )
			{
				diagram()->createUMLWidget(obj, diagramPos);
			}
			return;
		}
		break;
////////////////////////////////////////////////////////
//FIXME
	case WorkToolBar::InitialActivity: //initial/end activities and states are just
	case WorkToolBar::EndActivity:	  //roles of states/activites, not sub classes. how should we
	case WorkToolBar::Branch:	//handle this? create a state/activity, and then call setRole(type)?
	case WorkToolBar::Fork:		//what are fork/branches in the model?
			break;
/////////////////////////////////////////////////

	//Try to create an association between two UMLObjects.
	case WorkToolBar::Generalization:
	case WorkToolBar::Aggregation:
	case WorkToolBar::Association:
	case WorkToolBar::Dependency:
	case WorkToolBar::CollMessage:
//	case WorkToolBar::Implementation:
	case WorkToolBar::Composition:
		//we need to remove the path line from the list of collisions first
		list.remove(m_linePath->last());
		if( m_currentAction == CreatingAssociation )
		{//association creation is already in progres...
			if(list.isEmpty())
			{//make an anchor point / create a path
				//kdDebug()<<"check for shift/ctl to make orthogonal lines"<<endl;
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
				UMLWidget *wB = dynamic_cast<UMLWidget*>(list.first());
				if(!wB)
				{
					kdDebug()<<"we hit something that is not an UMLWidget!!"<<endl;
					return;
				}
				//FIXME checkAssociation widgatA->umlObject(), widgetB->umlObject(), assoctype
				// or have the document check the association and return 0 / throw exception if not valid
				QPointArray path( m_linePath->count() );
				for( int i = 0; i < m_linePath->count(); i++ )
				{
					path[i] = m_linePath->at(i)->endPoint();
				}
				UMLWidget *wA = dynamic_cast<UMLWidget*>(m_widgetStack->first());
				UMLAssociation *assoc = diagram()->document()->createUMLAssociation( wA->umlObject( ), wB->umlObject( ), associationTypeMap[m_tool]);
				if( assoc )
				{
					diagram()->createAssociationWidget( assoc,wA, wB, path );
				}

				viewport()->setMouseTracking(false);
				m_linePath->clear();
				m_widgetStack->clear();
				canvas()->update();
				m_currentAction = None;
				return;
			}
		}
		//else, start creating association
		//first check if the "source" objects accepts the association type at all
		if(list.isEmpty())
		{
			return;
		}
		else
		{
			m_currentAction = CreatingAssociation;
			UMLWidget *w = dynamic_cast<UMLWidget*>(list.first());
			if(!w)
			{//we hit something but it was not a UMLWidget (maybe a customwidget or another kind of
			// DiagramElement or CanvasItem
				return;
			}
			if(!(w->umlObject()->acceptAssociationType(associationTypeMap[m_tool])))
			{
				kdDebug()<<"Uml Object type does not accept association type (int)"<<(int)associationTypeMap[m_tool]<<endl;
				return;
			}
			m_widgetStack->append(w);
			// ok, association in progress.. create the path
			QCanvasLine *line = new QCanvasLine(canvas());
			line->setPoints(diagramPos.x(),diagramPos.y(),diagramPos.x(),diagramPos.y());
			m_linePath->append(line);
			line->show();
			canvas()->update();
			viewport()->setMouseTracking(true);
			return;
		}
		break;

	//create Generic / Custom Widgets
	case WorkToolBar::Note:
	case WorkToolBar::Text:
		diagram()->createCustomWidget( customWidgetMap[m_tool], diagramPos);
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
switch( m_tool )
{
	case WorkToolBar::Select:
		{QCanvasItemList list = diagram()->collisions(diagramPos);
		if(list.isEmpty())
		{
			return;
		}
		diagram()->moveSelectedBy(diff.x(),diff.y());
		}
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

void DiagramView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
QPoint diagramPos = e->pos(); //translate FIXME
QCanvasItemList list = diagram()->collisions(diagramPos);
if(!list.isEmpty())
{
	dynamic_cast<DiagramElement*>(list.first())->execDefaultAction();
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
	toolBar->slotCheckToolBar((Uml::Diagram_Type)diagram()->diagramType());
	QCanvasView::focusInEvent(e);
}

//////////////////////////////////////////////////////////
map<WorkToolBar::EditTool, const std::type_info*> DiagramView::umlTypeMap;
map<WorkToolBar::EditTool, Uml::Association_Type> DiagramView::associationTypeMap;
map<WorkToolBar::EditTool, int> DiagramView::customWidgetMap;
void DiagramView::initMaps()
{
// UMLObject type map
	umlTypeMap[WorkToolBar::Actor] = &typeid(UMLActor);
	umlTypeMap[WorkToolBar::UseCase] = &typeid(UMLUseCase);

	umlTypeMap[WorkToolBar::Class] = &typeid(UMLClass);
	umlTypeMap[WorkToolBar::Package] = &typeid(UMLPackage);
	umlTypeMap[WorkToolBar::Interface] = &typeid(UMLInterface);

	//FIXME missing: template, activity, state..

// Association type map

	associationTypeMap[WorkToolBar::Generalization] = Uml::at_Generalization;
	associationTypeMap[WorkToolBar::Aggregation] = Uml::at_Aggregation;
	associationTypeMap[WorkToolBar::Dependency] = Uml::at_Dependency;
	associationTypeMap[WorkToolBar::Association] = Uml::at_Association;
	associationTypeMap[WorkToolBar::CollMessage] = Uml::at_Coll_Message;
	associationTypeMap[WorkToolBar::SeqMessage] = Uml::at_Seq_Message;
	associationTypeMap[WorkToolBar::Composition] = Uml::at_Generalization;
	associationTypeMap[WorkToolBar::UniAssociation] = Uml::at_UniAssociation;
	associationTypeMap[WorkToolBar::StateTransition] = Uml::at_State;
	associationTypeMap[WorkToolBar::ActivityTransition] = Uml::at_Activity;

// custom widget map
	customWidgetMap[WorkToolBar::Note] = (int)Uml::wt_Note;
	customWidgetMap[WorkToolBar::Text] = (int)Uml::wt_Text;

	 /** self-msgs?, at_Implementation,
	    at_Composition, at_Anchor*/
}

}

#include "diagramview.moc"
