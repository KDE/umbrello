  /***************************************************************************
                               classtool.cpp
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "classtool.h"
#include "classwidget.h"


#include "../../diagram/diagramview.h"
#include "../../diagram/diagram.h"
#include "../../diagram/diagramelement.h"

#include "../../class.h"
#include "../../umldoc.h"

#include <qcursor.h>
#include <qevent.h>
#include <klocale.h>
#include <qpoint.h>
#include <kdebug.h>

#include <typeinfo>

namespace Umbrello
{


ClassTool::ClassTool( DiagramView  *view ):
	Tool( view, "classtool" )
{
	setIcon("umlclass");
	setToolTip(i18n("Creates UML::Class objects"));
}

ClassTool::~ClassTool()
{
}

void ClassTool::activate()
{
	setCursor( );
	kdDebug()<<"ClassTool is now active"<<endl;
}

void ClassTool::deactivate()
{
	kdDebug()<<"Deactivating ClassTool"<<endl;
}

void ClassTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}


bool ClassTool::mouseReleaseEvent( )
{
	if( buttonPressed() != Qt::LeftButton)
	{
		return false;
	}

	DiagramElement *e = diagram()->firstDiagramElement( currentPos() );
	if( e )
	{
		kdDebug()<<"ClassTool: cannot create widget on top of another widget"<<endl;
		return false;
	}
	if( !diagram()->acceptType( typeid(UMLClass) ) )
	{
		kdWarning()<<"ClassTool: request to create object of type \"UMLClass\" rejected by diagram"<<endl;
		return false;
	}
	UMLObject *obj = diagram()->document()->createUMLObject( typeid(UMLClass) );
	if(!obj)
	{
		kdWarning()<<"document failed to create object of type \"UMLClass\""<<endl;
		return false;
	}
	DiagramElement *w = new ClassWidget(diagram(),diagram()->document()->getUniqueID(),
	                                    static_cast<UMLClass*>(obj));

	w->moveAbs(currentPos().x(),currentPos().y());
	w->show();
	diagram()->update();
	return true;
}


}

