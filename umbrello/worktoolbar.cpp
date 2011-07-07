/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "worktoolbar.h"

// application specific includes
#include "debug_utils.h"
#include "icon_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// kde include files
#include <klocale.h>

// qt include files
#include <QtGui/QAction>
#include <QtGui/QToolButton>

/**
 * Creates a work tool bar.
 *
 * @param parentWindow      The parent of the toolbar.
 */
WorkToolBar::WorkToolBar(QMainWindow *parentWindow)
  : KToolBar("worktoolbar", parentWindow, Qt::TopToolBarArea, true, true, true)
{
    m_CurrentButtonID = tbb_Undefined;
    loadPixmaps();
    m_Type = Uml::DiagramType::Class; // first time in just want it to load arrow,
                                      // needs anything but Uml::DiagramType::Undefined
    setOrientation(Qt::Vertical);
//     setVerticalStretchable( true );
    // initialize old tool map, everything starts with select tool (arrow)
    m_map.insert(Uml::DiagramType::UseCase, tbb_Arrow);
    m_map.insert(Uml::DiagramType::Collaboration, tbb_Arrow);
    m_map.insert(Uml::DiagramType::Class, tbb_Arrow);
    m_map.insert(Uml::DiagramType::Sequence, tbb_Arrow);
    m_map.insert(Uml::DiagramType::State, tbb_Arrow);
    m_map.insert(Uml::DiagramType::Activity, tbb_Arrow);
    m_map.insert(Uml::DiagramType::Undefined, tbb_Arrow);

    slotCheckToolBar(Uml::DiagramType::Undefined);
}

/**
 * Standard destructor.
 */
WorkToolBar::~WorkToolBar()
{
}

/**
 * Inserts the button corresponding to the tbb value given
 * and activates the toggle.
 */
QAction* WorkToolBar::insertHotBtn(ToolBar_Buttons tbb)
{
    QAction* action = addAction(QIcon(m_ToolButtons[tbb].Symbol), m_ToolButtons[tbb].Label,
                                /*receiver*/this, /*member*/m_ToolButtons[tbb].Slot);
    m_actions[tbb] = action;
    action->setChecked(true);
    return action;
}

/**
 * Inserts most associations, just reduces some string
 * duplication (nice to translators)
 */
void WorkToolBar::insertBasicAssociations()
{
    insertHotBtn(tbb_Association);
    if (m_Type == Uml::DiagramType::Class || m_Type == Uml::DiagramType::UseCase) {
        insertHotBtn(tbb_UniAssociation);
    }
    insertHotBtn(tbb_Dependency);
    insertHotBtn(tbb_Generalization);
}

void WorkToolBar::slotCheckToolBar(Uml::DiagramType dt)
{
    if (dt == m_Type)
        return;
    clear();
    m_Type = dt;

    if (m_Type == Uml::DiagramType::Undefined)
        return;

    // insert note, anchor and lines of text on all diagrams
    QAction* action  = insertHotBtn(tbb_Arrow);
    action->setChecked(true);
    m_CurrentButtonID = tbb_Arrow;

    insertHotBtn(tbb_Note);
    insertHotBtn(tbb_Anchor);
    insertHotBtn(tbb_Text);
    insertHotBtn(tbb_Box);

    // insert diagram specific tools
    switch (m_Type) {
    case Uml::DiagramType::UseCase:
        insertHotBtn(tbb_Actor);
        insertHotBtn(tbb_UseCase);
        insertBasicAssociations();
        break;

    case Uml::DiagramType::Class:
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

    case Uml::DiagramType::Sequence:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Seq_Message_Synchronous);
        insertHotBtn(tbb_Seq_Message_Asynchronous);
        insertHotBtn(tbb_Seq_Message_Found);
        insertHotBtn(tbb_Seq_Message_Lost);
        insertHotBtn(tbb_Seq_Combined_Fragment);
        insertHotBtn(tbb_Seq_Precondition);
        break;

    case Uml::DiagramType::Collaboration:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Coll_Message);
        break;

    case Uml::DiagramType::State:
        insertHotBtn(tbb_Initial_State);
        insertHotBtn(tbb_State);
        insertHotBtn(tbb_End_State);
        insertHotBtn(tbb_State_Transition);
        insertHotBtn(tbb_DeepHistory);
        insertHotBtn(tbb_ShallowHistory);
        insertHotBtn(tbb_StateJoin);
        insertHotBtn(tbb_StateFork);
        insertHotBtn(tbb_Junction);
        insertHotBtn(tbb_Choice);
        //insertHotBtn(tbb_Andline);            //NotYetImplemented
        break;

    case Uml::DiagramType::Activity:
        insertHotBtn(tbb_Initial_Activity);
        insertHotBtn(tbb_Activity);
        insertHotBtn(tbb_End_Activity);
        insertHotBtn(tbb_Final_Activity);
        insertHotBtn(tbb_Branch);
        insertHotBtn(tbb_Fork);
        insertHotBtn(tbb_Activity_Transition);
        insertHotBtn(tbb_Exception);
        insertHotBtn(tbb_PrePostCondition);
        insertHotBtn(tbb_Send_Signal);
        insertHotBtn(tbb_Accept_Signal);
        insertHotBtn(tbb_Accept_Time_Event);
        insertHotBtn(tbb_Region);
        insertHotBtn(tbb_Pin);
        insertHotBtn(tbb_Object_Node);
        break;

    case Uml::DiagramType::Component:
        insertHotBtn(tbb_Interface);
        insertHotBtn(tbb_Component);
        insertHotBtn(tbb_Artifact);
        insertBasicAssociations();
        break;

    case Uml::DiagramType::Deployment:
        insertHotBtn(tbb_Object);
        insertHotBtn(tbb_Interface);
        insertHotBtn(tbb_Component);
        insertHotBtn(tbb_Node);
        insertBasicAssociations();
        break;

    case Uml::DiagramType::EntityRelationship:
        insertHotBtn(tbb_Entity);
        insertHotBtn(tbb_Category);
        insertHotBtn(tbb_Relationship);
        insertHotBtn(tbb_Category2Parent);
        insertHotBtn(tbb_Child2Category);
        break;

    default:
        uWarning() << "slotCheckToolBar() on unknown diagram type:" << m_Type.toString();
        break;
    }
}

void WorkToolBar::buttonChanged(int b)
{
    UMLView* view = UMLApp::app()->currentView();

    // if trying to turn off arrow - stop it
    ToolBar_Buttons tbb = (ToolBar_Buttons)b;
    if (tbb == tbb_Arrow && m_CurrentButtonID == tbb_Arrow) {
        m_actions[tbb_Arrow]->toggle();

        // signal needed, in the case ( when switching diagrams ) that
        // Arrow Button gets activated, but the toolBarState of the Views may be different
        emit sigButtonChanged(m_CurrentButtonID);

        view->setCursor(currentCursor());
        return;
    }

    // if toggling off a button set to arrow
    if (tbb == m_CurrentButtonID) {
        m_map[m_Type] = m_CurrentButtonID;  // store old tool for this diagram type
        m_actions[tbb_Arrow]->toggle();
        m_CurrentButtonID = tbb_Arrow;
        emit sigButtonChanged(m_CurrentButtonID);
        view->setCursor(currentCursor());
        return;
    }
    m_map[m_Type] = m_CurrentButtonID;
    m_actions[m_CurrentButtonID]->toggle();
    m_CurrentButtonID = tbb;
    emit sigButtonChanged(m_CurrentButtonID);
    view->setCursor(currentCursor());
}

/**
 * Returns the current cursor depending on m_CurrentButtonID
 */
QCursor WorkToolBar::currentCursor()
{
    return m_ToolButtons[m_CurrentButtonID].Cursor;
}

void WorkToolBar::slotResetToolBar()
{
    if (m_CurrentButtonID == tbb_Undefined)
        return;
    if (m_CurrentButtonID == tbb_Arrow)
        return;  // really shouldn't occur
    m_actions[m_CurrentButtonID]->toggle();
    m_CurrentButtonID = tbb_Arrow;
    m_actions[m_CurrentButtonID]->toggle();
    emit sigButtonChanged(m_CurrentButtonID);

    QCursor curs;
    curs.setShape(Qt::ArrowCursor);

    UMLView* view = UMLApp::app()->currentView();
    if (view != NULL) {
        view->setCursor(curs);
    }
}

/**
 * Sets the current tool to the previously used Tool. This is just
 * as if the user had pressed the button for the other tool.
 */
void WorkToolBar::setOldTool()
{
    QToolButton *b = (QToolButton*) widgetForAction(m_actions[m_map[m_Type]]);
    if (b)
        b->animateClick();
}

/**
 * Sets the current tool to the default tool. (select tool)
 * Calling this function is as if the user had pressed the "arrow"
 * button on the toolbar.
 */
void WorkToolBar::setDefaultTool()
{
    QToolButton *b = (QToolButton*) widgetForAction(m_actions[tbb_Arrow]);
    if (b)
        b->animateClick();
}

/**
 * Loads toolbar icon and mouse cursor images from disk
 */
void WorkToolBar::loadPixmaps()
{
    const struct ButtonInfo {
        const ToolBar_Buttons tbb;
        const QString btnName;
        const Icon_Utils::IconType icon;
        const char *slotName;
    } buttonInfo[] = {

        { tbb_Object,                   i18n("Object"),                  Icon_Utils::it_Object,                  SLOT(slotObject()) },
        { tbb_Seq_Message_Synchronous,  i18n("Synchronous Message"),     Icon_Utils::it_Message_Sync,            SLOT(slotSeq_Message_Synchronous()) },
        { tbb_Seq_Message_Asynchronous, i18n("Asynchronous Message"),    Icon_Utils::it_Message_Async,           SLOT(slotSeq_Message_Asynchronous()) },
        { tbb_Seq_Message_Found,        i18n("Found Message"),           Icon_Utils::it_Message_Found,           SLOT(slotSeq_Message_Found()) },
        { tbb_Seq_Message_Lost,         i18n("Lost Message"),            Icon_Utils::it_Message_Lost,            SLOT(slotSeq_Message_Lost()) },
        { tbb_Seq_Combined_Fragment,    i18n("Combined Fragment"),       Icon_Utils::it_Combined_Fragment,       SLOT(slotSeq_Combined_Fragment()) },
        { tbb_Seq_Precondition,         i18n("Precondition"),            Icon_Utils::it_Precondition,            SLOT(slotSeq_Precondition()) },
        { tbb_Association,              i18n("Association"),             Icon_Utils::it_Association,             SLOT(slotAssociation()) },
        { tbb_Containment,              i18n("Containment"),             Icon_Utils::it_Containment,             SLOT(slotContainment()) },
        { tbb_Anchor,                   i18n("Anchor"),                  Icon_Utils::it_Anchor,                  SLOT(slotAnchor()) },
        { tbb_Text,                     i18n("Label"),                   Icon_Utils::it_Text,                    SLOT(slotText()) },
        { tbb_Note,                     i18n("Note"),                    Icon_Utils::it_Note,                    SLOT(slotNote()) },
        { tbb_Box,                      i18n("Box"),                     Icon_Utils::it_Box,                     SLOT(slotBox()) },
        { tbb_Actor,                    i18n("Actor"),                   Icon_Utils::it_Actor,                   SLOT(slotActor()) },
        { tbb_Dependency,               i18n("Dependency"),              Icon_Utils::it_Dependency,              SLOT(slotDependency()) },
        { tbb_Aggregation,              i18n("Aggregation"),             Icon_Utils::it_Aggregation,             SLOT(slotAggregation()) },
        { tbb_Relationship,             i18n("Relationship"),            Icon_Utils::it_Relationship,            SLOT(slotRelationship()) },
        { tbb_UniAssociation,           i18n("Directional Association"), Icon_Utils::it_Directional_Association, SLOT(slotUniAssociation()) },
        { tbb_Generalization,           i18n("Implements"),              Icon_Utils::it_Implements,              SLOT(slotGeneralization()) },
        { tbb_Composition,              i18n("Composition"),             Icon_Utils::it_Composition,             SLOT(slotComposition()) },
        { tbb_UseCase,                  i18n("Use Case"),                Icon_Utils::it_UseCase,                 SLOT(slotUseCase()) },
        { tbb_Class,                    i18nc("UML class", "Class"),     Icon_Utils::it_Class,                   SLOT(slotClass()) },
        { tbb_Initial_State,            i18n("Initial State"),           Icon_Utils::it_InitialState,            SLOT(slotInitial_State()) },
        { tbb_Region,                   i18n("Region"),                  Icon_Utils::it_Region,                  SLOT(slotRegion()) },
        { tbb_End_State,                i18n("End State"),               Icon_Utils::it_EndState,                SLOT(slotEnd_State()) },
        { tbb_Branch,                   i18n("Branch/Merge"),            Icon_Utils::it_Branch,                  SLOT(slotBranch()) },
        { tbb_Send_Signal,              i18n("Send signal"),             Icon_Utils::it_Send_Signal,             SLOT(slotSend_Signal()) },
        { tbb_Accept_Signal,            i18n("Accept signal"),           Icon_Utils::it_Accept_Signal,           SLOT(slotAccept_Signal()) },
        { tbb_Accept_Time_Event,        i18n("Accept time event"),       Icon_Utils::it_Accept_TimeEvent,        SLOT(slotAccept_Time_Event()) },
        { tbb_Fork,                     i18n("Fork/Join"),               Icon_Utils::it_Fork_Join,               SLOT(slotFork()) },
        { tbb_Package,                  i18n("Package"),                 Icon_Utils::it_Package,                 SLOT(slotPackage()) },
        { tbb_Component,                i18n("Component"),               Icon_Utils::it_Component,               SLOT(slotComponent()) },
        { tbb_Node,                     i18n("Node"),                    Icon_Utils::it_Node,                    SLOT(slotNode()) },
        { tbb_Artifact,                 i18n("Artifact"),                Icon_Utils::it_Artifact,                SLOT(slotArtifact()) },
        { tbb_Interface,                i18n("Interface"),               Icon_Utils::it_Interface,               SLOT(slotInterface()) },
        { tbb_Datatype,                 i18n("Datatype"),                Icon_Utils::it_Datatype,                SLOT(slotDatatype()) },
        { tbb_Enum,                     i18n("Enum"),                    Icon_Utils::it_Enum,                    SLOT(slotEnum()) },
        { tbb_Entity,                   i18n("Entity"),                  Icon_Utils::it_Entity,                  SLOT(slotEntity()) },
        { tbb_DeepHistory,              i18n("Deep History"),            Icon_Utils::it_History_Deep,            SLOT(slotDeepHistory()) },
        { tbb_ShallowHistory,           i18n("Shallow History"),         Icon_Utils::it_History_Shallow,         SLOT(slotShallowHistory()) },
        { tbb_StateJoin,                i18nc("join states", "Join"),    Icon_Utils::it_Join,                    SLOT(slotStateJoin()) },
        { tbb_StateFork,                i18n("Fork"),                    Icon_Utils::it_Fork_State,              SLOT(slotStateFork()) },
        { tbb_Junction,                 i18n("Junction"),                Icon_Utils::it_Junction,                SLOT(slotJunction()) },
        { tbb_Choice,                   i18nc("state choice", "Choice"), Icon_Utils::it_Choice_Rhomb,            SLOT(slotChoice()) },
        //:TODO: let the user decide which symbol he wants (setting an option)
        //{ tbb_Choice,                   i18n("Choice"),                  Icon_Utils::it_Choice_Round,            SLOT(slotChoice()) },          //NotYetImplemented
        { tbb_Andline,                  i18n("And Line"),                Icon_Utils::it_And_Line,                SLOT(slotAndline()) },         //NotYetImplemented
        { tbb_State_Transition,         i18n("State Transition"),        Icon_Utils::it_State_Transition,        SLOT(slotState_Transition()) },
        { tbb_Activity_Transition,      i18n("Activity Transition"),     Icon_Utils::it_Activity_Transition,     SLOT(slotActivity_Transition()) },
        { tbb_Activity,                 i18n("Activity"),                Icon_Utils::it_Activity,                SLOT(slotActivity()) },
        { tbb_State,                    i18nc("state diagram", "State"), Icon_Utils::it_State,                   SLOT(slotState()) },
        { tbb_End_Activity,             i18n("End Activity"),            Icon_Utils::it_Activity_End,            SLOT(slotEnd_Activity()) },
        { tbb_Final_Activity,           i18n("Final Activity"),          Icon_Utils::it_Activity_Final,          SLOT(slotFinal_Activity()) },
        { tbb_Pin,                      i18n("Pin"),                     Icon_Utils::it_Pin,                     SLOT(slotPin()) },
        { tbb_Initial_Activity,         i18n("Initial Activity"),        Icon_Utils::it_Activity_Initial,        SLOT(slotInitial_Activity()) },
        { tbb_Coll_Message,             i18n("Message"),                 Icon_Utils::it_Message,                 SLOT(slotColl_Message()) },
        { tbb_Exception,                i18n("Exception"),               Icon_Utils::it_Exception,               SLOT(slotException()) },
        { tbb_Object_Node,              i18n("Object Node"),             Icon_Utils::it_Object_Node,             SLOT(slotObject_Node()) },
        { tbb_PrePostCondition,         i18n("Pre/Post condition"),      Icon_Utils::it_Condition_PrePost,       SLOT(slotPrePostCondition()) },
        { tbb_Category,                 i18n("Category"),                Icon_Utils::it_Category,                SLOT(slotCategory())  },
        { tbb_Category2Parent,          i18n("Category to Parent"),      Icon_Utils::it_Category_Parent,         SLOT(slotCategory2Parent()) },
        { tbb_Child2Category,           i18n("Child to Category"),       Icon_Utils::it_Category_Child,          SLOT(slotChild2Category()) }
    };

    const size_t n_buttonInfos = sizeof(buttonInfo) / sizeof(ButtonInfo);

    /*    m_ToolButtons.insert(tbb_Undefined,
                             ToolButton(i18n("UNDEFINED"),
                                        Icon_Utils::BarIcon(Icon_Utils::Arrow),
                                        QCursor(),
                                        SLOT(slotArrow())));    */
    m_ToolButtons.insert(tbb_Arrow,
                         ToolButton(i18nc("selection arrow", "Select"),
                                    Icon_Utils::BarIcon(Icon_Utils::it_Arrow),
                                    QCursor(),
                                    SLOT(slotArrow())));

    for (uint i = 0; i < n_buttonInfos; ++i) {
        const ButtonInfo& info = buttonInfo[i];
        m_ToolButtons.insert(info.tbb,
                             ToolButton(info.btnName,
                                        Icon_Utils::BarIcon(info.icon),
                                        Icon_Utils::Cursor(info.icon),
                                        info.slotName));
    }
}

/**
 * These slots are triggered by the buttons. They call buttonChanged with
 * the button id
 */
void WorkToolBar::slotArrow()                    { buttonChanged(tbb_Arrow);                    }
void WorkToolBar::slotGeneralization()           { buttonChanged(tbb_Generalization);           }
void WorkToolBar::slotAggregation()              { buttonChanged(tbb_Aggregation);              }
void WorkToolBar::slotDependency()               { buttonChanged(tbb_Dependency);               }
void WorkToolBar::slotAssociation()              { buttonChanged(tbb_Association);              }
void WorkToolBar::slotContainment()              { buttonChanged(tbb_Containment);              }
void WorkToolBar::slotColl_Message()             { buttonChanged(tbb_Coll_Message);             }
void WorkToolBar::slotSeq_Message_Synchronous()  { buttonChanged(tbb_Seq_Message_Synchronous);  }
void WorkToolBar::slotSeq_Message_Asynchronous() { buttonChanged(tbb_Seq_Message_Asynchronous); }
void WorkToolBar::slotSeq_Message_Found()        { buttonChanged(tbb_Seq_Message_Found);        }
void WorkToolBar::slotSeq_Message_Lost()         { buttonChanged(tbb_Seq_Message_Lost);         }
void WorkToolBar::slotSeq_Combined_Fragment()    { buttonChanged(tbb_Seq_Combined_Fragment);    }
void WorkToolBar::slotSeq_Precondition()         { buttonChanged(tbb_Seq_Precondition);         }
void WorkToolBar::slotComposition()              { buttonChanged(tbb_Composition);              }
void WorkToolBar::slotRelationship()             { buttonChanged(tbb_Relationship);             }
void WorkToolBar::slotUniAssociation()           { buttonChanged(tbb_UniAssociation);           }
void WorkToolBar::slotState_Transition()         { buttonChanged(tbb_State_Transition);         }
void WorkToolBar::slotActivity_Transition()      { buttonChanged(tbb_Activity_Transition);      }
void WorkToolBar::slotAnchor()                   { buttonChanged(tbb_Anchor);                   }
void WorkToolBar::slotNote()                     { buttonChanged(tbb_Note);                     }
void WorkToolBar::slotBox()                      { buttonChanged(tbb_Box);                      }
void WorkToolBar::slotText()                     { buttonChanged(tbb_Text);                     }
void WorkToolBar::slotActor()                    { buttonChanged(tbb_Actor);                    }
void WorkToolBar::slotUseCase()                  { buttonChanged(tbb_UseCase);                  }
void WorkToolBar::slotClass()                    { buttonChanged(tbb_Class);                    }
void WorkToolBar::slotInterface()                { buttonChanged(tbb_Interface);                }
void WorkToolBar::slotDatatype()                 { buttonChanged(tbb_Datatype);                 }
void WorkToolBar::slotEnum()                     { buttonChanged(tbb_Enum);                     }
void WorkToolBar::slotEntity()                   { buttonChanged(tbb_Entity);                   }
void WorkToolBar::slotPackage()                  { buttonChanged(tbb_Package);                  }
void WorkToolBar::slotComponent()                { buttonChanged(tbb_Component);                }
void WorkToolBar::slotNode()                     { buttonChanged(tbb_Node);                     }
void WorkToolBar::slotArtifact()                 { buttonChanged(tbb_Artifact);                 }
void WorkToolBar::slotObject()                   { buttonChanged(tbb_Object);                   }
void WorkToolBar::slotInitial_State()            { buttonChanged(tbb_Initial_State);            }
void WorkToolBar::slotState()                    { buttonChanged(tbb_State);                    }
void WorkToolBar::slotSend_Signal()              { buttonChanged(tbb_Send_Signal);              }
void WorkToolBar::slotAccept_Signal()            { buttonChanged(tbb_Accept_Signal);            }
void WorkToolBar::slotAccept_Time_Event()        { buttonChanged(tbb_Accept_Time_Event);        }
void WorkToolBar::slotEnd_State()                { buttonChanged(tbb_End_State);                }
void WorkToolBar::slotRegion()                   { buttonChanged(tbb_Region);                   }
void WorkToolBar::slotInitial_Activity()         { buttonChanged(tbb_Initial_Activity);         }
void WorkToolBar::slotActivity()                 { buttonChanged(tbb_Activity);                 }
void WorkToolBar::slotEnd_Activity()             { buttonChanged(tbb_End_Activity);             }
void WorkToolBar::slotFinal_Activity()           { buttonChanged(tbb_Final_Activity);           }
void WorkToolBar::slotBranch()                   { buttonChanged(tbb_Branch);                   }
void WorkToolBar::slotFork()                     { buttonChanged(tbb_Fork);                     }
void WorkToolBar::slotDeepHistory()              { buttonChanged(tbb_DeepHistory);              }
void WorkToolBar::slotShallowHistory()           { buttonChanged(tbb_ShallowHistory);           }
void WorkToolBar::slotStateJoin()                { buttonChanged(tbb_StateJoin);                }
void WorkToolBar::slotPin()                      { buttonChanged(tbb_Pin);                      }
void WorkToolBar::slotStateFork()                { buttonChanged(tbb_StateFork);                }
void WorkToolBar::slotJunction()                 { buttonChanged(tbb_Junction);                 }
void WorkToolBar::slotChoice()                   { buttonChanged(tbb_Choice);                   }
void WorkToolBar::slotAndline()                  { buttonChanged(tbb_Andline);                  }
void WorkToolBar::slotException()                { buttonChanged(tbb_Exception);                }
void WorkToolBar::slotObject_Node()              { buttonChanged(tbb_Object_Node);              }
void WorkToolBar::slotPrePostCondition()         { buttonChanged(tbb_PrePostCondition);         }
void WorkToolBar::slotCategory()                 { buttonChanged(tbb_Category);                 }
void WorkToolBar::slotCategory2Parent()          { buttonChanged(tbb_Category2Parent);          }
void WorkToolBar::slotChild2Category()           { buttonChanged(tbb_Child2Category);           }

#include "worktoolbar.moc"
