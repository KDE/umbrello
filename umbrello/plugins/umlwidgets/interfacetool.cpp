  /***************************************************************************
                               interfacetool.cpp
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


#include "interfacetool.h"
#include "interfacewidget.h"


#include "../../diagram/diagramview.h"
#include "../../diagram/diagram.h"
#include "../../diagram/diagramelement.h"

#include "../../interface.h"
#include "../../umldoc.h"

#include <qcursor.h>
#include <qevent.h>
#include <klocale.h>
#include <qpoint.h>
#include <kdebug.h>

#include <typeinfo>

namespace Umbrello
{


InterfaceTool::InterfaceTool( DiagramView  *view ):
	Tool( view, "interfacetool" )
{
	setIcon("interface");
	setToolTip(i18n("Creates UML::Interface objects"));
}

InterfaceTool::~InterfaceTool()
{
}

void InterfaceTool::activate()
{
	setCursor( );
	kdDebug()<<"InterfaceTool is now active"<<endl;
}

void InterfaceTool::deactivate()
{
	kdDebug()<<"Deactivating InterfaceTool"<<endl;
}

void InterfaceTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}


bool InterfaceTool::mouseReleaseEvent( )
{
	if( buttonPressed() != Qt::LeftButton)
	{
		return false;
	}

	DiagramElement *e = diagram()->firstDiagramElement( currentPos() );
	if( e )
	{
		kdDebug()<<"InterfaceTool: cannot create widget on top of another widget"<<endl;
		return false;
	}
	if( !diagram()->acceptType( typeid(UMLInterface) ) )
	{
		kdWarning()<<"InterfaceTool: request to create object of type \"UMLInterface\" rejected by diagram"<<endl;
		return false;
	}
	UMLObject *obj = diagram()->document()->createUMLObject( typeid(UMLInterface) );
	if(!obj)
	{
		kdWarning()<<"document failed to create object of type \"UMLInterface\""<<endl;
		return false;
	}
	DiagramElement *w = new InterfaceWidget(diagram(),diagram()->document()->getUniqueID(),
	                                    static_cast<UMLInterface*>(obj));

	w->moveAbs(currentPos().x(),currentPos().y());
	w->show();
	diagram()->update();
	return true;
}


}

