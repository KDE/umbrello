/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//qt include files

//kde include files
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>


// application specific includes
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "worktoolbar.h"
//Added by qt3to4:
#include <QPixmap>
#include <QAction>
#include <QToolButton>

WorkToolBar::WorkToolBar(QMainWindow *parentWindow)
        : KToolBar("worktoolbar", parentWindow,Qt::RightToolBarArea,false,true) {
    m_CurrentButtonID = tbb_Undefined;
    loadPixmaps();
    m_Type = Uml::dt_Class; /* first time in just want it to load arrow,
                           needs anything but dt_Undefined  */
    setOrientation( Qt::Vertical );
//     setVerticalStretchable( true );
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

QAction* WorkToolBar::insertHotBtn(ToolBar_Buttons tbb) {
    QAction* action = addAction(QIcon(m_ToolButtons[tbb].Symbol), m_ToolButtons[tbb].Label, 
        /*receiver*/this, /*member*/m_ToolButtons[tbb].Slot);
    m_actions[tbb] = action;
    action->setChecked(true);
    return action;
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
    QAction* action  = insertHotBtn(tbb_Arrow);
    action->setChecked(true);
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
	insertHotBtn(tbb_Seq_Message_Found);
	insertHotBtn(tbb_Seq_Message_Lost);
        insertHotBtn(tbb_Seq_Combined_Fragment);
	insertHotBtn(tbb_Seq_Precondition);
	insertHotBtn(tbb_Seq_End_Of_Life);
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
        insertHotBtn(tbb_Final_Activity);
        insertHotBtn(tbb_Branch);
        insertHotBtn(tbb_Fork);
        insertHotBtn(tbb_Activity_Transition);
        insertHotBtn(tbb_Exception);
        insertHotBtn(tbb_Object_Flow);
        insertHotBtn(tbb_Send_Signal);
        insertHotBtn(tbb_Accept_Signal);
        insertHotBtn(tbb_Accept_Time_Event);
        insertHotBtn(tbb_Region);
        insertHotBtn(tbb_Pin);
        
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
        m_actions[tbb_Arrow]->toggle();
        view->setCursor( currentCursor() );
        return;
    }

    //if toggling off a button set to arrow
    if (tbb == m_CurrentButtonID) {
        m_map[m_Type] = m_CurrentButtonID;  // store old tool for this diagram type
        m_actions[tbb_Arrow]->toggle();
        m_CurrentButtonID = tbb_Arrow;
        emit sigButtonChanged(m_CurrentButtonID);
        view->setCursor( currentCursor() );
        return;
    }
    m_map[m_Type] = m_CurrentButtonID;
    m_actions[m_CurrentButtonID]->toggle();
    m_CurrentButtonID = tbb;
    emit sigButtonChanged(m_CurrentButtonID);
    view->setCursor( currentCursor() );
}

QCursor WorkToolBar::currentCursor() {
    return m_ToolButtons[m_CurrentButtonID].Cursor;
}

void WorkToolBar::slotResetToolBar() {
    if (m_CurrentButtonID == tbb_Undefined)
        return;
    if (m_CurrentButtonID == tbb_Arrow)
        return;//really shouldn't occur
    m_actions[m_CurrentButtonID]->toggle();
    m_CurrentButtonID = tbb_Arrow;
    m_actions[m_CurrentButtonID]->toggle();
    emit sigButtonChanged(m_CurrentButtonID);

    QCursor curs;
    curs.setShape(Qt::ArrowCursor);

    UMLView* view = UMLApp::app()->getCurrentView();
    if (view != NULL) {
        view -> setCursor(curs);
    }
}

void WorkToolBar::setOldTool() {
    QToolButton *b = (QToolButton*) widgetForAction(m_actions[m_map[m_Type]]);
    if (b)
        b -> animateClick();
}

void WorkToolBar::setDefaultTool() {
    QToolButton *b = (QToolButton*) widgetForAction(m_actions[tbb_Arrow]);
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
        const char *slotName;
    } buttonInfo[] = {

        { tbb_Object, i18n("Object"), "object.png", SLOT(slotObject()) },
        { tbb_Seq_Message_Synchronous, i18n("Synchronous Message"), "message-synchronous.png", SLOT(slotSeq_Message_Synchronous()) },
        { tbb_Seq_Message_Asynchronous, i18n("Asynchronous Message"), "message-asynchronous.png", SLOT(slotSeq_Message_Asynchronous()) },
        { tbb_Seq_Message_Found, i18n("Found Message"), "message-found.png", SLOT(slotSeq_Message_Found()) },
        { tbb_Seq_Message_Lost, i18n("Lost Message"), "message-lost.png", SLOT(slotSeq_Message_Lost()) },
        { tbb_Seq_Combined_Fragment, i18n("Combined Fragment"), "combined_fragment.png", SLOT(slotSeq_Combined_Fragment()) },
	{ tbb_Seq_End_Of_Life, i18n("End of life"), "end_of_life.png", SLOT(slotSeq_End_Of_Life()) },
	{ tbb_Seq_Precondition, i18n("Precondition"),"precondition.png",SLOT(slotSeq_Precondition()) },
        { tbb_Association, i18n("Association"), "association.png", SLOT(slotAssociation()) },
        { tbb_Containment, i18n("Containment"), "containment.png", SLOT(slotContainment()) },
        { tbb_Anchor, i18n("Anchor"), "anchor.png", SLOT(slotAnchor()) },
        { tbb_Text, i18n("Label"), "text.png", SLOT(slotText()) },
        { tbb_Note, i18n("Note"), "note.png", SLOT(slotNote()) },
        { tbb_Box, i18n("Box"), "box.png", SLOT(slotBox()) },
        { tbb_Actor, i18n("Actor"), "actor.png", SLOT(slotActor()) },
        { tbb_Dependency, i18n("Dependency"), "dependency.png", SLOT(slotDependency()) },
        { tbb_Aggregation, i18n("Aggregation"), "aggregation.png", SLOT(slotAggregation()) },
        { tbb_Relationship, i18n("Relationship"), "relationship.png", SLOT(slotRelationship()) },
        { tbb_UniAssociation, i18n("Directional Association"), "uniassociation.png", SLOT(slotUniAssociation()) },
        { tbb_Generalization, i18n("Implements (Generalisation/Realisation)"), "generalisation.png", SLOT(slotGeneralization()) },
        { tbb_Composition, i18n("Composition"), "composition.png", SLOT(slotComposition()) },
        { tbb_UseCase, i18n("Use Case"), "usecase.png", SLOT(slotUseCase()) },
        { tbb_Class, i18n("Class"), "class.png", SLOT(slotClass()) },
        { tbb_Initial_State, i18n("Initial State"), "initial_state.png", SLOT(slotInitial_State()) },
        { tbb_Region, i18n("Region"), "region.png", SLOT(slotRegion()) },
        { tbb_End_State, i18n("End State"), "end_state.png", SLOT(slotEnd_State()) },
        { tbb_Branch, i18n("Branch/Merge"), "branch.png", SLOT(slotBranch()) },
        { tbb_Send_Signal, i18n("Send signal"), "send_signal.png", SLOT(slotSend_Signal()) },
        { tbb_Accept_Signal, i18n("Accept signal"), "accept_signal.png", SLOT(slotAccept_Signal()) },
        { tbb_Accept_Time_Event, i18n("Accept time event"), "accept_time_event.png", SLOT(slotAccept_Time_Event()) },
        { tbb_Fork, i18n("Fork/Join"), "fork.png", SLOT(slotFork()) },
        { tbb_Package, i18n("Package"), "package.png", SLOT(slotPackage()) },
        { tbb_Component, i18n("Component"), "component.png", SLOT(slotComponent()) },
        { tbb_Node, i18n("Node"), "node.png", SLOT(slotNode()) },
        { tbb_Artifact, i18n("Artifact"), "artifact.png", SLOT(slotArtifact()) },
        { tbb_Interface, i18n("Interface"), "interface.png", SLOT(slotInterface()) },
        { tbb_Datatype, i18n("Datatype"), "datatype.png", SLOT(slotDatatype()) },
        { tbb_Enum, i18n("Enum"), "enum.png", SLOT(slotEnum()) },
        { tbb_Entity, i18n("Entity"), "entity.png", SLOT(slotEntity()) },
        { tbb_DeepHistory, i18n("Deep History"), "deep-history.png", SLOT(slotDeepHistory()) },          //NotYetImplemented
        { tbb_ShallowHistory, i18n("Shallow History"), "shallow-history.png", SLOT(slotShallowHistory()) }, //NotYetImplemented
        { tbb_Join, i18n("Join"), "join.png", SLOT(slotJoin()) },    //NotYetImplemented
        { tbb_StateFork, i18n("Fork"), "state-fork.png", SLOT(slotStateFork()) },
        { tbb_Junction, i18n("Junction"), "junction.png", SLOT(slotJunction()) },    //NotYetImplemented
        { tbb_Choice, i18n("Choice"), "choice-round.png", SLOT(slotChoice()) },    //NotYetImplemented
    //:TODO: let the user decide which symbol he wants (setting an option)
    //    { tbb_Choice, i18n("Choice"), "choice-rhomb.png", SLOT(slotChoice()) },    //NotYetImplemented
        //{ tbb_Andline, i18n("And Line"), "andline.png", SLOT(slotAndline()) },    //NotYetImplemented
        { tbb_State_Transition, i18n("State Transition"), "uniassociation.png", SLOT(slotState_Transition()) },
        { tbb_Activity_Transition, i18n("Activity Transition"), "uniassociation.png", SLOT(slotActivity_Transition()) },
        { tbb_Activity, i18n("Activity"), "usecase.png", SLOT(slotActivity()) },
        { tbb_State, i18n("State"), "usecase.png", SLOT(slotState()) },
        { tbb_End_Activity, i18n("End Activity"), "end_state.png", SLOT(slotEnd_Activity()) },
        { tbb_Final_Activity, i18n("Final Activity"), "final_activity.png", SLOT(slotFinal_Activity()) },
        { tbb_Object_Flow, i18n("Object Flow"), "object_flow.png", SLOT(slotObject_Flow()) },
        { tbb_Pin, i18n("Pin"), "pin.png", SLOT(slotPin()) },
        { tbb_Initial_Activity, i18n("Initial Activity"), "initial_state.png", SLOT(slotInitial_Activity()) },
        { tbb_Coll_Message, i18n("Message"), "message-asynchronous.png", SLOT(slotColl_Message()) },
        { tbb_Exception, i18n("Exception"), "exception.png", SLOT(slotException()) }
    };

    KStandardDirs * dirs = KGlobal::dirs();
    QString dataDir = dirs->findResourceDir( "data", "umbrello/pics/object.png" );
    dataDir += "/umbrello/pics/";
    const size_t n_buttonInfos = sizeof(buttonInfo) / sizeof(ButtonInfo);

/*    m_ToolButtons.insert(tbb_Undefined,
                         ToolButton(i18n("UNDEFINED"),
                                    load(dataDir + "arrow.png"),
                                    QCursor(),
                                    SLOT(slotArrow())) );*/
    m_ToolButtons.insert(tbb_Arrow,
                         ToolButton(i18n("Select"),
                                    load(dataDir + "arrow.png"),
                                    QCursor(),
                                    SLOT(slotArrow())) );

    kDebug() << "WorkToolBar::loadPixmaps: n_buttonInfos = " << n_buttonInfos << endl;
    for (uint i = 0; i < n_buttonInfos; i++) {
        const ButtonInfo& info = buttonInfo[i];
        m_ToolButtons.insert(info.tbb,
            ToolButton(info.btnName,
                       load(dataDir + info.pngName),
                       QCursor(load(dataDir + "cursor-" + info.pngName), 9, 9),
                       info.slotName));
    }
}

void WorkToolBar::slotArrow() {buttonChanged(tbb_Arrow);}
void WorkToolBar::slotGeneralization() {buttonChanged(tbb_Generalization);}
void WorkToolBar::slotAggregation() {buttonChanged(tbb_Aggregation);}
void WorkToolBar::slotDependency() {buttonChanged(tbb_Dependency);}
void WorkToolBar::slotAssociation() {buttonChanged(tbb_Association);}
void WorkToolBar::slotContainment() {buttonChanged(tbb_Containment);}
void WorkToolBar::slotColl_Message() {buttonChanged(tbb_Coll_Message);}
void WorkToolBar::slotSeq_Message_Synchronous() {buttonChanged(tbb_Seq_Message_Synchronous);}
void WorkToolBar::slotSeq_Message_Asynchronous(){buttonChanged(tbb_Seq_Message_Asynchronous);}
void WorkToolBar::slotSeq_Message_Found(){buttonChanged(tbb_Seq_Message_Found);}
void WorkToolBar::slotSeq_Message_Lost(){buttonChanged(tbb_Seq_Message_Lost);}
void WorkToolBar::slotSeq_Combined_Fragment(){buttonChanged(tbb_Seq_Combined_Fragment);}
void WorkToolBar::slotSeq_Precondition(){buttonChanged(tbb_Seq_Precondition);}
void WorkToolBar::slotSeq_End_Of_Life(){buttonChanged(tbb_Seq_End_Of_Life);}
void WorkToolBar::slotComposition() {buttonChanged(tbb_Composition);}
void WorkToolBar::slotRelationship() {buttonChanged(tbb_Relationship);}
void WorkToolBar::slotUniAssociation() {buttonChanged(tbb_UniAssociation);}
void WorkToolBar::slotState_Transition() {buttonChanged(tbb_State_Transition);}
void WorkToolBar::slotActivity_Transition() {buttonChanged(tbb_Activity_Transition);}
void WorkToolBar::slotAnchor() {buttonChanged(tbb_Anchor);}
void WorkToolBar::slotNote() {buttonChanged(tbb_Note);}
void WorkToolBar::slotBox() {buttonChanged(tbb_Box);}
void WorkToolBar::slotText() {buttonChanged(tbb_Text);}
void WorkToolBar::slotActor() {buttonChanged(tbb_Actor);}
void WorkToolBar::slotUseCase() {buttonChanged(tbb_UseCase);}
void WorkToolBar::slotClass() {buttonChanged(tbb_Class);}
void WorkToolBar::slotInterface() {buttonChanged(tbb_Interface);}
void WorkToolBar::slotDatatype() {buttonChanged(tbb_Datatype);}
void WorkToolBar::slotEnum() {buttonChanged(tbb_Enum);}
void WorkToolBar::slotEntity() {buttonChanged(tbb_Entity);}
void WorkToolBar::slotPackage() {buttonChanged(tbb_Package);}
void WorkToolBar::slotComponent() {buttonChanged(tbb_Component);}
void WorkToolBar::slotNode() {buttonChanged(tbb_Node);}
void WorkToolBar::slotArtifact() {buttonChanged(tbb_Artifact);}
void WorkToolBar::slotObject() {buttonChanged(tbb_Object);}
void WorkToolBar::slotInitial_State() {buttonChanged(tbb_Initial_State);}
void WorkToolBar::slotState() {buttonChanged(tbb_State);}
void WorkToolBar::slotSend_Signal() {buttonChanged(tbb_Send_Signal);}
void WorkToolBar::slotAccept_Signal() {buttonChanged(tbb_Accept_Signal);}
void WorkToolBar::slotAccept_Time_Event() {buttonChanged(tbb_Accept_Time_Event);}
void WorkToolBar::slotEnd_State() {buttonChanged(tbb_End_State);}
void WorkToolBar::slotRegion() {buttonChanged(tbb_Region);}
void WorkToolBar::slotInitial_Activity() {buttonChanged(tbb_Initial_Activity);}
void WorkToolBar::slotActivity() {buttonChanged(tbb_Activity);}
void WorkToolBar::slotObject_Flow() {buttonChanged(tbb_Object_Flow);}
void WorkToolBar::slotEnd_Activity() {buttonChanged(tbb_End_Activity);}
void WorkToolBar::slotFinal_Activity() {buttonChanged(tbb_Final_Activity);}
void WorkToolBar::slotBranch() {buttonChanged(tbb_Branch);}
void WorkToolBar::slotFork() {buttonChanged(tbb_Fork);}
void WorkToolBar::slotDeepHistory() {buttonChanged(tbb_DeepHistory);}
void WorkToolBar::slotShallowHistory() {buttonChanged(tbb_ShallowHistory);}
void WorkToolBar::slotJoin() {buttonChanged(tbb_Join);}
void WorkToolBar::slotPin() {buttonChanged(tbb_Pin);}
void WorkToolBar::slotStateFork() {buttonChanged(tbb_StateFork);}
void WorkToolBar::slotJunction() {buttonChanged(tbb_Junction);}
void WorkToolBar::slotChoice() {buttonChanged(tbb_Choice);}
void WorkToolBar::slotAndline() {buttonChanged(tbb_Andline);}
void WorkToolBar::slotException() {buttonChanged(tbb_Exception);}


#include "worktoolbar.moc"
