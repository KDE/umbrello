/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_WIDGETSET_H
#define UMBRELLO_WIDGETSET_H

#include "../plugin.h"

class UMLObject;

namespace Umbrello{
class Diagram;
class DiagramView;
class DiagramWidget;
class Tool;
/** A WidgetSet is a widget (graphical object) provider for a Diagram - 
 * each widget provides one or more widgets, normally belonging to one category 
 * ,for example "UML Widgets" for representing objects in a UML Modell.
 * A WidgetSet should also provide tools to create and manage its widgets
 * If the Set creates/manages objects which are part of the modell (and not only
 * SimpleWidgets, it should also provide a factory function - this takes a void* 
 * so you have to make shure you are getting the right type by usisng typeinfo()
 */
class WidgetSet :  public Plugin
{Q_OBJECT
public:
	
	virtual QString category() { return QString("widget_set");}
	
	virtual bool providesTool( const QString &tool );
	virtual Tool* createTool( const QString &tool, DiagramView *view );

	virtual bool canCreateWidget( UMLObject *obj );
	virtual DiagramWidget* createWidget( UMLObject* obj, Diagram* );
	
protected:
	WidgetSet(QObject *parent, const char *name, const QStringList &args);
	virtual ~WidgetSet();
	
	/** reimplemented to define plugin specific startup behavior.
	 * This implementation registers the plug in to the ToolFactory -
	 * if your WidgetSet provides tools and you reimplement this method,
	 * make sure to register yourself or call this implementation */
	virtual bool onInit();

	/** reimplemented to define plugin specific shutdown behavior 
	 * This implementation removes the plug from the ToolFactory -
	 * if your WidgetSet provides tools and you reimplement this method,
	 * make sure to register yourself or call this implementation */
	virtual bool onShutdown();
}; 


} // end of namespace Umbrello

#endif  //  UMBRELLO_WIDGETSET_H
