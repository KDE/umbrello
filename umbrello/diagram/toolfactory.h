/***************************************************************************
                               toolfactory.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_TOOLFACTORY_H
#define UMBRELLO_TOOLFACTORY_H

#include <q3valuelist.h>

namespace Umbrello
{
class Tool;
class WidgetSet;
class DiagramView;


class ToolFactory
{
public:
	typedef Q3ValueList<WidgetSet*> WidgetSetList;
	
	static ToolFactory* instance();
	virtual ~ToolFactory();
	
	Tool* createTool( const QString &tool, DiagramView *view );
	

	void registerWidgetSet( WidgetSet* );
	void removeWidgetSet( WidgetSet* );
	void setDefaultWidgetSet( WidgetSet* );
private:
	ToolFactory();
	WidgetSetList widgetSets;
	WidgetSet* m_defaultSet;
	static ToolFactory* s_instance;
};


}


#endif //UMBRELLO_TOOLFACTORY_H
