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

#ifndef UMBRELLO_UMLWIDGETSET_H
#define UMBRELLO_UMLWIDGETSET_H


#include "../../diagram/widgetset.h"

class UMLObject;

namespace Umbrello{
class Diagram;
class DiagramView;
class Tool;

/** A set of widgets to display UML Objects 
  * @author Luis De la Parra*/
class UMLWidgetSet :  public WidgetSet
{
public:
	UMLWidgetSet(QObject *parent, const char*, const QStringList &args);
	virtual ~UMLWidgetSet();
	
	virtual bool providesTool( const QString &tool );
	virtual Tool* createTool( const QString &tool, DiagramView *view );

	virtual bool canCreateWidget( UMLObject *obj );
	virtual DiagramWidget* createWidget( UMLObject *obj, Diagram *diagram );
	
private:
       virtual bool configure() {return true;}
};


} // end of namespace Umbrello

#endif  //  UMBRELLO_UMLWIDGETSET_H
