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


class UMLObject;

namespace Umbrello{

class Diagram;
class DiagramElement;

class WidgetFactory :  public QObject
{Q_OBJECT

friend class Diagram;
	
protected:
	static DiagramElement* createUMLWidget( UMLObject* , Diagram *parent );
	static DiagramElement* createCustomWidget( int type, Diagram *parent );
	
};


} // end of namespace Umbrello

#endif  //  WIDGET_FACTORY_H
