/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "worktoolbar.h"
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ktoolbarbutton.h>
#include <qcursor.h>

WorkToolBar::WorkToolBar(QMainWindow *parentWindow, const char*name) : KToolBar(parentWindow,QMainWindow::Right,true,name) {
	m_CurrentButtonID = tbb_Undefined;
	loadPixmaps();
	m_Type = Uml::dt_Class;//first time in just want it to load arrow, needs anything but dt_Undefined
	setOrientation( Vertical );
	setVerticalStretchable( true );
	//intialize old tool mal, everything starts with select tool (arrow)
	m_map.insert(Uml::dt_UseCase,tbb_Arrow);
	m_map.insert(Uml::dt_Collaboration,tbb_Arrow);
	m_map.insert(Uml::dt_Class,tbb_Arrow);
	m_map.insert(Uml::dt_Sequence,tbb_Arrow);
	m_map.insert(Uml::dt_State,tbb_Arrow);
	m_map.insert(Uml::dt_Activity,tbb_Arrow);
	m_map.insert(Uml::dt_Undefined,tbb_Arrow);

	slotCheckToolBar( Uml::dt_Undefined );
	connect( this, SIGNAL( released( int ) ), this, SLOT( buttonChanged (int ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
WorkToolBar::~WorkToolBar() {
	disconnect(this, SIGNAL(released(int)),this,SLOT(buttonChanged(int)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::slotCheckToolBar(Uml::Diagram_Type dt) {
	if( dt == m_Type )
		return;
	clear();
	m_Type = dt;

	//insert note, anchor and lines of text on all diagrams
	if( m_Type != Uml::dt_Undefined ) {
		insertButton( m_Pixmaps.Arrow, tbb_Arrow );
		setToggle( tbb_Arrow,true );
		toggleButton( tbb_Arrow );
		m_CurrentButtonID = tbb_Arrow;

		insertButton( m_Pixmaps.Note, tbb_Note, true, i18n("Note"));
		setToggle( tbb_Note, true );

		insertButton( m_Pixmaps.Anchor, tbb_Anchor, true, i18n("Anchor"));
		setToggle( tbb_Anchor, true );

		insertButton( m_Pixmaps.Text, tbb_Text, true, i18n("Line of text"));
		setToggle( tbb_Text, true );
	}

	//insert diagram specific tools
	if( m_Type == Uml::dt_UseCase ) {
		insertButton( m_Pixmaps.Actor, tbb_Actor, true, i18n("Actor"));
		setToggle( tbb_Actor, true );

		insertButton( m_Pixmaps.UseCase, tbb_UseCase, true, i18n("Use case"));
		setToggle( tbb_UseCase, true );

		insertButton( m_Pixmaps.Generalization, tbb_Generalization, true,
			      i18n("Implements (Generalisation/Realisation)"));
		setToggle( tbb_Generalization, true );

		insertButton( m_Pixmaps.Dependency, tbb_Dependency, true, i18n("Dependency"));
		setToggle( tbb_Dependency,true );

		insertButton( m_Pixmaps.Association, tbb_Association, true, i18n("Association"));
		setToggle( tbb_Association,true );

		insertButton( m_Pixmaps.UniAssociation, tbb_UniAssociation, true, i18n("Unidirectional association"));
		setToggle( tbb_UniAssociation, true );
	} else if( m_Type == Uml::dt_Class ) {
		insertButton( m_Pixmaps.Concept, tbb_Concept, true, i18n("Class"));
		setToggle( tbb_Concept,true );

		insertButton( m_Pixmaps.Interface, tbb_Interface, true, i18n("Interface"));
		setToggle( tbb_Interface,true );

		insertButton( m_Pixmaps.Package, tbb_Package, true, i18n("Package"));
		setToggle( tbb_Package,true );

		insertButton( m_Pixmaps.Composition, tbb_Composition, true, i18n("Composition"));
		setToggle( tbb_Composition, true );

		insertButton( m_Pixmaps.Generalization, tbb_Generalization, true,
			      i18n("Implements (Generalisation/Realisation)"));
		setToggle( tbb_Generalization, true );

		insertButton( m_Pixmaps.Aggregation, tbb_Aggregation, true, i18n("Aggregation"));
		setToggle( tbb_Aggregation, true );

		insertButton( m_Pixmaps.Dependency, tbb_Dependency, true, i18n("Dependency"));
		setToggle( tbb_Dependency, true );

		insertButton( m_Pixmaps.Association, tbb_Association, true, i18n("Association"));
		setToggle( tbb_Association,true );

		insertButton( m_Pixmaps.UniAssociation, tbb_UniAssociation, true, i18n("Unidirectional association"));
		setToggle( tbb_UniAssociation,true );
	} else if( m_Type == Uml::dt_Sequence ) {
		insertButton( m_Pixmaps.Object, tbb_Object, true, i18n("Object"));
		setToggle (tbb_Object, true);

		insertButton( m_Pixmaps.Message, tbb_Seq_Message, true, i18n("Message"));
		setToggle( tbb_Seq_Message, true );
	} else if( m_Type == Uml::dt_Collaboration ) {
		insertButton( m_Pixmaps.Object, tbb_Object, true, i18n("Object"));
		setToggle( tbb_Object, true );

		insertButton( m_Pixmaps.Message, tbb_Coll_Message, true, i18n("Message"));
		setToggle( tbb_Coll_Message, true );
	} else if( m_Type == Uml::dt_State ) {
		insertButton( m_Pixmaps.Initial_State, tbb_Initial_State, true, i18n("Initial state"));
		setToggle( tbb_Initial_State, true );

		insertButton( m_Pixmaps.UseCase, tbb_State, true, i18n("State"));
		setToggle( tbb_State, true );

		insertButton( m_Pixmaps.End_State, tbb_End_State, true, i18n("End state"));
		setToggle( tbb_End_State, true );

		insertButton( m_Pixmaps.UniAssociation, tbb_State_Transition, true, i18n("State transition"));
		setToggle( tbb_State_Transition, true );
	} else if( m_Type == Uml::dt_Activity ) {
		insertButton( m_Pixmaps.Initial_State, tbb_Initial_Activity, true, i18n("Initial activity"));
		setToggle( tbb_Initial_Activity, true );

		insertButton( m_Pixmaps.UseCase, tbb_Activity, true, i18n("Activity"));
		setToggle( tbb_Activity, true );

		insertButton( m_Pixmaps.End_State, tbb_End_Activity, true, i18n("End activity"));
		setToggle( tbb_End_Activity, true );

		insertButton( m_Pixmaps.Branch, tbb_Branch, true, i18n("Branch/merge"));
		setToggle( tbb_Branch, true );

		insertButton( m_Pixmaps.Fork, tbb_Fork, true, i18n("Fork/join"));
		setToggle( tbb_Fork, true );

		insertButton( m_Pixmaps.UniAssociation, tbb_Activity_Transition, true, i18n("Activity transition"));
		setToggle( tbb_Activity_Transition, true );
	} else if (m_Type == Uml::dt_Component) {
		insertButton( m_Pixmaps.Interface, tbb_Interface, true, i18n("Interface"));
		setToggle( tbb_Interface,true );

		insertButton( m_Pixmaps.Generalization, tbb_Generalization, true,
			      i18n("Implements (Generalisation/Realisation)"));
		setToggle( tbb_Generalization, true );

		insertButton( m_Pixmaps.Dependency, tbb_Dependency, true, i18n("Dependency"));
		setToggle( tbb_Dependency, true );

		insertButton( m_Pixmaps.Association, tbb_Association, true, i18n("Association"));
		setToggle( tbb_Association,true );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::buttonChanged(int b) {

	UMLView *view = ((UMLApp*) parent()->parent())->getDocument()->getCurrentView();
	QCursor curs;
	//if trying to turn off arrow - stop it
	ToolBar_Buttons tbb = (ToolBar_Buttons)b;
	if(tbb == tbb_Arrow && m_CurrentButtonID == tbb_Arrow) {
		toggleButton(tbb_Arrow);
		curs.setShape(Qt::ArrowCursor);
		view -> setCursor(curs);
		return;
	}
	//if toggling off a button set to arrow
	if(tbb == m_CurrentButtonID) {
		m_map[m_Type] = m_CurrentButtonID;  // store old tool for this diagram type
		toggleButton(tbb_Arrow);
		m_CurrentButtonID = tbb_Arrow;
		emit sigButtonChanged(m_CurrentButtonID);
		curs.setShape(Qt::ArrowCursor);
		view -> setCursor(curs);
		return;
	}
	m_map[m_Type] = m_CurrentButtonID;
	toggleButton(m_CurrentButtonID);
	m_CurrentButtonID = tbb;
	emit sigButtonChanged(m_CurrentButtonID);
	if(m_CurrentButtonID == tbb_Arrow)
		curs.setShape(Qt::ArrowCursor);
	else   //maybe switch and set a different cursor for each tool
		curs.setShape(Qt::CrossCursor);

	view->setCursor(curs);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::slotResetToolBar() {
	if(m_CurrentButtonID == tbb_Arrow)
		return;//really shouldn't occur
	toggleButton(m_CurrentButtonID);
	m_CurrentButtonID = tbb_Arrow;
	toggleButton(m_CurrentButtonID);
	emit sigButtonChanged(m_CurrentButtonID);

	QCursor curs;
	curs.setShape(Qt::ArrowCursor);

	UMLView *view = ((UMLApp*) parent()->parent())->getDocument()->getCurrentView();
	if (view != NULL) {
		view -> setCursor(curs);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::setOldTool() {
	KToolBarButton *b = (KToolBarButton*) getWidget(m_map[m_Type]);
	if(b)
		b -> animateClick();
}
///////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::setDefaultTool() {
	KToolBarButton *b = (KToolBarButton*) getWidget(tbb_Arrow);
	if(b)
		b -> animateClick();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::loadPixmaps() {
	KStandardDirs * dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/object.xpm" );
	dataDir += "/umbrello/pics/";

	m_Pixmaps.Object.load( dataDir + "object.xpm" );
	m_Pixmaps.Message.load( dataDir + "message.xpm" );
	m_Pixmaps.Arrow.load( dataDir + "arrow.xpm" );
	m_Pixmaps.Association.load( dataDir + "line.xpm" );
	m_Pixmaps.Anchor.load( dataDir + "anchor.xpm" );
	m_Pixmaps.Text.load( dataDir + "text.xpm" );
	m_Pixmaps.Note.load( dataDir + "note.xpm" );
	m_Pixmaps.Actor.load( dataDir + "actor.xpm" );
	m_Pixmaps.Dependency.load( dataDir + "dependency.xpm" );
	m_Pixmaps.Aggregation.load( dataDir + "aggregation.xpm" );
	m_Pixmaps.UniAssociation.load( dataDir + "uniassoc.xpm" );
	m_Pixmaps.Generalization.load( dataDir + "generalization.xpm" );
	m_Pixmaps.Composition.load( dataDir + "composition.xpm" );
	m_Pixmaps.Implementation.load( dataDir + "implements.xpm" );
	m_Pixmaps.UseCase.load( dataDir + "case.xpm" );
	m_Pixmaps.Concept.load( dataDir + "umlclass.xpm" );
	m_Pixmaps.Initial_State.load( dataDir + "initial_state.xpm" );
	m_Pixmaps.End_State.load( dataDir + "end_state.xpm" );
	m_Pixmaps.Branch.load( dataDir + "branch.xpm" );
	m_Pixmaps.Fork.load( dataDir + "fork.xpm" );
	m_Pixmaps.Package.load( dataDir + "package.xpm" );
	m_Pixmaps.Interface.load( dataDir + "interface.xpm" );
}

#include "worktoolbar.moc"
