/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diagram.h"
#include "widgetfactory.h"
#include "diagramelement.h"
#include "umlwidget.h"

#include "../umldoc.h"

//include declarations of umlobject and subclasses
//needed for the RTTI
#include "../umlobject.h"
#include "../class.h"
#include "../interface.h"
#include "../package.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"
//#include "../activity.h"
//#include "../state.h"
#include "../association.h"

#include "../clipboard/umldrag.h"
#include "../umllistviewitemdatalist.h"


#include <typeinfo>
#include <algorithm>
#include <list>
#include <qpointarray.h>
#include <qpopupmenu.h>
#include <qcolor.h>


#include <kdebug.h>


using std::map;
using std::find;
using std::list;


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


void Diagram::createUMLWidget( uint umlObjectID, const QPoint &pos)
{
	UMLObject* o = 0;
	o = m_doc->findUMLObject(umlObjectID);
	if(!o)
	{
		kdWarning()<<"Diagram::createUMLWidget(uint,QPoint): object with id = "
			<<umlObjectID<<" not found in document"<<endl;
 		return;
	}
	createUMLWidget(o,pos);
}

void Diagram::createUMLWidget( UMLObject *obj, const QPoint &pos)
{
	WidgetFactory *factory = WidgetFactory::instance();
	if(!( acceptType(typeid(*obj)) ))
	{
		kdDebug()<<"type "<<typeid(*obj).name()
			<<"not accepted by diagram.(diagram type ="<<m_type<<")"<<endl;
		return;
	}
	DiagramElement *w;
	if( (w = factory->createUMLWidget(obj,this)) )
	{
		w->moveAbs(pos.x(),pos.y());
		w->show();
		update();
		emit modified();
	}
}

void Diagram::createAssociationWidget( UMLAssociation *assoc,UMLWidget *wA, UMLWidget *wB, const QPointArray &path )
{
	WidgetFactory *factory = WidgetFactory::instance();
	DiagramElement *w = factory->createAssociationWidget( assoc, wA, wB, path, this );
	if( w )
	{kdDebug()<<"showing association"<<endl;
		w->moveAbs(wA->x(), wA->y());
		w->show();
		update();
		emit modified( );
	}
	else kdDebug()<<"widget factory regturned null assoc widget"<<endl;

}

void Diagram::createCustomWidget( int type, const QPoint &pos )
{
	WidgetFactory *factory = WidgetFactory::instance();
	//since these widgets have no real meaning in the
	//model, we dont need any checks: they are allways accepted
	DiagramElement *w = factory->createCustomWidget( type, this );
	if(!w)
	{
		return;
	}
	w->moveAbs(pos.x(),pos.y());
	w->show();
	update();
	emit modified();
}


bool Diagram::acceptType(const std::type_info &type)
{//kdDebug()<<"accept? "<<type.name()<<endl;
return (find((allowedTypes[m_type]).begin(),
		     (allowedTypes[m_type]).end(),
		     &type) != allowedTypes[m_type].end());
}

void Diagram::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(canAcceptDrop(e));
}

void Diagram::dropEvent(QDropEvent *e)
{
	UMLListViewItemDataList list;
	bool status = UMLDrag::decodeClip3(e,list);
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
		kdWarning()<<"object with id = "<<data->getID()<<" not found in document"<<endl;
 		return;
	}
	createUMLWidget(o,e->pos());
}

bool Diagram::canAcceptDrop(QDropEvent *e)
{
	UMLListViewItemDataList list;
	bool status = UMLDrag::decodeClip3(e,list);
	if(!status)
	{
		return false;
	}

	UMLListViewItemDataListIt it(list);
	UMLListViewItemData* data = it.current();
	UMLObject* o = 0;
	if( !(o = m_doc->findUMLObject(data->getID())) )
	{
		kdWarning()<<"object with id = "<<data->getID()<<" not found in document"<<endl;
 		return false;
	}
	return acceptType(typeid(*o));
//FIXME - change UMLDrag - make it XMI based
// check if we are droping elements from another model (ie, from another instance of
// umbrello or another program which also "exports" XMI
// if we the drag comes from another app, check if the document (model) accepts it.
// and check if the diagram accepts it as well.
}


UMLDoc* Diagram::document() const
{
	return m_doc;
}


/////////////////////////////////////////////////////////////////
map<Diagram::DiagramType,list<const std::type_info*> > Diagram::allowedTypes;

void Diagram::initAllowedTypesMap()
{
	list<const std::type_info*> *list = &allowedTypes[ClassDiagram];
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


#include "diagram.moc"
