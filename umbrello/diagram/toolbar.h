  /***************************************************************************
                               toolbar.h
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

#ifndef UMBRELLO_TOOLBAR
#define UMBRELLO_TOOLBAR

#include <ktoolbar.h>
#include <qmap.h>

namespace Umbrello
{
class Tool;
class DiagramView;


class ToolBar : public KToolBar
{
Q_OBJECT
public:
	typedef QMap<int,Tool*> ToolMap;
	ToolBar( QMainWindow *parentWindow, DiagramView *view, bool newLine = false, 
	        const char *name = 0, bool honor_style = false, bool readConfig = true);
	ToolBar( QMainWindow *parentWindow, DiagramView *view, QMainWindow::Dock, bool newLine = false, 
	        const char *name = 0, bool honor_style = false, bool readConfig = true);
	virtual ~ToolBar();
	
	void setCurrentTool( const QString &name );
	Tool* currentTool( ) const;
	
	void showTools( const QString &category, bool exclusive = false );
	void hideTools( const QString &category );
	void showAllTools( );
	void hideAllTools( );	
signals:
	void toolChanged( Tool* );

protected slots:
	void buttonClicked(int);
	
protected:
	DiagramView *m_view;
	ToolMap m_toolMap;
private:
	void init();
	int m_nextID;
	int m_currentTool;
	
};


}

#endif //UMBRELLO_TOOLBAR
