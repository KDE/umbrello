/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgetfactory.h"
#include "widgetset.h"

#include <typeinfo>


#include <qpointarray.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <kapplication.h>
#include <kconfig.h>

#include "diagram.h"
#include "diagramelement.h"
#include "umlwidget.h"
#include "classwidget.h"
#include "actorwidget.h"
#include "associationwidget.h"

#include "../uml.h"
#include "../umldoc.h"
#include "../class.h"
#include "../interface.h"
#include "../package.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"
#include "../association.h"

#include "../pluginloader.h"
#include "../configurable.h"

#include <kdebug.h>

using namespace std;

namespace Umbrello{
class DummyConfig: public Configurable
{
	virtual bool configure(){return false;}
};

class DummyWidgetSet: public WidgetSet
{public:
	DummyWidgetSet(QObject *parent=0, const char *name=0, const QStringList &args = 0L):WidgetSet(parent,name,args){};
	virtual DiagramElement* createUMLWidget( UMLObject* , Diagram *parent ){ return (DiagramElement*)0L;};
	virtual DiagramElement* createCustomWidget( int type, Diagram *parent ){return (DiagramElement*)0L;};
	virtual DiagramElement* createAssociationWidget( UMLAssociation*, UMLWidget *wA, UMLWidget *wB,const QPointArray &path, Diagram *parent ){return (DiagramElement*)0L;};
};
}



namespace Umbrello{

WidgetFactory* WidgetFactory::instance()
{
	if( ! _instance )
		_instance = new WidgetFactory();
	return _instance;
}

WidgetFactory::WidgetFactory()
{
////////////////////
DummyConfig *uc = new DummyConfig();
	delete uc;
DummyWidgetSet *dws = new DummyWidgetSet( );
	delete dws;
	
	KConfig * config = kapp->config();
	config->setGroup("Plugin");
	QStringList names = config->readListEntry("widget_sets");
	for(uint i = 0; i != names.size(); i++) 
	{
		const QString &name = names[i];
		kdDebug() << "loading plugin " << name << endl;

    // load the plugin
		Plugin *plugin = PluginLoader::instance()->loadPlugin(name);
		if(plugin)
			widgetSets.append(static_cast<WidgetSet*>(plugin));
	}
////////////////////////
	//PluginLoader *loader = PluginLoader::instance();
	//PluginLoader::CategoryMap map = loader->categories();
	//kdDebug()<<"initializing WidgetFactory. categories map contains "<<map.size()<<"entries"<<endl;
	//if( map.contains( "widget_set" ) )
	//{kdDebug()<<"adding widget sets"<<endl;
	//	 PluginLoader::PluginList list = map["widget_set"];
	//	 for( PluginLoader::PluginList::Iterator it = list.begin(); it != list.end() ; ++it )
		 //{kdDebug()<<"set added"<<endl;
		 	//widgetSets.append(static_cast<WidgetSet*>(*it));
		 //}
	//}
}

WidgetFactory::~WidgetFactory() {}


DiagramElement* WidgetFactory:: createUMLWidget( UMLObject *obj , Diagram *parent )
{
	DiagramElement *w(0L);

	int id = UMLApp::app()->getDocument()->getUniqueID();
	for( WidgetSetList::Iterator it = widgetSets.begin(); it != widgetSets.end(); ++it )
	{
		if((*it)->acceptsRequest( obj ))
		{	kdDebug()<<"create uml widget request handled by plugin"<<endl;
			w = (*it)->createUMLWidget( obj, parent );
			return w;
		}
	}
	kdDebug()<<"create uml widget request not accepted by any plug in, usisng built-in set"<<endl;
	//use the default widget set -- maybe move this into a plug in as well...
	type_info &type = typeid(*obj);
	if( type == typeid(UMLClass) )
	{
		w = new ClassWidget(parent,id,dynamic_cast<UMLClass*>(obj));
	}
	else if ( type == typeid(UMLInterface) )
	{
	//	w = new InterfaceWidget(parent,id,dynamic_cast<UMLInterface*>(obj));
	}
	else if ( type == typeid(UMLActor) )
	{
		w = new ActorWidget(parent,id,dynamic_cast<UMLActor*>(obj));
	}
	//else if ( type == typeid(...) )
	else
	{
		kdDebug()<<"Widget fot type "<<type.name()<<" not yet implemented"<<endl;
	}
	return w;

}

DiagramElement* WidgetFactory::createAssociationWidget( UMLAssociation *assoc, UMLWidget *wA, UMLWidget *wB, const QPointArray &path, Diagram *parent )
{
	int id = parent->document()->getUniqueID();
	AssociationWidget *assocW = new AssociationWidget(parent, id, assoc, wA, wB);
	assocW->setPath( path );
	return assocW;
}

DiagramElement* WidgetFactory::createCustomWidget(int t, Diagram *parent)
{
	kdDebug()<<"custom widgets not yet implemented"<<t<<endl;
	return (DiagramElement*) 0L;
}

WidgetFactory* WidgetFactory::_instance = 0L; 
} // end of namespace Umbrello

#include "widgetfactory.moc"


