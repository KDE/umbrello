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

#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#include <qobject.h>
#include <qvaluelist.h>

class UMLObject;

namespace Umbrello{

class Diagram;
class DiagramElement;
class DiagramWidget;

class WidgetSet;

class WidgetFactory : public QObject
{Q_OBJECT
public:
	typedef QValueList<WidgetSet*> WidgetSetList;
	
	static WidgetFactory* instance();
	virtual ~WidgetFactory();
	
	virtual bool canCreateWidget( UMLObject *obj );
	virtual DiagramWidget* createWidget( UMLObject*, Diagram* );
	
	void registerWidgetSet( WidgetSet* );
	void removeWidgetSet( WidgetSet* );
	void setDefaultWidgetSet( WidgetSet* );
private:
	WidgetFactory();
	WidgetSetList widgetSets;
	WidgetSet* m_defaultSet;
	static WidgetFactory *s_instance; 
	
};


} // end of namespace Umbrello

#endif  //  WIDGET_FACTORY_H
