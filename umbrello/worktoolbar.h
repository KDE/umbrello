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
 * This is the toolbar that is displayed on the right-hand side of the program
 * window.  For each type of diagram it will change to suit that document.
 *
 * @short The toolbar that is different for each type of diagram.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class WorkToolBar : public KToolBar {
	Q_OBJECT
public:

	/**
	 * Enumeration of all available edit tools.
	 */
	enum EditTool {
		Select = 0,
		Generalization, Aggregation, Dependency, Association,
		CollMessage, SeqMessage,
		Composition, UniAssociation,
		StateTransition, ActivityTransition,
		Anchor, Note, Box, Text,
		Actor, UseCase, Class, Interface,
		Package, Component, Artifact, Object,
		InitialState, State, EndState,
		InitialActivity, Activity, EndActivity, Branch, Fork
	};

	/**
	 * Creates a work tool bar.
	 *
	 * @param parentWindow	The parent of the toolbar.
	 * @param name		The name of the toolbar.
	 */
	WorkToolBar(QMainWindow *parentWindow,const char*name);

	/**
	 * Sets the current tool to the previously used Tool. This is just
	 * as if the user had pressed the button for the other tool.
	 */
	void setOldTool();

	/**
	 * Sets the current tool to the default tool. (select tool)
	 * Calling this function is as if the user had pressed the "arrow"
	 * button on the toolbar.
	 */
	void setDefaultTool();

	/**
	 * Returns the currently selected EditTool.
	 *
	 * @return	The currently selected EditTool.
	 */
	EditTool currentTool();

	/**
	 * Standard deconstructor.
	 */
	~WorkToolBar();

	/**
	 * Enumeration of all available toolbar buttons.
	 */
	enum ToolBar_Buttons
	{
	    tbb_Undefined = -1,
	    tbb_Arrow,
	    tbb_Generalization,
	    tbb_Aggregation,
	    tbb_Dependency,
	    tbb_Association,
	    tbb_Coll_Message,
	    tbb_Seq_Message_Synchronous,
	    tbb_Seq_Message_Asynchronous,
	    tbb_Composition,
	    tbb_UniAssociation,
	    tbb_State_Transition,
	    tbb_Activity_Transition,
	    tbb_Anchor,//keep anchor as last association until code uses better algorithm for testing
	    tbb_Note,
	    tbb_Box,
	    tbb_Text,
	    tbb_Actor,
	    tbb_UseCase,
	    tbb_Class,
	    tbb_Interface,
	    tbb_Datatype,
	    tbb_Enum,
	    tbb_Package,
	    tbb_Component,
	    tbb_Node,
	    tbb_Artifact,
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

	static const unsigned nToolbarButtons = (unsigned)tbb_Fork -
						(unsigned)tbb_Arrow + 1;
	QPixmap m_Pixmaps[nToolbarButtons];

	/**
	 * These are loaded by loadPixmaps() and contain the images used
	 * for the mouse cursors.
	 */
	struct CursorPixmaps {
		QPixmap Actor;
		QPixmap Aggregation;
		QPixmap Anchor;
		QPixmap Artifact;
		QPixmap Association;
		QPixmap Box;
		QPixmap Branch;
		QPixmap Case;
		QPixmap Component;
		QPixmap Composition;
		QPixmap Dependency;
		QPixmap EndState;
		QPixmap Fork;
		QPixmap Generalisation;
		QPixmap InitialState;
		QPixmap Interface;
		QPixmap Datatype;
		QPixmap Enum;
		QPixmap MessageAsynchronous;
		QPixmap MessageSynchronous;
		QPixmap Node;
		QPixmap Note;
		QPixmap Object;
		QPixmap Package;
		QPixmap Text;
		QPixmap Class;
		QPixmap UniAssoc;
	}
	m_CursorPixmaps;

	/**
	 * Loads toolbar icon and mouse cursor images from disk
	 */
	void loadPixmaps();

	/**
	 * Returns the current cursor depending on m_CurrentButtonID
	 */
	QCursor currentCursor();

	/**
	 * Inserts the button corresponding to the tbb value given
	 * and activates the toggle.
	 */
	void insertHotBtn(ToolBar_Buttons tbb, const QString label);

	/**
	 * Inserts most associations, just reduces some string
	 * duplication (nice to translators)
	 */
	void insertBasicAssociations();

signals:
	void sigButtonChanged(int);
	void toolSelected(WorkToolBar::EditTool);
public slots:
	void slotCheckToolBar(Uml::Diagram_Type dt);
	void buttonChanged(int b);
	void slotResetToolBar();
};

#endif
