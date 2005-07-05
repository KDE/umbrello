/*
 *  copyright (C) 2002-2005
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//qt include files

//kde include files
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktoolbarbutton.h>

// application specific includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "worktoolbar.h"


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

void WorkToolBar::insertHotBtn(ToolBar_Buttons tbb) {
    try {
        insertButton(m_ToolButtons[tbb].Symbol, tbb, true, m_ToolButtons[tbb].Label);
        setToggle(tbb, true);
    }
    catch (...) {
        kdError() << "Undefined button id " << tbb << "!" << endl;
    }
}

void WorkToolBar::insertBasicAssociations()  {
    insertHotBtn(tbb_Association);
    if (m_Type == Uml::dt_Class || m_Type == Uml::dt_UseCase) {
        insertHotBtn(tbb_UniAssociation);
    }
    insertHotBtn(tbb_Dependency);
    insertHotBtn(tbb_Generalization);
}

void WorkToolBar::slotCheckToolBar(Uml::Diagram_Type dt) {
    if ( dt == m_Type )
        return;
    clear();
    m_Type = dt;

    if ( m_Type == Uml::dt_Undefined )
        return;

    //insert note, anchor and lines of text on all diagrams
    insertHotBtn(tbb_Arrow);
    toggleButton(tbb_Arrow);
    m_CurrentButtonID = tbb_Arrow;

    insertHotBtn(tbb_Note);
    insertHotBtn(tbb_Anchor);
    insertHotBtn(tbb_Text);
    insertHotBtn(tbb_Box);

    //insert diagram specific tools
    switch (m_Type) {
    case Uml::dt_UseCase:
        insertHotBtn(tbb_Actor);
        insertHotBtn(tbb_UseCase);
        insertBasicAssociations();
        break;

    case Uml::dt_Class:
        insertHotBtn(tbb_Class);
        insertHotBtn(tbb_Interface);
        insertHotBtn(tbb_Datatype);
        insertHotBtn(tbb_Enum);
        insertHotBtn(tbb_Package);
        insertBasicAssociations();
        insertHotBtn(tbb_Composition);
        insertHotBtn(tbb_Aggregation);
        insertHotBtn(tbb_Containment);
        break;

    case Uml::dt_Sequence:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Seq_Message_Synchronous);
        insertHotBtn(tbb_Seq_Message_Asynchronous);
        break;

    case Uml::dt_Collaboration:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Coll_Message);
        break;

    case Uml::dt_State:
        insertHotBtn(tbb_Initial_State);
        insertHotBtn(tbb_State);
        insertHotBtn(tbb_End_State);
        insertHotBtn(tbb_State_Transition);
        insertHotBtn(tbb_DeepHistory);
        insertHotBtn(tbb_ShallowHistory);
        insertHotBtn(tbb_Join);
        insertHotBtn(tbb_StateFork);
        insertHotBtn(tbb_Junction);
        insertHotBtn(tbb_Choice);
        insertHotBtn(tbb_Andline);
        break;

    case Uml::dt_Activity:
        insertHotBtn(tbb_Initial_Activity);
        insertHotBtn(tbb_Activity);
        insertHotBtn(tbb_End_Activity);
        insertHotBtn(tbb_Branch);
        insertHotBtn(tbb_Fork);
        insertHotBtn(tbb_Activity_Transition);
        break;

    case Uml::dt_Component:
        insertHotBtn(tbb_Interface);
        insertHotBtn(tbb_Component);
        insertHotBtn(tbb_Artifact);
        insertBasicAssociations();
        break;

    case Uml::dt_Deployment:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Interface);
        insertHotBtn(tbb_Component);
        insertHotBtn(tbb_Node);
        insertBasicAssociations();
        break;

    case Uml::dt_EntityRelationship:
        insertHotBtn(tbb_Entity);
        insertHotBtn(tbb_Relationship);
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
    if (tbb == tbb_Arrow && m_CurrentButtonID == tbb_Arrow) {
        toggleButton(tbb_Arrow);
        view->setCursor( currentCursor() );
        return;
    }

    //if toggling off a button set to arrow
    if (tbb == m_CurrentButtonID) {
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

QCursor WorkToolBar::currentCursor() {
    try {
        return m_ToolButtons[m_CurrentButtonID].Cursor;
    }
    catch (...) {
        kdError() << "Cursor not found for id " << m_CurrentButtonID << "!" << endl;
        return QCursor();
    }
}

void WorkToolBar::slotResetToolBar() {
    if (m_CurrentButtonID == tbb_Arrow)
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
    if (b)
        b -> animateClick();
}

void WorkToolBar::setDefaultTool() {
    KToolBarButton *b = (KToolBarButton*) getWidget(tbb_Arrow);
    if (b)
        b -> animateClick();
}

QPixmap WorkToolBar::load(const QString & fileName) {
    try {
        QPixmap pxm;
        pxm.load(fileName);
        return pxm;
    }
    catch (...) {
        kdError() << "Could not load pixmap from " << fileName << "!" << endl;
        return 0;
    }
}

void WorkToolBar::loadPixmaps() {
    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs->findResourceDir( "data", "umbrello/pics/object.png" );
    dataDir += "/umbrello/pics/";

    m_ToolButtons.insert(tbb_Undefined,
                         ToolButton(i18n("UNDEFINED"),
                                    0,
                                    QCursor()) );
    m_ToolButtons.insert(tbb_Arrow,
                         ToolButton(i18n("Select"),
                                    load(dataDir + "arrow.png"),
                                    QCursor()) );
    m_ToolButtons.insert(tbb_Object,
                         ToolButton(i18n("Object"),
                                    load(dataDir + "object.png"),
                                    QCursor(load(dataDir + "cursor-object.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Seq_Message_Synchronous,
                         ToolButton(i18n("Synchronous Message"),
                                    load(dataDir + "message-synchronous.png"),
                                    QCursor(load(dataDir + "cursor-message-synchronous.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Seq_Message_Asynchronous,
                         ToolButton(i18n("Asynchronous Message"),
                                    load(dataDir + "message-asynchronous.png"),
                                    QCursor(load(dataDir + "cursor-message-asynchronous.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Association,
                         ToolButton(i18n("Association"),
                                    load(dataDir + "association.png"),
                                    QCursor(load(dataDir + "cursor-association.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Containment,
                         ToolButton(i18n("Containment"),
                                    load(dataDir + "containment.png"),
                                    QCursor(load(dataDir + "cursor-containment.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Anchor,
                         ToolButton(i18n("Anchor"),
                                    load(dataDir + "anchor.png"),
                                    QCursor(load(dataDir + "cursor-anchor.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Text,
                         ToolButton(i18n("Label"),
                                    load(dataDir + "text.png"),
                                    QCursor(load(dataDir + "cursor-text.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Note,
                         ToolButton(i18n("Note"),
                                    load(dataDir + "note.png"),
                                    QCursor(load(dataDir + "cursor-note.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Box,
                         ToolButton(i18n("Box"),
                                    load(dataDir + "box.png"),
                                    QCursor(load(dataDir + "cursor-box.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Actor,
                         ToolButton(i18n("Actor"),
                                    load(dataDir + "actor.png"),
                                    QCursor(load(dataDir + "cursor-actor.png"), 9, 9)));
    m_ToolButtons.insert(tbb_Dependency,
                         ToolButton(i18n("Dependency"),
                                    load(dataDir + "dependency.png"),
                                    QCursor(load(dataDir + "cursor-dependency.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Aggregation,
                         ToolButton(i18n("Aggregation"),
                                    load(dataDir + "aggregation.png"),
                                    QCursor(load(dataDir + "cursor-aggregation.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Relationship,
                         ToolButton(i18n("Relationship"),
                                    load(dataDir + "relationship.png"),
                                    QCursor(load(dataDir + "cursor-relationship.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_UniAssociation,
                         ToolButton(i18n("Directional Association"),
                                    load(dataDir + "uniassociation.png"),
                                    QCursor(load(dataDir + "cursor-uniassociation.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Generalization,
                         ToolButton(i18n("Implements (Generalisation/Realisation)"),
                                    load(dataDir + "generalisation.png"),
                                    QCursor(load(dataDir + "cursor-generalisation.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Composition,
                         ToolButton(i18n("Composition"),
                                    load(dataDir + "composition.png"),
                                    QCursor(load(dataDir + "cursor-composition.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_UseCase,
                         ToolButton(i18n("Use Case"),
                                    load(dataDir + "usecase.png"),
                                    QCursor(load(dataDir + "cursor-usecase.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Class,
                         ToolButton(i18n("Class"),
                                    load(dataDir + "class.png"),
                                    QCursor(load(dataDir + "cursor-class.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Initial_State,
                         ToolButton(i18n("Initial State"),
                                    load(dataDir + "initial_state.png"),
                                    QCursor(load(dataDir + "cursor-initial-state.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_End_State,
                         ToolButton(i18n("End State"),
                                    load(dataDir + "end_state.png"),
                                    QCursor(load(dataDir + "cursor-end-state.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Branch,
                         ToolButton(i18n("Branch/Merge"),
                                    load(dataDir + "branch.png"),
                                    QCursor(load(dataDir + "cursor-branch.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Fork,
                         ToolButton(i18n("Fork/Join"),
                                    load(dataDir + "fork.png"),
                                    QCursor(load(dataDir + "cursor-fork.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Package,
                         ToolButton(i18n("Package"),
                                    load(dataDir + "package.png"),
                                    QCursor(load(dataDir + "cursor-package.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Component,
                         ToolButton(i18n("Component"),
                                    load(dataDir + "component.png"),
                                    QCursor(load(dataDir + "cursor-component.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Node,
                         ToolButton(i18n("Node"),
                                    load(dataDir + "node.png"),
                                    QCursor(load(dataDir + "cursor-node.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Artifact,
                         ToolButton(i18n("Artifact"),
                                    load(dataDir + "artifact.png"),
                                    QCursor(load(dataDir + "cursor-artifact.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Interface,
                         ToolButton(i18n("Interface"),
                                    load(dataDir + "interface.png"),
                                    QCursor(load(dataDir + "cursor-interface.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Datatype,
                         ToolButton(i18n("Datatype"),
                                    load(dataDir + "datatype.png"),
                                    QCursor(load(dataDir + "cursor-datatype.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Enum,
                         ToolButton(i18n("Enum"),
                                    load(dataDir + "enum.png"),
                                    QCursor(load(dataDir + "cursor-enum.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Entity,
                         ToolButton(i18n("Entity"),
                                    load(dataDir + "entity.png"),
                                    QCursor(load(dataDir + "cursor-entity.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_DeepHistory,
                         ToolButton(i18n("Deep History"),
                                    load(dataDir + "deep-history.png"),
                                    QCursor(load(dataDir + "cursor-deep-history.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_ShallowHistory,
                         ToolButton(i18n("Shallow History"),
                                    load(dataDir + "shallow-history.png"),
                                    QCursor(load(dataDir + "cursor-shallow-history.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Join,
                         ToolButton(i18n("Join"),
                                    load(dataDir + "join.png"),
                                    QCursor(load(dataDir + "cursor-join.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_StateFork,
                         ToolButton(i18n("Fork"),
                                    load(dataDir + "state-fork.png"),
                                    QCursor(load(dataDir + "cursor-state-fork.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Junction,
                         ToolButton(i18n("Junction"),
                                    load(dataDir + "junction.png"),
                                    QCursor(load(dataDir + "cursor-junction.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Choice,
                         ToolButton(i18n("Choice"),
                                    load(dataDir + "choice-round.png"),
                                    QCursor(load(dataDir + "cursor-choice-round.png"), 9, 9)) );
    //:TODO: let the user decide which symbol he wants (setting an option)
    //m_ToolButtons.insert(tbb_Choice,
    //	ToolButton(i18n("Choice"),
    //		load(dataDir + "choice-rhomb.png"),
    //		QCursor(load(dataDir + "cursor-choice-rhomb.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Andline,
                         ToolButton(i18n("And Line"),
                                    load(dataDir + "andline.png"),
                                    QCursor(load(dataDir + "cursor-andline.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_State_Transition,
                         ToolButton(i18n("State Transition"),
                                    load(dataDir + "uniassociation.png"),
                                    QCursor(load(dataDir + "cursor-uniassociation.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Activity_Transition,
                         ToolButton(i18n("Activity Transition"),
                                    load(dataDir + "uniassociation.png"),
                                    QCursor(load(dataDir + "cursor-uniassociation.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Activity,
                         ToolButton(i18n("Activity"),
                                    load(dataDir + "usecase.png"),
                                    QCursor(load(dataDir + "cursor-usecase.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_State,
                         ToolButton(i18n("State"),
                                    load(dataDir + "usecase.png"),
                                    QCursor(load(dataDir + "cursor-usecase.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_End_Activity,
                         ToolButton(i18n("End Activity"),
                                    load(dataDir + "end_state.png"),
                                    QCursor(load(dataDir + "cursor-end-state.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Initial_Activity,
                         ToolButton(i18n("Initial Activity"),
                                    load(dataDir + "initial_state.png"),
                                    QCursor(load(dataDir + "cursor-initial-state.png"), 9, 9)) );
    m_ToolButtons.insert(tbb_Coll_Message,
                         ToolButton(i18n("Message"),
                                    load(dataDir + "message-asynchronous.png"),
                                    QCursor(load(dataDir + "cursor-message-asynchronous.png"), 9, 9)) );
}

#include "worktoolbar.moc"
