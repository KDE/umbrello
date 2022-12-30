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

signals:
    void sigButtonChanged(int);

public slots:
    void slotCheckToolBar(Uml::DiagramType::Enum dt);
    void buttonChanged(int b);
    void slotResetToolBar();

    /**
      * These slots are triggered by the buttons. They call buttonChanged with
      * the button id
      */
    void slotArrow();
    void slotGeneralization();
    void slotAggregation();
    void slotDependency();
    void slotAssociation();
    void slotContainment();
    void slotColl_Mesg_Sync();
    void slotColl_Mesg_Async();
    void slotSeq_Message_Creation();
    void slotSeq_Message_Destroy();
    void slotSeq_Message_Synchronous();
    void slotSeq_Message_Asynchronous();
    void slotSeq_Message_Found();
    void slotSeq_Message_Lost();
    void slotSeq_Combined_Fragment();
    void slotSeq_Precondition();
    void slotComposition();
    void slotRelationship();
    void slotUniAssociation();
    void slotState_Transition();
    void slotActivity_Transition();
    void slotAnchor(); // keep anchor as last association until code uses better algorithm for testing
    void slotNote();
    void slotBox();
    void slotText();
    void slotActor();
    void slotUseCase();
    void slotClass();
    void slotInterface();
    void slotInterfaceProvider();
    void slotInterfaceRequired();
    void slotDatatype();
    void slotEnum();
    void slotEntity();
    void slotPackage();
    void slotComponent();
    void slotNode();
    void slotArtifact();
    void slotObject();
    void slotRegion();
    void slotInitial_State();
    void slotState();
    void slotEnd_State();
    void slotInitial_Activity();
    void slotActivity();
    void slotEnd_Activity();
    void slotFinal_Activity();
    void slotBranch();
    void slotSend_Signal();
    void slotAccept_Signal();
    void slotAccept_Time_Event();
    void slotFork();
    void slotDeepHistory();
    void slotShallowHistory();
    void slotStateJoin();
    void slotStateFork();
    void slotJunction();
    void slotChoice();
    void slotAndline();
    void slotException();
    void slotPrePostCondition();
    void slotPin();
    void slotPort();
    void slotObject_Node();
    void slotCategory();
    void slotCategory2Parent();
    void slotChild2Category();
    void slotInstance();
    void slotSubsystem();
};

#endif
