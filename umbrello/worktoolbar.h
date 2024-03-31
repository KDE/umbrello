/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef WORKTOOLBAR_H
#define WORKTOOLBAR_H

#include "basictypes.h"

#include <ktoolbar.h>

#include <QCursor>
#include <QMap>
#include <QPixmap>

class QMainWindow;
class KAction;

/**
 * This is the toolbar that is displayed on the right-hand side of the program
 * window.  For each type of diagram it will change to suit that document.
 *
 * To add a new tool button do the following:
 * - create a button pixmap (symbol)
 * - create a cursor pixmap
 * - add an element to the ToolBar_Buttons enum
 * - adjust function loadPixmaps
 * - adjust function slotCheckToolBar
 *
 * @short The toolbar that is different for each type of diagram.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class WorkToolBar : public KToolBar
{
    Q_OBJECT
    Q_ENUMS(ToolBar_Buttons)
public:

    explicit WorkToolBar(QMainWindow *parentWindow);
    ~WorkToolBar();

    void setOldTool();
    void setDefaultTool();

    void setupActions();

    /**
     * Enumeration of all available toolbar buttons.
     */
    enum ToolBar_Buttons {
        tbb_Undefined = -1,
        tbb_Arrow,
        tbb_Generalization,
        tbb_Aggregation,
        tbb_Dependency,
        tbb_Association,
        tbb_Containment,
        tbb_Coll_Mesg_Sync,
        tbb_Coll_Mesg_Async,
        tbb_Seq_Message_Creation,
        tbb_Seq_Message_Destroy,
        tbb_Seq_Message_Synchronous,
        tbb_Seq_Message_Asynchronous,
        tbb_Seq_Message_Found,
        tbb_Seq_Message_Lost,
        tbb_Seq_Combined_Fragment,
        tbb_Seq_Precondition,
        tbb_Composition,
        tbb_Relationship,
        tbb_UniAssociation,
        tbb_State_Transition,
        tbb_Activity_Transition,
        tbb_Send_Signal,
        tbb_Accept_Signal,
        tbb_Accept_Time_Event,
        tbb_Anchor, //keep anchor as last association until code uses better algorithm for testing
        tbb_Note,
        tbb_Box,
        tbb_Text,
        tbb_Actor,
        tbb_UseCase,
        tbb_Class,
        tbb_Interface,
        tbb_Interface_Provider,
        tbb_Interface_Requirement,
        tbb_Datatype,
        tbb_Enum,
        tbb_Entity,
        tbb_Package,
        tbb_Component,
        tbb_Node,
        tbb_Artifact,
        tbb_Object,
        tbb_Initial_State,
        tbb_State,
        tbb_Region,
        tbb_End_State,
        tbb_Initial_Activity,
        tbb_Activity,
        tbb_End_Activity,
        tbb_Final_Activity,
        tbb_Pin,
        tbb_Port,
        tbb_Branch,
        tbb_Fork,
        tbb_DeepHistory,
        tbb_ShallowHistory,
        tbb_StateFork,
        tbb_StateJoin,
        tbb_Junction,
        tbb_Choice,
        tbb_Andline,
        tbb_Exception,
        tbb_Object_Node,
        tbb_PrePostCondition,
        tbb_Category,
        tbb_Category2Parent,
        tbb_Child2Category,
        tbb_Instance,
        tbb_SubSystem
    };

private:

    typedef QMap<Uml::DiagramType::Enum, ToolBar_Buttons> OldToolMap;
    typedef QMap<ToolBar_Buttons, QCursor> CursorMap;
    typedef QMap<ToolBar_Buttons, QAction*> ActionsMap;

    ToolBar_Buttons          m_CurrentButtonID;
    OldToolMap               m_map;
    Uml::DiagramType::Enum   m_Type;
    CursorMap                m_cursors;
    ActionsMap               m_actions;

    void loadPixmaps();

    QCursor currentCursor();
    QCursor defaultCursor();

    QAction* insertHotBtn(ToolBar_Buttons tbb);

    void insertBasicAssociations();

    Q_SIGNAL void sigButtonChanged(int);

public:
    Q_SLOT void slotCheckToolBar(Uml::DiagramType::Enum dt);
    Q_SLOT void buttonChanged(int b);
    Q_SLOT void slotResetToolBar();

    /**
      * These slots are triggered by the buttons. They call buttonChanged with
      * the button id
      */
    Q_SLOT void slotArrow();
    Q_SLOT void slotGeneralization();
    Q_SLOT void slotAggregation();
    Q_SLOT void slotDependency();
    Q_SLOT void slotAssociation();
    Q_SLOT void slotContainment();
    Q_SLOT void slotColl_Mesg_Sync();
    Q_SLOT void slotColl_Mesg_Async();
    Q_SLOT void slotSeq_Message_Creation();
    Q_SLOT void slotSeq_Message_Destroy();
    Q_SLOT void slotSeq_Message_Synchronous();
    Q_SLOT void slotSeq_Message_Asynchronous();
    Q_SLOT void slotSeq_Message_Found();
    Q_SLOT void slotSeq_Message_Lost();
    Q_SLOT void slotSeq_Combined_Fragment();
    Q_SLOT void slotSeq_Precondition();
    Q_SLOT void slotComposition();
    Q_SLOT void slotRelationship();
    Q_SLOT void slotUniAssociation();
    Q_SLOT void slotState_Transition();
    Q_SLOT void slotActivity_Transition();
    Q_SLOT void slotAnchor(); // keep anchor as last association until code uses better algorithm for testing
    Q_SLOT void slotNote();
    Q_SLOT void slotBox();
    Q_SLOT void slotText();
    Q_SLOT void slotActor();
    Q_SLOT void slotUseCase();
    Q_SLOT void slotClass();
    Q_SLOT void slotInterface();
    Q_SLOT void slotInterfaceProvider();
    Q_SLOT void slotInterfaceRequired();
    Q_SLOT void slotDatatype();
    Q_SLOT void slotEnum();
    Q_SLOT void slotEntity();
    Q_SLOT void slotPackage();
    Q_SLOT void slotComponent();
    Q_SLOT void slotNode();
    Q_SLOT void slotArtifact();
    Q_SLOT void slotObject();
    Q_SLOT void slotRegion();
    Q_SLOT void slotInitial_State();
    Q_SLOT void slotState();
    Q_SLOT void slotEnd_State();
    Q_SLOT void slotInitial_Activity();
    Q_SLOT void slotActivity();
    Q_SLOT void slotEnd_Activity();
    Q_SLOT void slotFinal_Activity();
    Q_SLOT void slotBranch();
    Q_SLOT void slotSend_Signal();
    Q_SLOT void slotAccept_Signal();
    Q_SLOT void slotAccept_Time_Event();
    Q_SLOT void slotFork();
    Q_SLOT void slotDeepHistory();
    Q_SLOT void slotShallowHistory();
    Q_SLOT void slotStateJoin();
    Q_SLOT void slotStateFork();
    Q_SLOT void slotJunction();
    Q_SLOT void slotChoice();
    Q_SLOT void slotAndline();
    Q_SLOT void slotException();
    Q_SLOT void slotPrePostCondition();
    Q_SLOT void slotPin();
    Q_SLOT void slotPort();
    Q_SLOT void slotObject_Node();
    Q_SLOT void slotCategory();
    Q_SLOT void slotCategory2Parent();
    Q_SLOT void slotChild2Category();
    Q_SLOT void slotInstance();
    Q_SLOT void slotSubsystem();
};

#endif
