/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIDGET_SET_H
#define WIDGET_SET_H

#include "../plugin.h"
#include <qpointarray.h>


class UMLObject;
class UMLAssociation;
class QPoint;
class QPointArray;

namespace Umbrello{

class Diagram;
class DiagramElement;
class UMLWidget;

class WidgetSet :  public Plugin
{Q_OBJECT
public:
	
	virtual QString category() { return QString("widget_set");}
	
	/**Returns true if the WidgetSet can/wants to handle the creation of a widget corresponding
	* to the UMLObject */
	virtual bool acceptsRequest(UMLObject* ) {return false;}
	/**Returns true if the WidgetSet can/wants to handle the creation of a "CustomeWidget" of
	* type  t */
	virtual bool acceptsRequest(int) {return false;}
	
	virtual DiagramElement* createUMLWidget( UMLObject* , Diagram *parent ) = 0;
	virtual DiagramElement* createCustomWidget( int type, Diagram *parent ) = 0;
	virtual DiagramElement* createAssociationWidget( UMLAssociation*, UMLWidget *wA, UMLWidget *wB,const QPointArray &path, Diagram *parent ) = 0;
protected:
	WidgetSet(QObject *parent, const char *name, const QStringList &args);
	virtual ~WidgetSet();
};


} // end of namespace Umbrello

#endif  //  WIDGET_SET_H
