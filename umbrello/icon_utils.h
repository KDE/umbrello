/*
    SPDX-FileCopyrightText: 2008 Andreas Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef ICON_UTILS_H
#define ICON_UTILS_H

// app includes
#include "basictypes.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kicon.h>
#endif

// qt includes
#include <QCursor>
#if QT_VERSION >= 0x050000
#include <QIcon>
#endif
#include <QPixmap>

/**
 * This class is a gateway to KIconLoader for for handling all the
 * icons used in umbrello. Identification is done with an enum and
 * then a translation to the icon name.
 * @author  Andreas Fischer
 */
namespace Icon_Utils {

    enum IconType
    {
        it_Home = 0,
        it_Arrow,
        it_File_Open,
        it_Folder,
        it_Folder_Cyan,
        it_Folder_Cyan_Open,
        it_Folder_Green,
        it_Folder_Green_Open,
        it_Folder_Grey,
        it_Folder_Grey_Open,
        it_Folder_Red,
        it_Folder_Red_Open,
        it_Folder_Violet,
        it_Folder_Violet_Open,
        it_Folder_Orange,
        it_Folder_Orange_Open,
        it_Properties_General,
        it_Properties_Color,
        it_Properties_Display,
        it_Properties_Attributes,
        it_Properties_Operations,
        it_Properties_Templates,
        it_Properties_EnumLiterals,
        it_Properties_EntityAttributes,
        it_Properties_EntityConstraints,
        it_Properties_Contents,
        it_Properties_Associations,
        it_Properties_Font,
        it_Properties_Activities,
        it_Properties_Roles,
        it_Properties_UserInterface,
        it_Properties_Class,
        it_Properties_CodeImport,
        it_Properties_CodeGeneration,
        it_Properties_CodeViewer,
        it_Properties_Columns,
        it_Diagram, //change to have different one for each type of diagram
        it_Class,
        it_Object,
        it_Template,
        it_Template_Class,
        it_Template_Interface,
        it_Package,
        it_Subsystem,
        it_Component,
        it_Port,
        it_Node,
        it_Artifact,
        it_Interface,
        it_Interface_Provider,
        it_Interface_Requirement,
        it_Datatype,
        it_Enum,
        it_Entity,
        it_Actor,
        it_UseCase,
        it_Generalisation,
        it_Association,
        it_Uniassociation,
        it_Text,
        it_Note,
        it_Box,
        it_Anchor,
        it_Containment,
        it_Public_Method,
        it_Private_Method,
        it_Protected_Method,
        it_Implementation_Method,
        it_Public_Attribute,
        it_Private_Attribute,
        it_Protected_Attribute,
        it_Implementation_Attribute,
        it_Unique_Constraint,
        it_PrimaryKey_Constraint,
        it_ForeignKey_Constraint,
        it_Check_Constraint,
        it_Diagram_Activity,
        it_Diagram_Class,
        it_Diagram_Collaboration,
        it_Diagram_Component,
        it_Diagram_Deployment,
        it_Diagram_EntityRelationship,
        it_Diagram_Sequence,
        it_Diagram_State,
        it_Diagram_Usecase,
        it_Diagram_Object,
        it_New,
        it_Delete,
        it_Add_Point,
        it_Delete_Point,
        it_Rename,
        it_Cut,
        it_Copy,
        it_Paste,
        it_Undo,
        it_Redo,
        it_UndoView,
        it_Go_Next,
        it_Go_Previous,
        it_Properties,
        it_Show,
        it_Refactor,
        it_View_Code,
        it_Tab_New,
        it_Tab_Close,
        it_Change_Font,
        it_Arrow_Up,
        it_Arrow_Down,
        it_Clear,
        it_Operation_New,
        it_Operation_Public_New,
        it_Attribute_New,
        it_Template_New,
        it_Literal_New,
        it_Entity_Attribute_New,
        it_Parameter_New,
        it_Color_Line,
        it_Color_Fill,
        it_Import_File,
        it_Import_Project,
        it_Import_Files,
        it_Export_Files,
        it_Export_Picture,
        it_InitialState,
        it_EndState,
        it_Branch,
        it_Entity_Attribute,
        it_Constraint_PrimaryKey,
        it_Constraint_ForeignKey,
        it_Constraint_Check,
        it_Constraint_Unique,
        it_Enum_Literal,
        it_State_Activity,
        it_Message_Creation,
        it_Message_Destroy,
        it_Message_Sync,
        it_Message_Async,
        it_Message_Found,
        it_Message_Lost,
        it_Combined_Fragment,
        it_Precondition,
        it_Dependency,
        it_Aggregation,
        it_Relationship,
        it_Directional_Association,
        it_Implements,
        it_Composition,
        it_Region,
        it_Send_Signal,
        it_Accept_Signal,
        it_Accept_TimeEvent,
        it_Fork_Join,
        it_History_Deep,
        it_History_Shallow,
        it_Join,
        it_Fork_State,
        it_Junction,
        it_Choice_Round,
        it_Choice_Rhomb,
        it_And_Line,
        it_State_Transition,
        it_Activity_Transition,
        it_Activity,
        it_State,
        it_Activity_End,
        it_Activity_Final,
        it_Pin,
        it_Activity_Initial,
        it_Message_Synchronous,
        it_Message_Asynchronous,
        it_Exception,
        it_Object_Node,
        it_Condition_PrePost,
        it_Category,
        it_Category_Parent,
        it_Category_Child,
        it_Zoom_Slider,
        it_Zoom_100,
        it_Align_Right,
        it_Align_Left,
        it_Align_Top,
        it_Align_Bottom,
        it_Align_VerticalMiddle,
        it_Align_HorizontalMiddle,
        it_Align_VerticalDistribute,
        it_Align_HorizontalDistribute,
        it_Code_Gen_Wizard,
        it_Properties_AutoLayout,
        it_Document_Edit,
        it_ClassOrPackage,
        it_Instance,
        it_Remove,
        it_Duplicate,
        N_ICONTYPES    // must remain last
    };

    QPixmap SmallIcon(IconType type);
    QPixmap BarIcon(IconType type);
    QPixmap MainBarIcon(IconType type);
    QPixmap UserIcon(IconType type);

    QPixmap DesktopIcon(IconType type);

    QCursor Cursor(IconType type);

    QString toString(IconType type);

    QPixmap iconSet(Uml::DiagramType::Enum dt);
    QPixmap smallIcon(Uml::DiagramType::Enum dt);

}  // namespace

#endif  // ICONCONTAINER_H
