/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WORKTOOLBAR_H
#define WORKTOOLBAR_H

#include <ktoolbar.h>
#include "umlnamespace.h"

#include <qmap.h>


/**
 *	This is the toolbar that is displayed on the right-hand side of the program
 *	window.  For each type of diagram it will change to suit that document.
 *
 *	@short	The toolbar that is different for each type of diagram.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version 1.0
 */
class WorkToolBar : public KToolBar {
	Q_OBJECT
public:
	/**
	 *	Creates a work tool bar
	 *
	 *	@param	parentWindow	The parent of the toolbar.
	 *	@param	name					The name of the toolbar.
	 */
	WorkToolBar(QMainWindow *parentWindow,const char*name);

	/** //luis
	 * Sets the current tool to the previously used Tool, this is just
	 * as if the user had pressed the button for the other tool
	 */
	void setOldTool();

	/**
	 * Sets the current tool to the default tool. (select tool)
	 * calling this function is as if the user had pressed the "arrow"
	 * button on the toolbar
	 */
	void setDefaultTool();

	/**
	 *	Standard deconstructor
	 */
	~WorkToolBar();


	enum ToolBar_Buttons
	{
	    tbb_Undefined = -1,
	    tbb_Arrow,
	    tbb_Generalization,
	    tbb_Aggregation,
	    tbb_Dependency,
	    tbb_Association,
	    tbb_Coll_Message,
	    tbb_Seq_Message,
	    tbb_Implementation,
	    tbb_Composition,
	    tbb_UniAssociation,
	    tbb_State_Transition,
	    tbb_Activity_Transition,
	    tbb_Anchor,//keep anchor as last association until code uses better algorithm for testing
	    tbb_Note,
	    tbb_Text,
	    tbb_Actor,
	    tbb_UseCase,
	    tbb_Concept,
	    tbb_Interface,
	    tbb_Package,
	    tbb_Object,
	    tbb_Initial_State,
	    tbb_State,
	    tbb_End_State,
	    tbb_Initial_Activity,
	    tbb_Activity,
	    tbb_End_Activity,
	    tbb_Branch,
	    tbb_Fork
	};

	typedef QMap<Uml::Diagram_Type,ToolBar_Buttons> OldToolMap;

private:
	ToolBar_Buttons m_CurrentButtonID;
	OldToolMap m_map;
	Uml::Diagram_Type m_Type;

	struct Pixmaps {
		QPixmap Arrow,
		Generalization,
		Aggregation,
		Dependency,
		Association,
		Message,
		Implementation,
		Composition,
		UniAssociation,
		Realization,
		Anchor,//keep anchor as last association until code uses better algorithm for testing
		Note,
		Text,
		Actor,
		UseCase,
		Concept,
		Object,
		Initial_State,
		End_State,
		Branch,
		Fork,
		Package,
		Interface;
	}
	m_Pixmaps;

	void loadPixmaps();
signals:
	void sigButtonChanged(int);
public slots:
	void slotCheckToolBar(Uml::Diagram_Type dt);
	void buttonChanged(int b);
	void slotResetToolBar();
};

#endif
