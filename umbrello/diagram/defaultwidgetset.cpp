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


#include "defaultwidgetset.h"
#include "toolfactory.h"
#include "widgetfactory.h"

#include "unknownumlwidget.h"
#include "selecttool.h"
#include "pathtool.h"
#include "associationtool.h"
#include "diagramwidget.h"
#include "diagram.h"
#include "../umlobject.h"
#include "../umldoc.h"

#include <kdebug.h>

namespace Umbrello{

DefaultWidgetSet::DefaultWidgetSet(QObject *parent, const QStringList &args):
           WidgetSet(parent,"DefaultWidgetSet",args)
{

}

DefaultWidgetSet::~DefaultWidgetSet()
{
}

bool DefaultWidgetSet::onInit()
{
	ToolFactory::instance()->setDefaultWidgetSet( this );
	WidgetFactory::instance()->setDefaultWidgetSet( this );
	return true;
}


 bool DefaultWidgetSet::onShutdown()
 {
 	ToolFactory::instance()->setDefaultWidgetSet( (WidgetSet*)0L );
	WidgetFactory::instance()->setDefaultWidgetSet( (WidgetSet*)0L );
	return true;
 }

bool DefaultWidgetSet::providesTool( const QString &tool )
{
	if( tool == "selecttool" || tool == "pathtool"
	    ||  tool == "associationtool" )
		return true;
	else
		return false;
}

Tool* DefaultWidgetSet::createTool( const QString &tool, DiagramView *view )
{
	Tool *t(0);
	if( tool == "selecttool" )
	{
		t = new SelectTool(view);
	}
	else if( tool == "pathtool" )
	{
		t = new PathTool(view);
	}
	else if( tool == "associationtool" )
	{
		t = new AssociationTool(view);
	}
	else
	{
		kdWarning()<<"Request to create tool "<<tool<<" sent to DefaultWidgetSet."
		           <<" Use providesTool() to check before calling this method"<<endl;
	}
	return t;
}


bool DefaultWidgetSet::canCreateWidget( UMLObject *obj )
{
	//we can represent any uml object -
	if(dynamic_cast<UMLObject*>(obj))
		return true;
	return false;
}

DiagramWidget* DefaultWidgetSet::createWidget( UMLObject  *obj, Diagram *diagram )
{
	DiagramWidget *w(0);
	UMLObject *uml = dynamic_cast<UMLObject*>(obj);
	if(!uml)
		return w;
	kdWarning()<<"DefaultWidgetSet::createWidget( ) called - "
	           <<"This means your normal widget sets are not fully implemented - please FIX"<<endl;
	w = new UnknownUMLWidget( diagram, diagram->document()->getUniqueID(), obj );
	return w;
}

}


#include "defaultwidgetset.moc"
