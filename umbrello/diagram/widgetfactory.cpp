/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgetfactory.h"

#include <typeinfo>

#include <qptrlist.h>
#include <qpoint.h>

#include "diagram.h"
#include "diagramelement.h"
#include "umlwidget.h"
#include "classwidget.h"
#include "actorwidget.h"
#include "associationwidget.h"

#include "../umldoc.h"
#include "../class.h"
#include "../interface.h"
#include "../package.h"
#include "../template.h"
#include "../actor.h"
#include "../usecase.h"
#include "../association.h"


using namespace std;

namespace Umbrello{



DiagramElement* WidgetFactory:: createUMLWidget( UMLObject *obj , Diagram *parent )
{
	DiagramElement *w(0L);
	int id = parent->document()->getUniqueID();
	
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

DiagramElement* WidgetFactory::createAssociationWidget( UMLAssociation *assoc, UMLWidget *wA, UMLWidget *wB, const QPtrList<QPoint> &path, Diagram *parent )
{
	int id = parent->document()->getUniqueID();
	DiagramElement *e = new AssociationWidget(parent, id, assoc, wA, wB);
	return e;
}

DiagramElement* WidgetFactory::createCustomWidget(int t, Diagram *parent)
{
	kdDebug()<<"custom widgets not yet implemented"<<t<<endl;
	return (DiagramElement*) 0L;
}

} // end of namespace Umbrello

#include "widgetfactory.moc"


