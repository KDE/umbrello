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


#include "widgetset.h"
#include "toolfactory.h"
#include "widgetfactory.h"

#include <kdebug.h>

namespace Umbrello{

WidgetSet::WidgetSet(QObject *parent, const char *name, const QStringList &args):
           Plugin(parent,name,args)
{

}

WidgetSet::~WidgetSet()
{
}

bool WidgetSet::onInit()
{
	ToolFactory::instance()->registerWidgetSet( this );
	WidgetFactory::instance()->registerWidgetSet( this );
	return true;
}


 bool WidgetSet::onShutdown()
 {
 	ToolFactory::instance()->removeWidgetSet( this );
	WidgetFactory::instance()->removeWidgetSet( this );
	return true;
 }

bool WidgetSet::providesTool( const QString & )
{
	return false;
}


Tool* WidgetSet::createTool(const QString& /*tool*/, DiagramView* /*view*/)
{
	kdWarning()<<"WidgetSet::createTool( ) called - "
	           <<"This method should be reimplemented in WidgetSet's derived calsses"<<endl;
	return (Tool*)0L;
}

bool WidgetSet::canCreateWidget( UMLObject *)
{
	return false;
}

DiagramWidget* WidgetSet::createWidget( UMLObject*, Diagram* )
{
	kdWarning()<<"WidgetSet::createWidget( ) called - "
	           <<"This method should be reimplemented in WidgetSet's derived calsses"<<endl;
	return (DiagramWidget*)0L;

}

}

#include "widgetset.moc"
