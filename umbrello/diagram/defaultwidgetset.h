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

#ifndef UMBRELLO_DEFAULT_WIDGETSET_H
#define UMBRELLO_DEFAULT_WIDGETSET_H

#include "widgetset.h"

class UMLObject;

namespace Umbrello{
class Diagram;
class DiagramView;
class Tool;
/** This widget sets provides a default "unknown" widget which can be used when no other
 * widget is available. It also provides a select tool which is capable of handling all
 * the basic functions like moving DiagramElement's around and selecting them
 */
class DefaultWidgetSet :  public WidgetSet
{
    Q_OBJECT
public:
	DefaultWidgetSet(QObject *parent, const QStringList &args);
	virtual ~DefaultWidgetSet();
	
	virtual bool providesTool( const QString &tool );
	virtual Tool* createTool( const QString &tool, DiagramView *view );

	virtual bool canCreateWidget( UMLObject *obj );
	virtual DiagramWidget* createWidget( UMLObject *obj, Diagram* diagram );
	
	bool init() { return onInit(); }
	
protected:
	

	
	/** reimplemented to define plugin specific startup behavior.
	 * reimplemented from WidgetSet - we need to register ourselves
	 * as a "special" plugin to be used only when all others fail */
	virtual bool onInit();

	/** reimplemented to define plugin specific shutdown behavior 
	 * reimplemented from WidgetSet - we need to register ourselves
	 * as a "special" plugin to be used only when all others fail */
	virtual bool onShutdown();
}; 


} // end of namespace Umbrello

#endif  //  UMBRELLO_DEFAULT_WIDGETSET_H
