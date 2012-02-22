/*
    Copyright 2008  Andreas Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy 
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "icon_utils.h"

#include "basictypes.h"
#include "debug_utils.h"

#include <kiconloader.h>
#include <kicon.h>


namespace Icon_Utils {

/**
 * Returns the pixmap for the given type as small icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap SmallIcon(IconType type)
{
    return SmallIcon(toString(type));
}

/**
 * Returns the pixmap for the given type as bar icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap BarIcon(IconType type)
{
    return BarIcon(toString(type));
}

/**
 * Returns the pixmap for the given type as main bar icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap MainBarIcon(IconType type)
{
    return MainBarIcon(toString(type));
}

/**
 * Returns the pixmap for the given type as user icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap UserIcon(IconType type)
{
    return UserIcon(toString(type));
}

/**
 * Returns the pixmap for the given type as desktop icon.
 * @param type   the identification of the icon
 * @return       the wanted icon
 */
KIcon DesktopIcon(IconType type)
{
    return KIcon(DesktopIcon(toString(type)));
}

/**
 * Returns the pixmap for the given type as user icon.
 * This is used in worktoolbar to create cursors.
 * @param type   the identification of the icon
 * @return       the wanted cursor
 */
QCursor Cursor(IconType type)
{
    return QCursor(UserIcon("cursor-" + toString(type)), 9, 9);
}

/**
 * Return the icon corresponding to the given Diagram_Type.
 * @param dt   the diagram type
 * @return     the wanted icon
 */
KIcon iconSet(Uml::DiagramType dt)
{
    switch (dt) {
        case Uml::DiagramType::UseCase:
            return DesktopIcon(it_Diagram_Usecase);
        case Uml::DiagramType::Collaboration:
            return DesktopIcon(it_Diagram_Collaboration);
        case Uml::DiagramType::Class:
            return DesktopIcon(it_Diagram_Class);
        case Uml::DiagramType::Sequence:
            return DesktopIcon(it_Diagram_Sequence);
        case Uml::DiagramType::State:
            return DesktopIcon(it_Diagram_State);
        case Uml::DiagramType::Activity:
            return DesktopIcon(it_Diagram_Activity);
        case Uml::DiagramType::Component:
            return DesktopIcon(it_Diagram_Component);
        case Uml::DiagramType::Deployment:
            return DesktopIcon(it_Diagram_Deployment);
        case Uml::DiagramType::EntityRelationship:
            return DesktopIcon(it_Diagram_EntityRelationship);
        default:
            uDebug() << "Widget_Utils::iconSet: unknown diagram type " << dt.toString();
            return KIcon();
    }
}

/**
 * Conversion from icon type to its string name.
 * @param type   the identification of the icon
 * @return       the string representation of the type
 */
QString toString(IconType type)
{
    switch (type) {
        case it_Home:
            return "user-home";
        case it_Arrow:
            return "arrow";
        case it_Folder:
            return "folder-new";
        case it_Folder_Cyan:
            return "folder";
        case it_Folder_Cyan_Open:
            return "folder-open";
        case it_Folder_Green:
            return "folder-green";
        case it_Folder_Green_Open:
            return "folder-green";   //FIXME was folder_green_open
        case it_Folder_Orange:
            return "folder-orange";
        case it_Folder_Orange_Open:
            return "folder-orange";  //FIXME was folder_orange_open
        case it_Folder_Grey:
            return "folder-grey";
        case it_Folder_Grey_Open:
            return "folder-grey";    //FIXME was folder_grey_open
        case it_Folder_Red:
            return "folder-red";
        case it_Folder_Red_Open:
            return "folder-red";     //FIXME was folder_red_open
        case it_Folder_Violet:
            return "folder-violet";
        case it_Folder_Violet_Open:
            return "folder-violet";  //FIXME was folder_violet_open
        case it_Properties_General:
            return "preferences-other";
        case it_Properties_Color:
            return "preferences-desktop-color";
        case it_Properties_Display:
            return "preferences-desktop-theme";
        case it_Properties_Attributes:
            return "preferences-other";
        case it_Properties_Operations:
            return "preferences-other";
        case it_Properties_Templates:
            return "preferences-other";
        case it_Properties_EnumLiterals:
            return "preferences-other";
        case it_Properties_EntityAttributes:
            return "preferences-other";
        case it_Properties_EntityConstraints:
            return "preferences-other";
        case it_Properties_Contents:
            return "preferences-other";
        case it_Properties_Associations:
            return "preferences-other";
        case it_Properties_Font:
            return "preferences-desktop-font";
        case it_Properties_Activities:
            return "text-x-generic";
        case it_Properties_Roles:
            return "preferences-other";
        case it_Properties_UserInterface:
            return "preferences-desktop-theme";
        case it_Properties_Class:
            return "document-properties";
        case it_Properties_CodeImport:
            return "text-x-generic";
        case it_Properties_CodeGeneration:
            return "text-x-generic";
        case it_Properties_CodeViewer:
            return "text-x-generic";
        case it_Properties_Columns:
            return "preferences-other";
        case it_Diagram_Activity:
            return "umbrello_diagram_activity";
        case it_Diagram_Class:
            return "umbrello_diagram_class";
        case it_Diagram_Component:
            return "umbrello_diagram_component";
        case it_Diagram_State:
            return "umbrello_diagram_state";
        case it_Diagram_Sequence:
            return "umbrello_diagram_sequence";
        case it_Diagram_Deployment:
            return "umbrello_diagram_deployment";
        case it_Diagram_EntityRelationship:
            return "umbrello_diagram_deployment";
        case it_Diagram_Usecase:
            return "umbrello_diagram_usecase";
        case it_Diagram_Collaboration:
            return "umbrello_diagram_collaboration";
        case it_Diagram:
            return "CVnamespace";
        case it_Class:
            return "class";
        case it_Object:
            return "object";
        case it_Template:
            return "template";
        case it_Template_Class:
            return "format-justify-fill";
        case it_Template_Interface:
            return "text-x-generic-template";
        case it_Package:
            return "package";
        case it_Subsystem:
            return "subsystem";
        case it_Component:
            return "component";
        case it_Node:
            return "node";
        case it_Artifact:
            return "artifact";
        case it_Interface:
            return "interface";
        case it_Datatype:
            return "datatype";
        case it_Enum:
            return "enum";
        case it_Entity:
            return "entity";
        case it_Actor:
            return "actor";
        case it_UseCase:
            return "usecase";
        case it_Public_Method:
            return "CVpublic_meth";
        case it_Private_Method:
            return "CVprivate_meth";
        case it_Protected_Method:
            return "CVprotected_meth";
        case it_Implementation_Method:
            return "CVimplementation_meth";
        case it_Public_Attribute:
            return "CVpublic_var";
        case it_Private_Attribute:
            return "CVprivate_var";
        case it_Protected_Attribute:
            return "CVprotected_var";
        case it_Implementation_Attribute:
            return "CVimplementation_var";
        case it_PrimaryKey_Constraint:
            return "primarykey_constraint";
        case it_ForeignKey_Constraint:
            return "foreignkey_constraint";
        case it_Unique_Constraint:
            return "unique_constraint";
        case it_Check_Constraint:
            return "check_constraint";
        case it_Generalisation:
            return "generalisation";
        case it_Association:
            return "association";
        case it_Uniassociation:
            return "uniassociation";
        case it_Text:
            return "text";
        case it_Note:
            return "note";
        case it_Box:
            return "box";
        case it_Anchor:
            return "anchor";
        case it_Containment:
            return "containment";
        case it_InitialState:
            return "initial_state";
        case it_EndState:
            return "end_state";
        case it_Branch:
            return "branch";
        case it_New:
            return "document-new";
        case it_Delete:
            return "edit-delete";
        case it_Rename:
            return "edit-rename";
        case it_Cut:
            return "edit-cut";
        case it_Copy:
            return "edit-copy";
        case it_Paste:
            return "edit-paste";
        case it_Undo:
            return "edit-undo";
        case it_Redo:
            return "edit-redo";
        case it_UndoView:
            return "document-save";
        case it_Go_Next:
            return "go-next";
        case it_Go_Previous:
            return "go-previous";
        case it_Properties:
            return "document-properties";
        case it_Show:
            return "document-preview";
        case it_Refactor:
            return "refactor";
        case it_View_Code:
            return "text-x-generic";
        case it_Tab_New:
            return "tab-new";
        case it_Tab_Close:
            return "tab-close";
        case it_Change_Font:
            return "preferences-desktop-font";
        case it_Arrow_Up:
            return "arrow-up";
        case it_Arrow_Down:
            return "arrow-down";
        case it_Clear:
            return "edit-clear";
        case it_Operation_New:
            return "document-new";
        case it_Operation_Public_New:
            return "CVpublic_meth";
        case it_Attribute_New:
            return "CVpublic_var";
        case it_Template_New:
            return "text-x-generic-template";
        case it_Literal_New:
            return "text-x-generic";
        case it_Entity_Attribute_New:
            return "text-x-generic";
        case it_Parameter_New:
            return "text-x-generic";
        case it_Color_Line:
            return "draw-brush";
        case it_Color_Fill:
            return "fill-color";
        case it_Import_Files:
            return "document-import";
        case it_Export_Files:
            return "document-export";
        case it_Export_Picture:
            return "image-x-generic";
        case it_Entity_Attribute:
            return "text-x-generic";
        case it_Constraint_PrimaryKey:
            return "primarykey_constraint";
        case it_Constraint_ForeignKey:
            return "foreignkey_constraint";
        case it_Constraint_Check:
            return "check_constraint";
        case it_Constraint_Unique:
            return "unique_constraint";
        case it_Enum_Literal:
            return "text-x-generic";
        case it_State_Activity:
            return "text-x-generic";
        case it_Message_Sync:
            return "umbr-message-synchronous";
        case it_Message_Async:
            return "umbr-message-asynchronous";
        case it_Message_Found:
            return "umbr-message-found";
        case it_Message_Lost:
            return "umbr-message-lost";
        case it_Combined_Fragment:
            return "combined_fragment";
        case it_Precondition:
            return "precondition";
        case it_Dependency:
            return "dependency";
        case it_Aggregation:
            return "aggregation";
        case it_Relationship:
            return "relationship";
        case it_Directional_Association:
            return "uniassociation";
        case it_Implements:
            return "generalisation";
        case it_Composition:
            return "composition";
        case it_Region:
            return "region";
        case it_Send_Signal:
            return "send_signal";
        case it_Accept_Signal:
            return "accept_signal";
        case it_Accept_TimeEvent:
            return "accept_time_event";
        case it_Fork_Join:
            return "activity-fork";
        case it_History_Deep:
            return "deep-history";
        case it_History_Shallow:
            return "shallow-history";
        case it_Join:
            return "join";
        case it_Fork_State:
            return "state-fork";
        case it_Junction:
            return "junction";
        case it_Choice_Round:
            return "choice-round";
        case it_Choice_Rhomb:
            return "choice-rhomb";
        case it_And_Line:
            return "andline";
        case it_State_Transition:
            return "uniassociation";
        case it_Activity_Transition:
            return "uniassociation";
        case it_Activity:
            return "usecase";
        case it_State:
            return "usecase";
        case it_Activity_End:
            return "end_state";
        case it_Activity_Final:
            return "final_activity";
        case it_Pin:
            return "pin";
        case it_Activity_Initial:
            return "initial_state";
        case it_Message:
            return "umbr-message-asynchronous";
        case it_Exception:
            return "exception";
        case it_Object_Node:
            return "object_node";
        case it_Condition_PrePost:
            return "PrePostCondition";
        case it_Category:
            return "category";
        case it_Category_Parent:
            return "category2parent";
        case it_Category_Child:
            return "child2category";
        case it_Zoom_Slider:
            return "zoom-original";
        case it_Zoom_100:
            return "zoom-original";
        case it_Align_Right:
            return "align-horizontal-right";
        case it_Align_Left:
            return "align-horizontal-left";
        case it_Align_Top:
            return "align-vertical-top";
        case it_Align_Bottom:
            return "align-vertical-bottom";
        case it_Align_VerticalMiddle:
            return "align-vertical-center";
        case it_Align_HorizontalMiddle:
            return "align-horizontal-center";
        case it_Align_VerticalDistribute:
            return "distribute-vertical";
        case it_Align_HorizontalDistribute:
            return "distribute-horizontal";
        case it_Code_Gen_Wizard:
            return "hi64-app-umbrello.png";
        default:
            return QString();
    }
}

}  // namespace
