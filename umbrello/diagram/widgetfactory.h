/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <qobject.h>
#include <qvaluelist.h>


class UMLObject;
class UMLAssociation;
class QPoint;
class QPointArray;

namespace Umbrello{

class Diagram;
class DiagramElement;
class UMLWidget;
class WidgetSet;

class WidgetFactory : public QObject
{Q_OBJECT
public:
	typedef QValueList<WidgetSet*> WidgetSetList;
	
	static WidgetFactory* instance();
	virtual ~WidgetFactory();
	
	DiagramElement* createUMLWidget( UMLObject* , Diagram *parent );
	DiagramElement* createCustomWidget( int type, Diagram *parent );
	DiagramElement* createAssociationWidget( UMLAssociation*, UMLWidget *wA, UMLWidget *wB,const QPointArray &path, Diagram *parent );
private:
	WidgetFactory();
	WidgetSetList widgetSets;
	static WidgetFactory* _instance; 
	
};


} // end of namespace Umbrello

#endif  //  WIDGET_FACTORY_H
