/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 #include "toolfactory.h"
 #include "tool.h"
 #include "selecttool.h"
 #include "widgetset.h"
 
 #include <kdebug.h>
 
 namespace Umbrello{
 
 ToolFactory* ToolFactory::instance()
 {
	if( !s_instance )
		s_instance = new ToolFactory();
	return s_instance;
 }
 
 ToolFactory::ToolFactory()
 {
 }
 
 ToolFactory::~ToolFactory()
 {
 }
 
 void ToolFactory::registerWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"ToolFactory: Appending widget set "<<set->instanceName()<<" to list of known sets"<<endl;
 	widgetSets.append( set );
 }
 
 void ToolFactory::removeWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"ToolFactory: Removing widget set "<<set->instanceName()<<" from list of known sets"<<endl;
 	widgetSets.remove( set );
 }
 
 void ToolFactory::setDefaultWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"ToolFactory: setting "<<(set? (const char*)set->instanceName() : "null")<<" as default widget set"<<endl;
	m_defaultSet = set;
 }
 
 Tool* ToolFactory::createTool( const QString &tool, DiagramView *view )
 {
	Tool *t(0);
	for( WidgetSetList::Iterator it = widgetSets.begin(); it != widgetSets.end(); ++it )
	{
		if((*it)->providesTool( tool ))
		{
			t = (*it)->createTool( tool, view );
			return t;
		}
	}
	if( m_defaultSet && m_defaultSet->providesTool( tool ) )
	{
		t = m_defaultSet->createTool(tool,view);
		return t;
	}
		
	kdWarning()<<"ToolFactory::createTool: tool "<<tool<<" not provided by any widget set"<<endl;
	return t;
 }
 
 ToolFactory* ToolFactory::s_instance = 0L;
 }
 
