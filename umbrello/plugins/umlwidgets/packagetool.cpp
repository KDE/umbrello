  /***************************************************************************
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


#include "packagetool.h"
#include "packagewidget.h"


#include "../../diagram/diagramview.h"
#include "../../diagram/diagram.h"
#include "../../diagram/diagramelement.h"

#include "../../package.h"
#include "../../umldoc.h"

#include <qcursor.h>
#include <qevent.h>
#include <klocale.h>
#include <qpoint.h>
#include <kdebug.h>

#include <typeinfo>

namespace Umbrello
{


PackageTool::PackageTool( DiagramView  *view ):
	Tool( view, "packagetool" )
{
	setIcon("package");
	setToolTip(i18n("Creates UML::Package objects"));
}

PackageTool::~PackageTool()
{
}

void PackageTool::activate()
{
	setCursor( );
	kdDebug()<<"PackageTool is now active"<<endl;
}

void PackageTool::deactivate()
{
	kdDebug()<<"Deactivating PackageTool"<<endl;
}

void PackageTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}


bool PackageTool::mouseReleaseEvent( )
{
	if( buttonPressed() != Qt::LeftButton)
	{
		return false;
	}

	DiagramElement *e = diagram()->firstDiagramElement( currentPos() );
	if( e )
	{
		kdDebug()<<"PackageTool: cannot create widget on top of another widget"<<endl;
		return false;
	}
	if( !diagram()->acceptType( typeid(UMLPackage) ) )
	{
		kdWarning()<<"PackageTool: request to create object of type \"UMLPackage\" rejected by diagram"<<endl;
		return false;
	}
	UMLObject *obj = diagram()->document()->createUMLObject( typeid(UMLPackage) );
	if(!obj)
	{
		kdWarning()<<"document failed to create object of type \"UMLPackage\""<<endl;
		return false;
	}
	DiagramElement *w = new PackageWidget(diagram(),diagram()->document()->getUniqueID(),
	                                    static_cast<UMLPackage*>(obj));

	w->moveAbs(currentPos().x(),currentPos().y());
	w->show();
	diagram()->update();
	return true;
}


}

