/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diagram.h"
#include "diagramelement.h"
#include "diagramview.h"
#include "path.h"

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
#include "../umllistviewitemdata.h"

#include <typeinfo>
#include <algorithm>
#include <list>
#include <qpointarray.h>
#include <qpopupmenu.h>
#include <qcolor.h>
#include <klocale.h>


#include <kdebug.h>
#include "../pluginloader.h"
#include "../plugin.h"


#include "widgetfactory.h"
#include "defaultwidgetset.h"
#include "diagramwidget.h"


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
		//create default widget set and load uml widgets... this should not be done here,
		// but somewhere else.. maybe Umbrello's init would be the right place - test only.
		initAllowedTypesMap();
		DefaultWidgetSet *plugin = new DefaultWidgetSet(this,0L);
		if(!plugin)
		{
			kdWarning()<<"cannot create default widget set"<<endl;
			return;
		}
		plugin->init();
		PluginLoader::instance()->loadPlugin("libumlwidgets");
	}
	setUpdatePeriod( 10 );
	resize(defaultWidth,defaultHeight);
	m_pen.setColor(defaultPenColor);
	m_brush.setColor(defaultBrushColor);
	m_brush.setStyle(Qt::SolidPattern);
	m_elements.setAutoDelete(false);
	m_selected.setAutoDelete(false);
	
}

Diagram::~Diagram()
{
	m_selected.clear();
	m_elements.setAutoDelete(true);
	m_elements.clear();
}


void Diagram::fillContextMenu(QPopupMenu &menu) const
{
	menu.insertItem(i18n("Create View"),this,SLOT(createView()));
	menu.insertItem(i18n("Diagram Properties"),this,SLOT(properties()));
}


DiagramView* Diagram::createView( QWidget *parent )
{
	DiagramView *view = new DiagramView(this,parent,"diagram_view");
	if(!parent)
		view->show();
	return view;
}

DiagramView* Diagram::createView()
{
	return createView((QWidget*)0L);
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
	DiagramElement *e(0);
	for( e = m_selected.first(); e; e = m_selected.next())
	{
		e->setSelected(false);
	}
}

void Diagram::setItemsSelected(const QRect &rect, bool selected)
{
	QCanvasItemList list = collisions(rect);
	QCanvasItemList::iterator it;
	for(it = list.begin(); it != list.end(); ++it)
	{
		(*it)->setSelected(selected);
	}
}

void Diagram::moveSelectedBy(int x, int y)
{
	DiagramElement *e(0);
	for( e = m_selected.first(); e; e = m_selected.next())
	{
		e->moveBy(x,y);
	}
}


bool Diagram::acceptType(const std::type_info &type)
{
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
	DiagramWidget* w = WidgetFactory::instance()->createWidget(o,this);
	if(w)
	{
		w->moveAbs(e->pos().x(),e->pos().y());
		w->show();
		update();
	}
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
}

void Diagram::registerElement( DiagramElement *e )
{
	m_elements.append(e);
	connect(e,SIGNAL(selected(bool)),this,SLOT(elementSelected(bool)));
	connect(e,SIGNAL(destroyed()),this,SLOT(elementDestroyed()));
}

void Diagram::elementDestroyed()
{
	const DiagramElement* e = dynamic_cast<const DiagramElement*>(sender());
	if(!e)
		return;
	m_elements.removeRef(e);
	m_selected.removeRef(e);
}


void Diagram::elementSelected(bool s)
{
	const DiagramElement* e = dynamic_cast<const DiagramElement*>(sender());
	if(!e)
		return;
	if(s)
	{
		m_selected.append(e);
	}
	else
	{
		m_selected.removeRef(e);
	}
}

UMLDoc* Diagram::document() const
{
	return m_doc;
}

DiagramElement* Diagram::firstDiagramElement( const QPoint &pos )
{
	QCanvasItemList list = collisions(pos);
	DiagramElement *element(0);
	Path *path(0);
	PathSegment *segment(0);
	//give priority to selected items
	if(m_selected.count() == 1 && list.find(m_selected.first()) != list.end() )
	{
		return m_selected.first();
	}
	QCanvasItemList::Iterator it = list.begin();
	for( ; it != list.end(); ++it)
	{
		path = dynamic_cast<Path*>(*it);
		if( path )
			continue; //we need to check for collisions with the segments instead of the path
		segment = dynamic_cast<PathSegment*>(*it);
		if( segment )
		{
			element = segment->path();
			break;
		}
		element = dynamic_cast<DiagramElement*>(*it);
		if( element )
			break;
	}
	return element;
}

DiagramWidget* Diagram::firstDiagramWidget( const QPoint &pos )
{
	QCanvasItemList list = collisions(pos);
	DiagramWidget *widget(0);
	//TEST
	if(m_selected.count() == 1 && 
	   list.find(m_selected.first()) != list.end() && 
	   dynamic_cast<DiagramWidget*>(m_selected.first()))
	{
		return dynamic_cast<DiagramWidget*>(m_selected.first());
	}
	//endtest
	QCanvasItemList::Iterator it = list.begin();
	for( ; it != list.end(); ++it)
	{
		//neither paths nor segments are diagramwidgets, so a simple test does it
		widget = dynamic_cast<DiagramWidget*>(*it);
		if( widget )
			break;
	}
	return widget;
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
