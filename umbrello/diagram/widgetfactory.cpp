/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 #include "widgetfactory.h"
 #include "widgetset.h"
 #include "diagramwidget.h"
 
 #include <kdebug.h>
 
 namespace Umbrello{
 
 WidgetFactory* WidgetFactory::instance()
 {
	if( !s_instance )
		s_instance = new WidgetFactory();
	return s_instance;
 }
 
 WidgetFactory::WidgetFactory()
 {
 }
 
 WidgetFactory::~WidgetFactory()
 {
 }
 
 void WidgetFactory::registerWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"WidgetFactory: Appending widget set "<<set->instanceName()<<" to list of known sets"<<endl;
 	widgetSets.append( set );
 }
 
 void WidgetFactory::removeWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"WidgetFactory: Removing widget set "<<set->instanceName()<<" from list of known sets"<<endl;
 	widgetSets.remove( set );
 }
 
 void WidgetFactory::setDefaultWidgetSet( WidgetSet *set )
 {
 	kdDebug()<<"WidgetFactory: setting "<<(set? (const char*)set->instanceName() : "null")<<" as default widget set"<<endl;
	m_defaultSet = set;
 }
 
 bool WidgetFactory::canCreateWidget( UMLObject *obj )
 {
	for( WidgetSetList::Iterator it = widgetSets.begin(); it != widgetSets.end(); ++it )
	{
		if((*it)->canCreateWidget( obj ))
		{
			return true;
		}
	}
	if( m_defaultSet && m_defaultSet->canCreateWidget( obj ) )
	{
		return true;
	}
	return false;
 }
 DiagramWidget* WidgetFactory::createWidget( UMLObject *obj, Diagram *diagram )
 {
	DiagramWidget *w(0);
	for( WidgetSetList::Iterator it = widgetSets.begin(); it != widgetSets.end(); ++it )
	{
		if((*it)->canCreateWidget( obj ))
		{
			w = (*it)->createWidget( obj, diagram );
			return w;
		}
	}
	if( m_defaultSet && m_defaultSet->canCreateWidget( obj ) )
	{
		w = m_defaultSet->createWidget(obj, diagram);
		return w;
	}
		
	kdWarning()<<"WidgetFactory::createWidget:"<<obj<<" not provided by any widget set"<<endl;
	return w;
 }
 
 WidgetFactory* WidgetFactory::s_instance = 0L;
 }
 
