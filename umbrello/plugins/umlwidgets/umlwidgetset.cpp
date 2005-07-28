 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlwidgetset.h"
#include "classtool.h"
#include "classwidget.h"
#include "interfacewidget.h"
#include "interfacetool.h"
#include "packagewidget.h"
#include "packagetool.h"
#include "umlassociationtool.h"
#include "../../diagram/toolfactory.h"
#include "../../diagram/widgetfactory.h"
#include "../../class.h"
#include "../../interface.h"
#include "../../package.h"
#include "../../umldoc.h"

#include <qstring.h>
#include <q3pointarray.h>
#include <q3ptrlist.h>
#include <qpoint.h>

#include <kdebug.h>

#include <typeinfo>



UMBRELLO_EXPORT_PLUGIN_FACTORY(libumlwidgets, KGenericFactory<Umbrello::UMLWidgetSet>)


namespace Umbrello{

UMLWidgetSet::UMLWidgetSet(QObject *parent, const char *c, const QStringList &args):
	WidgetSet(parent, "umlwidgetset", args)
{
	kdDebug()<<"UMLWidgetSet: passing \"umlwidgetset\" to parent, received \""<<c<<"\""<<endl;

}

UMLWidgetSet::~UMLWidgetSet() {}

bool UMLWidgetSet::providesTool( const QString &tool )
{
	if( tool == QString("classtool") || tool == QString("interfacetool") ||
	    tool == QString("packagetool") || tool == QString("generalizationtool") ||
	    tool == QString("compositiontool") || tool == QString("aggregationtool") ||
	    tool == QString("dependencytool") || tool == QString("associationtool") ||
	    tool == QString("uniassociationtool") )
		return true;
	return false;
}

Tool* UMLWidgetSet::createTool( const QString &tool, DiagramView *view )
{
	Tool *t(0);
	if( tool == QString("classtool") )
	{
		t = new ClassTool( view );
		return t;
	}
	if( tool == QString("interfacetool") )
	{
		t = new InterfaceTool( view );
		return t;
	}
	if( tool == QString("packagetool") )
	{
		t = new PackageTool( view );
		return t;
	}
	if( tool == QString("generalizationtool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::Generalization );
		return t;
	}
	if( tool == QString("compositiontool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::Composition );
		return t;
	}
	if( tool == QString("aggregationtool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::Aggregation );
		return t;
	}
	if( tool == QString("dependencytool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::Dependency );
		return t;
	}
	if( tool == QString("associationtool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::Association );
		return t;
	}
	if( tool == QString("uniassociationtool") )
	{
		t = new UMLAssociationTool( view );
		static_cast<UMLAssociationTool*>(t)->setType( UMLAssociationTool::UniDiAssociation );
		return t;
	}
	kdWarning()<<"UMLWidgetSet: request to create tool "<<tool<<" is not handled by this widget set"<<endl;
	return t;
}

bool UMLWidgetSet::canCreateWidget( UMLObject *obj )
{
	if(dynamic_cast<UMLClass*>(obj) || dynamic_cast<UMLInterface*>(obj) ||
	   dynamic_cast<UMLPackage*>(obj))
		return true;
	return false;
}

DiagramWidget* UMLWidgetSet::createWidget( UMLObject *obj, Diagram *diagram )
{
	DiagramWidget *w(0);
	if(dynamic_cast<UMLClass*>(obj))
	{
		w = new ClassWidget( diagram,
		                    diagram->document()->getUniqueID(),
				    static_cast<UMLClass*>(obj) );
		return w;
	}
	else if(dynamic_cast<UMLInterface*>(obj))
	{
		w = new InterfaceWidget( diagram,
		                    diagram->document()->getUniqueID(),
				    static_cast<UMLInterface*>(obj) );
		return w;
	}
	else if(dynamic_cast<UMLPackage*>(obj))
	{
		w = new PackageWidget( diagram,
		                    diagram->document()->getUniqueID(),
				    static_cast<UMLPackage*>(obj) );
		return w;
	}
	//else....

	kdWarning()<<"UMLWidgetSet::createWidget( ) called for object of unrecognized type "<<typeid(*obj).name()<<endl;
	return w;
}

}
//#include "umlwidgetset.moc"


