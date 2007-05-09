/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "worktoolbar.h"

// qt/kde include files
#include <qmainwindow.h>
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
    setOrientation( Qt::Vertical );
    setVerticalStretchable( true );
    // initialize old tool map, everything starts with select tool (arrow)
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
    insertButton(m_ToolButtons[tbb].Symbol, tbb, true, m_ToolButtons[tbb].Label);
    setToggle(tbb, true);
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
        //insertHotBtn(tbb_DeepHistory);        //NotYetImplemented
        //insertHotBtn(tbb_ShallowHistory);     //NotYetImplemented
        //insertHotBtn(tbb_Join);               //NotYetImplemented
        insertHotBtn(tbb_StateFork);
        //insertHotBtn(tbb_Junction);           //NotYetImplemented
        //insertHotBtn(tbb_Choice);             //NotYetImplemented
        //insertHotBtn(tbb_Andline);            //NotYetImplemented
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
        kWarning() << "slotCheckToolBar() on unknown diagram type:"
        << m_Type << endl;
        break;
    }
}

void WorkToolBar::buttonChanged(int b) {
    UMLView* view = UMLApp::app()->getCurrentView();

    //if trying to turn off arrow - stop it
    ToolBar_Buttons tbb = (ToolBar_Buttons)b;
    if (tbb == tbb_Arrow && m_CurrentButtonID == tbb_Arrow) {
        toggleButton(tbb_Arrow);

        // signal needed, in the case ( when switching diagrams ) that
        // Arrow Button gets activated, but the toolBarState of the Views may be different
        emit sigButtonChanged( m_CurrentButtonID );

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
    return m_ToolButtons[m_CurrentButtonID].Cursor;
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

    UMLView* view = UMLApp::app()->getCurrentView();
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
    QPixmap pxm;
    pxm.load(fileName);
    return pxm;
}

void WorkToolBar::loadPixmaps() {
    const struct ButtonInfo {
        const ToolBar_Buttons tbb;
        const QString btnName;
        const char *pngName;
    } buttonInfo[] = {
        { tbb_Object, i18n("Object"), "object.png" },
        { tbb_Seq_Message_Synchronous, i18n("Synchronous Message"), "message-synchronous.png" },
        { tbb_Seq_Message_Asynchronous, i18n("Asynchronous Message"), "message-asynchronous.png" },
        { tbb_Association, i18n("Association"), "association.png" },
        { tbb_Containment, i18n("Containment"), "containment.png" },
        { tbb_Anchor, i18n("Anchor"), "anchor.png" },
        { tbb_Text, i18n("Label"), "text.png" },
        { tbb_Note, i18n("Note"), "note.png" },
        { tbb_Box, i18n("Box"), "box.png" },
        { tbb_Actor, i18n("Actor"), "actor.png" },
        { tbb_Dependency, i18n("Dependency"), "dependency.png" },
        { tbb_Aggregation, i18n("Aggregation"), "aggregation.png" },
        { tbb_Relationship, i18n("Relationship"), "relationship.png" },
        { tbb_UniAssociation, i18n("Directional Association"), "uniassociation.png" },
        { tbb_Generalization, i18n("Implements (Generalisation/Realisation)"), "generalisation.png" },
        { tbb_Composition, i18n("Composition"), "composition.png" },
        { tbb_UseCase, i18n("Use Case"), "usecase.png" },
        { tbb_Class, i18n("Class"), "class.png" },
        { tbb_Initial_State, i18n("Initial State"), "initial_state.png" },
        { tbb_End_State, i18n("End State"), "end_state.png" },
        { tbb_Branch, i18n("Branch/Merge"), "branch.png" },
        { tbb_Fork, i18n("Fork/Join"), "fork.png" },
        { tbb_Package, i18n("Package"), "package.png" },
        { tbb_Component, i18n("Component"), "component.png" },
        { tbb_Node, i18n("Node"), "node.png" },
        { tbb_Artifact, i18n("Artifact"), "artifact.png" },
        { tbb_Interface, i18n("Interface"), "interface.png" },
        { tbb_Datatype, i18n("Datatype"), "datatype.png" },
        { tbb_Enum, i18n("Enum"), "enum.png" },
        { tbb_Entity, i18n("Entity"), "entity.png" },
        { tbb_DeepHistory, i18n("Deep History"), "deep-history.png" },          //NotYetImplemented
        { tbb_ShallowHistory, i18n("Shallow History"), "shallow-history.png" }, //NotYetImplemented
        { tbb_Join, i18n("Join"), "join.png" },    //NotYetImplemented
        { tbb_StateFork, i18n("Fork"), "state-fork.png" },
        { tbb_Junction, i18n("Junction"), "junction.png" },    //NotYetImplemented
        { tbb_Choice, i18n("Choice"), "choice-round.png" },    //NotYetImplemented
    //:TODO: let the user decide which symbol he wants (setting an option)
    //    { tbb_Choice, i18n("Choice"), "choice-rhomb.png" },    //NotYetImplemented
        //{ tbb_Andline, i18n("And Line"), "andline.png" },    //NotYetImplemented
        { tbb_State_Transition, i18n("State Transition"), "uniassociation.png" },
        { tbb_Activity_Transition, i18n("Activity Transition"), "uniassociation.png" },
        { tbb_Activity, i18n("Activity"), "usecase.png" },
        { tbb_State, i18n("State"), "usecase.png" },
        { tbb_End_Activity, i18n("End Activity"), "end_state.png" },
        { tbb_Initial_Activity, i18n("Initial Activity"), "initial_state.png" },
        { tbb_Coll_Message, i18n("Message"), "message-asynchronous.png" }
    };
    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs->findResourceDir( "data", "umbrello/pics/object.png" );
    dataDir += "/umbrello/pics/";
    const size_t n_buttonInfos = sizeof(buttonInfo) / sizeof(ButtonInfo);

    m_ToolButtons.insert(tbb_Undefined,
                         ToolButton(i18n("UNDEFINED"),
                                    0,
                                    QCursor()) );
    m_ToolButtons.insert(tbb_Arrow,
                         ToolButton(i18n("Select"),
                                    load(dataDir + "arrow.png"),
                                    QCursor()) );
    kDebug() << "WorkToolBar::loadPixmaps: n_buttonInfos = " << n_buttonInfos << endl;
    for (uint i = 0; i < n_buttonInfos; i++) {
        const ButtonInfo& info = buttonInfo[i];
        m_ToolButtons.insert(info.tbb,
            ToolButton(info.btnName,
                       load(dataDir + info.pngName),
                       QCursor(load(dataDir + "cursor-" + info.pngName), 9, 9)));
    }
}

#include "worktoolbar.moc"
