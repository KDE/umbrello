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

WorkToolBar::WorkToolBar(QMainWindow *parentWindow, const char*name)
  : KToolBar(parentWindow,Qt::DockRight,false,name) {
	m_CurrentButtonID = tbb_Undefined;
	loadPixmaps();
	m_Type = Uml::dt_Class; /* first time in just want it to load arrow,
				   needs anything but dt_Undefined  */
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

WorkToolBar::~WorkToolBar() {
	disconnect(this, SIGNAL(released(int)),this,SLOT(buttonChanged(int)));
}

void WorkToolBar::insertHotBtn(ToolBar_Buttons tbb, const QString label) {
	insertButton(m_Pixmaps[tbb], tbb, true, label);
	setToggle( tbb, true );
}

void WorkToolBar::slotCheckToolBar(Uml::Diagram_Type dt) {
	if( dt == m_Type )
		return;
	clear();
	m_Type = dt;

	if( m_Type == Uml::dt_Undefined )
		return;

	//insert note, anchor and lines of text on all diagrams
	insertButton( m_Pixmaps[tbb_Arrow], tbb_Arrow, true, i18n("Select") );
	setToggle( tbb_Arrow, true );
	toggleButton( tbb_Arrow );
	m_CurrentButtonID = tbb_Arrow;

	insertHotBtn(tbb_Note, i18n("Note"));
	insertHotBtn(tbb_Anchor, i18n("Anchor"));
	insertHotBtn(tbb_Text, i18n("Label"));
	insertHotBtn(tbb_Box, i18n("Box"));

	//insert diagram specific tools
	switch (m_Type) {
	case Uml::dt_UseCase:
		insertHotBtn(tbb_Actor, i18n("Actor"));
		insertHotBtn(tbb_UseCase, i18n("Use case"));
		insertBasicAssociations();
		break;

	case Uml::dt_Class:
		insertHotBtn(tbb_Class, i18n("Class"));
		insertHotBtn(tbb_Interface, i18n("Interface"));
		insertHotBtn(tbb_Datatype, i18n("Datatype"));
		insertHotBtn(tbb_Enum, i18n("Enum"));
		insertHotBtn(tbb_Package, i18n("Package"));
		insertBasicAssociations();
		insertHotBtn(tbb_Composition, i18n("Composition"));
		insertHotBtn(tbb_Aggregation, i18n("Aggregation"));
		insertHotBtn(tbb_Containment, i18n("Containment"));
		break;

	case Uml::dt_Sequence:
		insertHotBtn(tbb_Object, i18n("Object"));

		insertHotBtn(tbb_Seq_Message_Synchronous, i18n("Synchronous Message"));
		insertHotBtn(tbb_Seq_Message_Asynchronous, i18n("Asynchronous Message"));
		break;

	case Uml::dt_Collaboration:
		insertHotBtn(tbb_Object, i18n("Object"));
		insertButton( m_Pixmaps[tbb_Seq_Message_Asynchronous], tbb_Coll_Message,
			      true, i18n("Message"));
		setToggle( tbb_Coll_Message, true );
		break;

	case Uml::dt_State:
		insertHotBtn(tbb_Initial_State, i18n("Initial state"));
		insertButton( m_Pixmaps[tbb_UseCase], tbb_State,
			      true, i18n("State"));
		setToggle( tbb_State, true );
		insertHotBtn(tbb_End_State, i18n("End state"));
		insertButton( m_Pixmaps[tbb_UniAssociation], tbb_State_Transition,
			      true, i18n("State transition"));
		setToggle( tbb_State_Transition, true );
		break;

	case Uml::dt_Activity:
		insertButton( m_Pixmaps[tbb_Initial_State], tbb_Initial_Activity,
			      true, i18n("Initial activity"));
		setToggle( tbb_Initial_Activity, true );
		insertButton( m_Pixmaps[tbb_UseCase], tbb_Activity,
			      true, i18n("Activity"));
		setToggle( tbb_Activity, true );
		insertButton( m_Pixmaps[tbb_End_State], tbb_End_Activity,
			      true, i18n("End activity"));
		setToggle( tbb_End_Activity, true );

		insertHotBtn(tbb_Branch, i18n("Branch/merge"));

		insertHotBtn(tbb_Fork, i18n("Fork/join"));

		insertButton( m_Pixmaps[tbb_UniAssociation], tbb_Activity_Transition,
			      true, i18n("Activity transition"));
		setToggle( tbb_Activity_Transition, true );
		break;

	case Uml::dt_Component:
		insertHotBtn(tbb_Interface, i18n("Interface"));
		insertHotBtn(tbb_Component, i18n("Component"));
		insertHotBtn(tbb_Artifact, i18n("Artifact"));
		insertBasicAssociations();
		break;

	case Uml::dt_Deployment:
		insertHotBtn(tbb_Object, i18n("Object"));
		insertHotBtn(tbb_Interface, i18n("Interface"));
		insertHotBtn(tbb_Component, i18n("Component"));
		insertHotBtn(tbb_Node, i18n("Node"));
		insertBasicAssociations();
		break;

	default:
		kdWarning() << "slotCheckToolBar() on unknown diagram type:"
			    << m_Type << endl;
		break;
	}
}

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
		view->setCursor( currentCursor() );
		return;
	}
	m_map[m_Type] = m_CurrentButtonID;
	toggleButton(m_CurrentButtonID);
	m_CurrentButtonID = tbb;
	emit sigButtonChanged(m_CurrentButtonID);
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
	} else if (m_CurrentButtonID == tbb_Association) {
		return QCursor(m_CursorPixmaps.Association, 9, 9);
	} else if (m_CurrentButtonID == tbb_Containment)  {
		return QCursor(m_CursorPixmaps.Containment, 9, 9);
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
	} else if (m_CurrentButtonID == tbb_Datatype)  {
		return QCursor(m_CursorPixmaps.Datatype, 9, 9);
	} else if (m_CurrentButtonID == tbb_Enum)  {
		return QCursor(m_CursorPixmaps.Enum, 9, 9);
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

	QCursor curs;
	curs.setShape(Qt::ArrowCursor);

	UMLView* view = UMLApp::app()->getDocument()->getCurrentView();
	if (view != NULL) {
		view -> setCursor(curs);
	}
}

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

	m_Pixmaps[tbb_Object].load( dataDir + "object.png" );
	m_Pixmaps[tbb_Seq_Message_Synchronous].load( dataDir + "message-synchronous.png" );
	m_Pixmaps[tbb_Seq_Message_Asynchronous].load( dataDir + "message-asynchronous.png" );
	m_Pixmaps[tbb_Arrow].load( dataDir + "arrow.png" );
	m_Pixmaps[tbb_Association].load( dataDir + "association.png" );
	m_Pixmaps[tbb_Containment].load( dataDir + "containment.png" );
	m_Pixmaps[tbb_Anchor].load( dataDir + "anchor.png" );
	m_Pixmaps[tbb_Text].load( dataDir + "text.png" );
	m_Pixmaps[tbb_Note].load( dataDir + "note.png" );
	m_Pixmaps[tbb_Box].load( dataDir + "box.png" );
	m_Pixmaps[tbb_Actor].load( dataDir + "actor.png" );
	m_Pixmaps[tbb_Dependency].load( dataDir + "dependency.png" );
	m_Pixmaps[tbb_Aggregation].load( dataDir + "aggregation.png" );
	m_Pixmaps[tbb_UniAssociation].load( dataDir + "uniassociation.png" );
	m_Pixmaps[tbb_Generalization].load( dataDir + "generalisation.png" );
	m_Pixmaps[tbb_Composition].load( dataDir + "composition.png" );
	m_Pixmaps[tbb_UseCase].load( dataDir + "usecase.png" );
	m_Pixmaps[tbb_Class].load( dataDir + "class.png" );
	m_Pixmaps[tbb_Initial_State].load( dataDir + "initial_state.png" );
	m_Pixmaps[tbb_End_State].load( dataDir + "end_state.png" );
	m_Pixmaps[tbb_Branch].load( dataDir + "branch.png" );
	m_Pixmaps[tbb_Fork].load( dataDir + "fork.png" );
	m_Pixmaps[tbb_Package].load( dataDir + "package.png" );
	m_Pixmaps[tbb_Component].load( dataDir + "component.png" );
	m_Pixmaps[tbb_Node].load( dataDir + "node.png" );
	m_Pixmaps[tbb_Artifact].load( dataDir + "artifact.png" );
	m_Pixmaps[tbb_Interface].load( dataDir + "interface.png" );
	m_Pixmaps[tbb_Datatype].load( dataDir + "datatype.png" );
	m_Pixmaps[tbb_Enum].load( dataDir + "enum.png" );

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
	m_CursorPixmaps.Containment.load( dataDir + "cursor-containment.xpm");
	m_CursorPixmaps.Dependency.load( dataDir + "cursor-dependency.xpm");
	m_CursorPixmaps.EndState.load( dataDir + "cursor-end-state.xpm");
	m_CursorPixmaps.Fork.load( dataDir + "cursor-fork.xpm");
	m_CursorPixmaps.InitialState.load( dataDir + "cursor-initial-state.xpm");
	m_CursorPixmaps.Interface.load( dataDir + "cursor-interface.xpm");
	m_CursorPixmaps.Datatype.load( dataDir + "cursor-datatype.xpm");
	m_CursorPixmaps.Enum.load( dataDir + "cursor-enum.xpm");
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

void WorkToolBar::insertBasicAssociations()  {
	insertHotBtn(tbb_Association, i18n("Association"));
	if (m_Type == dt_Class || m_Type == dt_UseCase)  {
		insertHotBtn(tbb_UniAssociation, i18n("Directional Association"));
	}
	insertHotBtn(tbb_Dependency, i18n("Dependency"));
	insertHotBtn(tbb_Generalization, i18n("Implements (Generalisation/Realisation)"));
}

#include "worktoolbar.moc"
