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
#include <kstandarddirs.h>
#include <ktoolbarbutton.h>
#include <qcursor.h>

WorkToolBar::WorkToolBar(QMainWindow *parentWindow, const char*name) : KToolBar(parentWindow,Qt::DockRight,true,name) {
	m_CurrentButtonID = tbb_Undefined;
	loadPixmaps();
	m_Type = Uml::dt_Class;//first time in just want it to load arrow, needs anything but dt_Undefined
	setOrientation( Vertical );
	setVerticalStretchable( true );
	//intialize old tool map, everything starts with select tool (arrow)
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
#define BTN(name, label)  \
	insertButton(m_Pixmaps . name, tbb_ ## name, true, i18n(label)); \
	setToggle( tbb_ ## name, true )

void WorkToolBar::slotCheckToolBar(Uml::Diagram_Type dt) {
	if( dt == m_Type )
		return;
	clear();
	m_Type = dt;

	if( m_Type == Uml::dt_Undefined )
		return;

	//insert note, anchor and lines of text on all diagrams
	insertButton( m_Pixmaps.Arrow, tbb_Arrow );
	setToggle( tbb_Arrow,true );
	toggleButton( tbb_Arrow );
	m_CurrentButtonID = tbb_Arrow;

	BTN(Note, "Note");
	BTN(Anchor, "Anchor");
	BTN(Text, "Line of text");
	BTN(Box, "Box");

	//insert diagram specific tools
	if( m_Type == Uml::dt_UseCase ) {
		BTN(Actor, "Actor");
		BTN(UseCase, "Use case");
		BTN(Generalization, "Implements (Generalisation/Realisation)");
		BTN(Dependency, "Dependency");
		BTN(Association, "Association");
		BTN(UniAssociation, "Unidirectional association");

	} else if( m_Type == Uml::dt_Class ) {
		insertButton( m_Pixmaps.Concept, tbb_Class, true, i18n("Class"));
		setToggle( tbb_Class,true );

		BTN(Interface, "Interface");
		BTN(Package, "Package");
		BTN(Composition, "Composition");
		BTN(Generalization, "Implements (Generalisation/Realisation)");
		BTN(Aggregation, "Aggregation");
		BTN(Dependency, "Dependency");
		BTN(Association, "Association");
		BTN(UniAssociation, "Unidirectional association");

	} else if( m_Type == Uml::dt_Sequence ) {
		BTN(Object, "Object");

		insertButton( m_Pixmaps.MessageSynchronous, tbb_Seq_Message_Synchronous, true,
			      i18n("Synchronous Message"));
		setToggle( tbb_Seq_Message_Synchronous, true );

		insertButton( m_Pixmaps.MessageAsynchronous, tbb_Seq_Message_Asynchronous, true,
			      i18n("Asynchronous Message"));
		setToggle( tbb_Seq_Message_Asynchronous, true );

	} else if( m_Type == Uml::dt_Collaboration ) {
		BTN(Object, "Object");

		insertButton( m_Pixmaps.MessageAsynchronous, tbb_Coll_Message, true, i18n("Message"));
		setToggle( tbb_Coll_Message, true );

	} else if( m_Type == Uml::dt_State ) {
		BTN(Initial_State, "Initial state");

		insertButton( m_Pixmaps.UseCase, tbb_State, true, i18n("State"));
		setToggle( tbb_State, true );

		BTN(End_State, "End state");

		insertButton( m_Pixmaps.UniAssociation, tbb_State_Transition, true, i18n("State transition"));
		setToggle( tbb_State_Transition, true );

	} else if( m_Type == Uml::dt_Activity ) {
		insertButton( m_Pixmaps.Initial_State, tbb_Initial_Activity, true, i18n("Initial activity"));
		setToggle( tbb_Initial_Activity, true );

		insertButton( m_Pixmaps.UseCase, tbb_Activity, true, i18n("Activity"));
		setToggle( tbb_Activity, true );

		insertButton( m_Pixmaps.End_State, tbb_End_Activity, true, i18n("End activity"));
		setToggle( tbb_End_Activity, true );

		BTN(Branch, "Branch/merge");

		BTN(Fork, "Fork/join");

		insertButton( m_Pixmaps.UniAssociation, tbb_Activity_Transition, true, i18n("Activity transition"));
		setToggle( tbb_Activity_Transition, true );

	} else if (m_Type == Uml::dt_Component) {
		BTN(Interface, "Interface");
		BTN(Component, "Component");
		BTN(Artifact, "Artifact");
		BTN(Generalization, "Implements (Generalisation/Realisation)");
		BTN(Dependency, "Dependency");
		BTN(Association, "Association");

	} else if (m_Type == Uml::dt_Deployment) {
		BTN(Object, "Object");
		BTN(Interface, "Interface");
		BTN(Component, "Component");
		BTN(Node, "Node");
		BTN(Generalization, "Implements (Generalisation/Realisation)");
		BTN(Dependency, "Dependency");
		BTN(Association, "Association");

	} else {
		kdWarning() << "slotCheckToolBar() on unknown diagram type:" << m_Type << endl;
	}
}
#undef BTN

////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::buttonChanged(int b) {
	UMLView* view = UMLApp::app()->getDocument()->getCurrentView();

	//if trying to turn off arrow - stop it
	ToolBar_Buttons tbb = (ToolBar_Buttons)b;
	if(tbb == tbb_Arrow && m_CurrentButtonID == tbb_Arrow) {
		toggleButton(tbb_Arrow);
                view->setCursor( currentCursor() );
		return;
	}

	//if toggling off a button set to arrow
	if(tbb == m_CurrentButtonID) {
		m_map[m_Type] = m_CurrentButtonID;  // store old tool for this diagram type
		toggleButton(tbb_Arrow);
		m_CurrentButtonID = tbb_Arrow;
		emit sigButtonChanged(m_CurrentButtonID);
		emit toolSelected((EditTool)m_CurrentButtonID);
		view->setCursor( currentCursor() );
		return;
	}
	m_map[m_Type] = m_CurrentButtonID;
	toggleButton(m_CurrentButtonID);
	m_CurrentButtonID = tbb;
	emit sigButtonChanged(m_CurrentButtonID);
	emit toolSelected((EditTool)m_CurrentButtonID);
        view->setCursor( currentCursor() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QCursor WorkToolBar::currentCursor() {
	KStandardDirs* dirs = KGlobal::dirs();
	QString dataDir = dirs->findResourceDir("data", "umbrello/pics/object.xpm");
	dataDir += "/umbrello/pics/";

	if (m_CurrentButtonID == tbb_Actor)  {
		return QCursor(m_CursorPixmaps.Actor, 9, 9);
	} else if (m_CurrentButtonID == tbb_Aggregation)  {
		return QCursor(m_CursorPixmaps.Aggregation, 9, 9);
	} else if (m_CurrentButtonID == tbb_Anchor)  {
		return QCursor(m_CursorPixmaps.Anchor, 9, 9);
	} else if (m_CurrentButtonID == tbb_Artifact)  {
		return QCursor(m_CursorPixmaps.Artifact, 9, 9);
	} else if (m_CurrentButtonID == tbb_Association)  {
		return QCursor(m_CursorPixmaps.Association, 9, 9);
	} else if (m_CurrentButtonID == tbb_Box)  {
		return QCursor(m_CursorPixmaps.Box, 9, 9);
	} else if (m_CurrentButtonID == tbb_Branch)  {
		return QCursor(m_CursorPixmaps.Branch, 9, 9);
	} else if (m_CurrentButtonID == tbb_UseCase ||
		   m_CurrentButtonID == tbb_State ||
		   m_CurrentButtonID == tbb_Activity)  {
		return QCursor(m_CursorPixmaps.Case, 9, 9);
	} else if (m_CurrentButtonID == tbb_Component)  {
		return QCursor(m_CursorPixmaps.Component, 9, 9);
	} else if (m_CurrentButtonID == tbb_Composition)  {
		return QCursor(m_CursorPixmaps.Composition, 9, 9);
	} else if (m_CurrentButtonID == tbb_Dependency)  {
		return QCursor(m_CursorPixmaps.Dependency, 9, 9);
	} else if (m_CurrentButtonID == tbb_End_State ||
		   m_CurrentButtonID == tbb_End_Activity)  {
		return QCursor(m_CursorPixmaps.EndState, 9, 9);
	} else if (m_CurrentButtonID == tbb_Fork)  {
		return QCursor(m_CursorPixmaps.Fork, 9, 9);
	} else if (m_CurrentButtonID == tbb_Generalization)  {
		return QCursor(m_CursorPixmaps.Generalisation, 9, 9);
	} else if (m_CurrentButtonID == tbb_Initial_State ||
		   m_CurrentButtonID == tbb_Initial_Activity)  {
		return QCursor(m_CursorPixmaps.InitialState, 9, 9);
	} else if (m_CurrentButtonID == tbb_Interface)  {
		return QCursor(m_CursorPixmaps.Interface, 9, 9);
	} else if (m_CurrentButtonID == tbb_Seq_Message_Asynchronous ||
		   m_CurrentButtonID == tbb_Coll_Message)  {
		return QCursor(m_CursorPixmaps.MessageAsynchronous, 9, 9);
	} else if (m_CurrentButtonID == tbb_Seq_Message_Synchronous)  {
		return QCursor(m_CursorPixmaps.MessageSynchronous, 9, 9);
	} else if (m_CurrentButtonID == tbb_Node)  {
		return QCursor(m_CursorPixmaps.Node, 9, 9);
	} else if (m_CurrentButtonID == tbb_Note)  {
		return QCursor(m_CursorPixmaps.Note, 9, 9);
	} else if (m_CurrentButtonID == tbb_Object)  {
		return QCursor(m_CursorPixmaps.Object, 9, 9);
	} else if (m_CurrentButtonID == tbb_Package)  {
		return QCursor(m_CursorPixmaps.Package, 9, 9);
	} else if (m_CurrentButtonID == tbb_Text)  {
		return QCursor(m_CursorPixmaps.Text, 9, 9);
	} else if (m_CurrentButtonID == tbb_Class)  {
		return QCursor(m_CursorPixmaps.Class, 9, 9);
	} else if (m_CurrentButtonID == tbb_UniAssociation ||
		   m_CurrentButtonID == tbb_State_Transition ||
		   m_CurrentButtonID == tbb_Activity_Transition)  {
		return QCursor(m_CursorPixmaps.UniAssoc, 9, 9);
	} else {
		return QCursor(Qt::ArrowCursor);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkToolBar::slotResetToolBar() {
	if(m_CurrentButtonID == tbb_Arrow)
		return;//really shouldn't occur
	toggleButton(m_CurrentButtonID);
	m_CurrentButtonID = tbb_Arrow;
	toggleButton(m_CurrentButtonID);
	emit sigButtonChanged(m_CurrentButtonID);
	emit toolSelected((EditTool)m_CurrentButtonID);

	QCursor curs;
	curs.setShape(Qt::ArrowCursor);

	UMLView* view = UMLApp::app()->getDocument()->getCurrentView();
	if (view != NULL) {
		view -> setCursor(curs);
	}
}


WorkToolBar::EditTool WorkToolBar::currentTool()
{
return (EditTool)m_CurrentButtonID;
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
	m_Pixmaps.MessageSynchronous.load( dataDir + "message-synchronous.xpm" );
	m_Pixmaps.MessageAsynchronous.load( dataDir + "message-asynchronous.xpm" );
	m_Pixmaps.Arrow.load( dataDir + "arrow.png" );
	m_Pixmaps.Association.load( dataDir + "line.xpm" );
	m_Pixmaps.Anchor.load( dataDir + "anchor.xpm" );
	m_Pixmaps.Text.load( dataDir + "text.xpm" );
	m_Pixmaps.Note.load( dataDir + "note.xpm" );
	m_Pixmaps.Box.load( dataDir + "box.xpm" );
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
	m_Pixmaps.Component.load( dataDir + "component.xpm" );
	m_Pixmaps.Node.load( dataDir + "node.xpm" );
	m_Pixmaps.Artifact.load( dataDir + "artifact.xpm" );
	m_Pixmaps.Interface.load( dataDir + "interface.xpm" );

	m_CursorPixmaps.Actor.load( dataDir + "cursor-actor.xpm");
	m_CursorPixmaps.Aggregation.load( dataDir + "cursor-aggregation.xpm");
	m_CursorPixmaps.Anchor.load( dataDir + "cursor-anchor.xpm");
	m_CursorPixmaps.Artifact.load( dataDir + "cursor-artifact.xpm");
	m_CursorPixmaps.Association.load( dataDir + "cursor-association.xpm");
	m_CursorPixmaps.Box.load( dataDir + "cursor-box.xpm");
	m_CursorPixmaps.Branch.load( dataDir + "cursor-branch.xpm");
	m_CursorPixmaps.Case.load( dataDir + "cursor-case.xpm");
	m_CursorPixmaps.Component.load( dataDir + "cursor-component.xpm");
	m_CursorPixmaps.Composition.load( dataDir + "cursor-composition.xpm");
	m_CursorPixmaps.Dependency.load( dataDir + "cursor-dependency.xpm");
	m_CursorPixmaps.EndState.load( dataDir + "cursor-end-state.xpm");
	m_CursorPixmaps.Fork.load( dataDir + "cursor-fork.xpm");
	m_CursorPixmaps.InitialState.load( dataDir + "cursor-initial-state.xpm");
	m_CursorPixmaps.Interface.load( dataDir + "cursor-interface.xpm");
	m_CursorPixmaps.Generalisation.load( dataDir + "cursor-generalisation.xpm");
	m_CursorPixmaps.MessageAsynchronous.load( dataDir + "cursor-message-asynchronous.xpm");
	m_CursorPixmaps.MessageSynchronous.load( dataDir + "cursor-message-synchronous.xpm");
	m_CursorPixmaps.Node.load( dataDir + "cursor-node.xpm");
	m_CursorPixmaps.Note.load( dataDir + "cursor-note.xpm");
	m_CursorPixmaps.Object.load( dataDir + "cursor-object.xpm");
	m_CursorPixmaps.Package.load( dataDir + "cursor-package.xpm");
	m_CursorPixmaps.Text.load( dataDir + "cursor-text.xpm");
	m_CursorPixmaps.Class.load( dataDir + "cursor-umlclass.xpm");
	m_CursorPixmaps.UniAssoc.load( dataDir + "cursor-uniassoc.xpm");
}

#include "worktoolbar.moc"
