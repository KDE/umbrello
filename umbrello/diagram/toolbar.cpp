/***************************************************************************
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

#include "toolbar.h"
#include "tool.h"
#include "toolfactory.h"
#include "diagramview.h"

#include <kdebug.h>

namespace{

}

namespace Umbrello
{


ToolBar::ToolBar( QMainWindow *parentWindow, DiagramView *view, bool newLine,
		const char *name, bool honor_style, bool readConfig):
		KToolBar( parentWindow, view , newLine, name, honor_style, readConfig ),
		m_view(view),m_currentTool(0)
{
	init();
}

ToolBar::ToolBar( QMainWindow *parentWindow, DiagramView *view, QMainWindow::Dock dock, bool newLine,
		const char *name, bool honor_style, bool readConfig):
		KToolBar( parentWindow, dock , newLine, name, honor_style, readConfig ),
		m_view(view),m_currentTool(0)
{
	init();
}

void ToolBar::init()
{
	m_nextID = 1;
	connect(this,SIGNAL(released(int)),this,SLOT(buttonClicked(int)));
	setMovingEnabled(true);
}


ToolBar::~ToolBar()
{
}

void ToolBar::setCurrentTool( const QString &name )
{
	ToolMap::Iterator end(m_toolMap.end());
	for(ToolMap::Iterator it(m_toolMap.begin()); it != end; ++it )
	{
		if((*it)->name() == name )
		{
			setButton(m_currentTool,false);
			setButton(it.key(),true);
			//simulate user click
			buttonClicked(it.key());
		}
	}
}

Tool* ToolBar::currentTool( ) const
{
	if( m_currentTool )
		return m_toolMap[m_currentTool];
	else
		return (Tool*)0L;
}

void ToolBar::buttonClicked(int t)
{
	if( t == m_currentTool)
	{
		setButton(m_currentTool,true);
		return; //the only way to deselect the current tool is by selecting another tool
	}

	setButton(m_currentTool,false);

	m_currentTool = t;
	emit toolChanged(m_toolMap[t]);
}

void ToolBar::showTools( const QString &category, bool exclusive )
{
//FIXME - make this category based and not tool-based
	if( exclusive )
	{
	        ToolMap::Iterator end(m_toolMap.end());
		for(ToolMap::Iterator it(m_toolMap.begin()); it != end; ++it )
		{
			hideItem( it.key() );
		}
	}
	//FIXME dont create - first look if we already have this, and only create
	// if we dont - right now this can only be called once per tool
	Tool *tool = ToolFactory::instance()->createTool( category, m_view );
	if(tool)
	{
		int id = ++m_nextID;
		insertButton( tool->icon(), id, true, tool->toolTip() );
		setToggle(id,true);
		m_toolMap[id] = tool;
	}
}

void ToolBar::hideTools(const QString& /*category*/)
{
	kdWarning()<<"hideTools -  not implemented"<<endl;
}
void ToolBar::showAllTools( )
{
	kdWarning()<<"showAllTools -  not implemented"<<endl;
}
void ToolBar::hideAllTools( )
{
	kdWarning()<<"hideAllTools -  not implemented"<<endl;
}



}

#include "toolbar.moc"
