/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "diagram.h"
#include "classwidget.h"
#include "actorwidget.h"

#include "../umldoc.h"
#include "../umlobject.h"

#include "../concept.h"
#include "../interface.h"
#include "../package.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"
//#include "../activity.h"
//#include "../state.h"

#include "../clipboard/umldrag.h"
#include "../umllistviewitemdatalist.h"

#include <typeinfo>
#include <algorithm>
#include <qpopupmenu.h>
#include <qcolor.h>

#include <kdebug.h>


typedef UMLConcept UMLClass;
using std::map;
using std::find;


//anonymous namespace for file-only data
namespace {
 int defaultWidth = 800;
 int defaultHeight = 600;
 
 QColor defaultPenColor(255,10,10);
 QColor defaultBrushColor(50,255,20);

}

namespace Umbrello{

Diagram::Diagram( DiagramType type, UMLDoc *docparent, int id, const QString &name) :
	QCanvas(docparent,name.latin1()), m_type(type), m_doc(docparent), m_id(id), m_name(name)
{
	if(allowedTypes.empty())
	{
		initAllowedTypesMap();
	}
	resize(defaultWidth,defaultHeight);
	m_pen.setColor(defaultPenColor);
	m_brush.setColor(defaultBrushColor);
	m_brush.setStyle(Qt::SolidPattern);

}

Diagram::~Diagram()
{
}


void Diagram::fillContextMenu(QPopupMenu &menu) const
{
	menu.insertItem("diagram properties",this,SLOT(properties()));
}

void Diagram::properties()
{
kdDebug()<<"diagram::properties : show dialog here"<<endl;
}

void Diagram::selectAll()
{
	QCanvasItemList list = allItems();
	QCanvasItemList::iterator it;
	for(it = list.begin(); it != list.end(); ++it)
	{
  		(*it)->setSelected(true);
	}

}
void Diagram::deselectAll()
{
	QCanvasItemList list = allItems();
	QCanvasItemList::iterator it;
	for(it = list.begin(); it != list.end(); ++it)
	{
  		(*it)->setSelected(false);
	}
}

void Diagram::moveSelectedBy(int x, int y)
{
	QCanvasItemList list = allItems();
	QCanvasItemList::iterator it;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((*it)->isSelected())
			dynamic_cast<DiagramElement*>(*it)->moveBy(x,y);
	}
}


void Diagram::createWidget( uint umlObjectID, const QPoint &pos)
{
	UMLObject* o = 0;
	o = m_doc->findUMLObject(umlObjectID);
	if(!o)
	{
		kdWarning()<<"Diagram::createWidget(uint,QPoint): object with id = "
			<<umlObjectID<<" not found in document"<<endl;
 		return;
	}
	createWidget(o,pos);
}

void Diagram::createWidget( UMLObject *obj, const QPoint &pos)
{

	UMLWidget *w = 0L;
	const type_info &type = typeid(*obj);
	if(!acceptType(type))
	{kdDebug()<<"type "<<type.name()<<"not accepted by diagram.(diagram type ="<<m_type<<")"<<endl;
		return;
	}
	
	int widgetId = m_doc->getUniqueID();
	if( type == typeid(UMLClass) )
	{
		w = w = new ClassWidget(this,widgetId,dynamic_cast<UMLClass*>(obj));
	}
	else if ( type == typeid(UMLActor) )
	{
		w = new ActorWidget(this,widgetId,dynamic_cast<UMLActor*>(obj));
	}
	//else if ( type == typeid(...) )
	else
	{
		kdDebug()<<"Widget fot type "<<type.name()<<" not yet implemented"<<endl;
	}
		
	w->moveAbs(pos.x(),pos.y());
	w->show();
	update();
	
	emit modified();

}


bool Diagram::acceptType(const type_info &type)
{kdDebug()<<"accept? "<<type.name()<<endl;
return (find((allowedTypes[m_type]).begin(),
		     (allowedTypes[m_type]).end(),
		     &type) != allowedTypes[m_type].end());

}

bool Diagram::canAcceptDrop(QDropEvent *e)
{
	UMLListViewItemDataList list;
	bool status = UMLDrag::decode(e,list);
	if(!status)
	{
		return false;
	}

	UMLListViewItemDataListIt it(list);
	UMLListViewItemData* data = it.current();
	UMLObject* o = 0;
	o = m_doc->findUMLObject(data->getID());
	if(!o)
	{
		kdWarning()<<"object with id = "<<data->getID()<<" not found in document"<<endl;
 		return false;
	}
	return acceptType(typeid(*o));
	
// see if we are trying to drop elements from our own model
	//if (m_doc->hasObjects(x))
	//{
	
	//}
	//else
	//{
// see if the model can accept the data being draged	
	//m_doc->canAcceptDrop(x);
	//if yes, we have two options:
	// if the data is accepted by both, the model and the diagram add it to both
	// if it is accepted by the model but not by the diagram we can 1) just reject
	// the drop, or add the data to the model but not to the diagram
	//}
	
}

void Diagram::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(canAcceptDrop(e));
}

void Diagram::dropEvent(QDropEvent *e)
{
	UMLListViewItemDataList list;
	bool status = UMLDrag::decode(e,list);
	if(!status)
	{
		return;
	}

	UMLListViewItemDataListIt it(list);
	UMLListViewItemData* data = it.current();
	UMLObject* o = 0;
	o = m_doc->findUMLObject(data->getID());
	if(!o)
	{
		kdDebug()<<"object with id = "<<data->getID()<<" not found in document"<<endl;
 		return;
	}
	
	//depending on what type we are.. for now we are only class diagrams
	UMLClass *umlClass = dynamic_cast<UMLClass*>(o);
	if(!umlClass)
	{
		kdDebug()<<"object found, but is not a class. not accepted!"<<endl;
		kdDebug()<<"rtti is "<<typeid(o).name()<<endl;
		return;
	}
	int widgetId = m_doc->getUniqueID();
	kdDebug()<<"creating widget with id = "<<widgetId<<endl;
	ClassWidget *wid = new ClassWidget(this,widgetId,umlClass); 
	wid->moveAbs(e->pos().x(),e->pos().y());
	wid->show();
	update();
	
	m_doc->setModified(true);
	
	//////FIXME
	/*This will be removed. a port of the drag and drop mechanism
	//is needed. make it based on the XMI info
	//FIXME	UMLListViewItemDataList list;
	bool status = UMLDrag::decode(e, list);
		if(!status) {
		return;
	}

	UMLListViewItemDataListIt it(list);
	UMLListViewItemData* data = it.current();
	
//FIXME endl*/

}

UMLDoc* Diagram::document() const
{
	return m_doc;
}


/////////////////////////////////////////////////////////////////
map<Diagram::DiagramType,list<const type_info*> > Diagram::allowedTypes;

void Diagram::initAllowedTypesMap()
{
	list<const type_info*> *list = &allowedTypes[ClassDiagram];
		list->insert(list->end(),&typeid(UMLClass));
		list->insert(list->end(),&typeid(UMLInterface));
		list->insert(list->end(),&typeid(UMLPackage));
		list->insert(list->end(),&typeid(UMLTemplate));

	list = &allowedTypes[UseCaseDiagram];
		list->insert(list->end(),&typeid(UMLActor));
		list->insert(list->end(),&typeid(UMLUseCase));
		
	list = &allowedTypes[CollaborationDiagram];
		list->insert(list->end(),&typeid(UMLClass));
		
//	allowedTypes[ActivityDiagram].insert(&typeid(UMLActivity));
	
//	allowedTypes[SequenceDiagram].insert(&typeid(UMLState));
	
}


}

