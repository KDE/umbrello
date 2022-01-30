/*
    SPDX-FileCopyrightText: 2008 Andreas Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2009-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "icon_utils.h"

#include "basictypes.h"
#define DBG_SRC QLatin1String("Icon_Utils")
#include "debug_utils.h"
#include "optionstate.h"
#include "uml.h"  // Only needed for logDebug

#include <kiconloader.h>

#include <QFile>

DEBUG_REGISTER(Icon_Utils)

namespace Icon_Utils {

#define ICON_PREFIX QLatin1String(":/pics/")

/**
 * Returns the pixmap for the given type as small icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap SmallIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QPixmap(ICON_PREFIX + icon);
    else
        return SmallIcon(icon);
}

/**
 * Returns the pixmap for the given type as bar icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap BarIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QPixmap(ICON_PREFIX + icon);
    else
        return BarIcon(icon);
}

/**
 * Returns the pixmap for the given type as main bar icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap MainBarIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QPixmap(ICON_PREFIX + icon);
    else
        return MainBarIcon(icon);
}

/**
 * Returns the pixmap for the given type as user icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap UserIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QPixmap(ICON_PREFIX + icon);
    else
        return UserIcon(icon);
}

/**
 * Returns the pixmap for the given type as desktop icon.
 * @param type   the identification of the icon
 * @return       the wanted icon
 */
QPixmap DesktopIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QPixmap(ICON_PREFIX + icon);
    else
        return DesktopIcon(icon);
}

/**
 * Returns the pixmap for the given type as user icon.
 * This is used in worktoolbar to create cursors.
 * @param type   the identification of the icon
 * @return       the wanted cursor
 */
QCursor Cursor(IconType type)
{
    // TODO: generate from a 32x32 cursor template and place requested icon into
    QString icon = QLatin1String("cursor-") + toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QLatin1String(".png")))
        return QCursor(QPixmap(ICON_PREFIX + icon), 9, 9);
    else
        return QCursor(UserIcon(icon), 9, 9);
}

/**
 * Return the icon corresponding to the given Diagram_Type.
 * @param dt   the diagram type
 * @return     the wanted icon
 */
QPixmap iconSet(Uml::DiagramType::Enum dt)
{
    switch (dt) {
        case Uml::DiagramType::UseCase:
            return DesktopIcon(it_Diagram_Usecase);
        case Uml::DiagramType::Collaboration:
            return DesktopIcon(it_Diagram_Collaboration);
        case Uml::DiagramType::Class:
            return DesktopIcon(it_Diagram_Class);
        case Uml::DiagramType::Object:
            return DesktopIcon(it_Diagram_Object);
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
            logDebug1("Icon_Utils::iconSet: unknown diagram type %1",
                      Uml::DiagramType::toString(dt));
            return QPixmap();
    }
}

/**
 * Return the icon corresponding to the given Diagram_Type.
 * @param dt   the diagram type
 * @return     the wanted icon
 */
QPixmap smallIcon(Uml::DiagramType::Enum dt)
{
    switch (dt) {
        case Uml::DiagramType::UseCase:
            return SmallIcon(it_Diagram_Usecase);
        case Uml::DiagramType::Collaboration:
            return SmallIcon(it_Diagram_Collaboration);
        case Uml::DiagramType::Class:
            return SmallIcon(it_Diagram_Class);
        case Uml::DiagramType::Object:
            return SmallIcon(it_Diagram_Object);
        case Uml::DiagramType::Sequence:
            return SmallIcon(it_Diagram_Sequence);
        case Uml::DiagramType::State:
            return SmallIcon(it_Diagram_State);
        case Uml::DiagramType::Activity:
            return SmallIcon(it_Diagram_Activity);
        case Uml::DiagramType::Component:
            return SmallIcon(it_Diagram_Component);
        case Uml::DiagramType::Deployment:
            return SmallIcon(it_Diagram_Deployment);
        case Uml::DiagramType::EntityRelationship:
            return SmallIcon(it_Diagram_EntityRelationship);
        default:
            logDebug1("Icon_Utils::smallIcon: unknown diagram type %1",
                      Uml::DiagramType::toString(dt));
            return QPixmap();
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
    case it_Accept_Signal: return QLatin1String("accept_signal");
    case it_Accept_TimeEvent: return QLatin1String("accept_time_event");
    case it_Activity: return QLatin1String("activity");
    case it_Activity_End: return QLatin1String("end_state");
    case it_Activity_Final: return QLatin1String("final_activity");
    case it_Activity_Initial: return QLatin1String("initial_state");
    case it_Activity_Transition: return QLatin1String("uniassociation");
    case it_Actor: return QLatin1String("actor");
    case it_Add_Point: return QLatin1String("format-add-node");
    case it_Aggregation: return QLatin1String("aggregation");
    case it_Align_Bottom: return QLatin1String("align-vertical-bottom");
    case it_Align_HorizontalDistribute: return QLatin1String("distribute-horizontal");
    case it_Align_HorizontalMiddle: return QLatin1String("align-horizontal-center");
    case it_Align_Left: return QLatin1String("align-horizontal-left");
    case it_Align_Right: return QLatin1String("align-horizontal-right");
    case it_Align_Top: return QLatin1String("align-vertical-top");
    case it_Align_VerticalDistribute: return QLatin1String("distribute-vertical");
    case it_Align_VerticalMiddle: return QLatin1String("align-vertical-center");
    case it_Anchor: return QLatin1String("anchor");
    case it_And_Line: return QLatin1String("andline");
    case it_Arrow: return QLatin1String("arrow");
    case it_Arrow_Down: return QLatin1String("arrow-down");
    case it_Arrow_Up: return QLatin1String("arrow-up");
    case it_Artifact: return QLatin1String("artifact");
    case it_Association: return QLatin1String("association");
    case it_Attribute_New: return QLatin1String("CVpublic_var");
    case it_Box: return QLatin1String("box");
    case it_Branch: return QLatin1String("branch");
    case it_Category: return QLatin1String("category");
    case it_Category_Child: return QLatin1String("child2category");
    case it_Category_Parent: return QLatin1String("category2parent");
    case it_Change_Font: return QLatin1String("preferences-desktop-font");
    case it_Check_Constraint: return QLatin1String("check_constraint");
    case it_Choice_Rhomb: return QLatin1String("choice-rhomb");
    case it_Choice_Round: return QLatin1String("choice-round");
    case it_Class: return QLatin1String("class");
    case it_ClassOrPackage: return QLatin1String("class-or-package");
    case it_Clear: return QLatin1String("edit-clear");
    case it_Code_Gen_Wizard: return QLatin1String("umbrello");
    case it_Color_Fill: return QLatin1String("fill-color");
    case it_Color_Line: return QLatin1String("draw-brush");
    case it_Combined_Fragment: return QLatin1String("combined_fragment");
    case it_Component: return Settings::optionState().generalState.uml2 ? QLatin1String("component") : QLatin1String("component1");
    case it_Composition: return QLatin1String("composition");
    case it_Condition_PrePost: return QLatin1String("PrePostCondition");
    case it_Constraint_Check: return QLatin1String("check_constraint");
    case it_Constraint_ForeignKey: return QLatin1String("foreignkey_constraint");
    case it_Constraint_PrimaryKey: return QLatin1String("primarykey_constraint");
    case it_Constraint_Unique: return QLatin1String("unique_constraint");
    case it_Containment: return QLatin1String("containment");
    case it_Copy: return QLatin1String("edit-copy");
    case it_Cut: return QLatin1String("edit-cut");
    case it_Datatype: return QLatin1String("datatype");
    case it_Delete: return QLatin1String("edit-delete");
    case it_Delete_Point: return QLatin1String("format-remove-node");
    case it_Dependency: return QLatin1String("dependency");
    case it_Diagram: return QLatin1String("CVnamespace");
    case it_Diagram_Activity: return QLatin1String("umbrello_diagram_activity");
    case it_Diagram_Class: return QLatin1String("umbrello_diagram_class");
    case it_Diagram_Collaboration: return QLatin1String("umbrello_diagram_collaboration");
    case it_Diagram_Component: return QLatin1String("umbrello_diagram_component");
    case it_Diagram_Deployment: return QLatin1String("umbrello_diagram_deployment");
    case it_Diagram_EntityRelationship: return QLatin1String("umbrello_diagram_deployment");
    case it_Diagram_Object: return QLatin1String("umbrello_diagram_object");
    case it_Diagram_Sequence: return QLatin1String("umbrello_diagram_sequence");
    case it_Diagram_State: return QLatin1String("umbrello_diagram_state");
    case it_Diagram_Usecase: return QLatin1String("umbrello_diagram_usecase");
    case it_Directional_Association: return QLatin1String("uniassociation");
    case it_Document_Edit: return QLatin1String("document-edit");
    case it_Duplicate: return QLatin1String("duplicate");
    case it_EndState: return QLatin1String("end_state");
    case it_Entity: return QLatin1String("entity");
    case it_Entity_Attribute: return QLatin1String("text-x-generic");
    case it_Entity_Attribute_New: return QLatin1String("text-x-generic");
    case it_Enum: return QLatin1String("enum");
    case it_Enum_Literal: return QLatin1String("text-x-generic");
    case it_Exception: return QLatin1String("exception");
    case it_Export_Files: return QLatin1String("document-export");
    case it_Export_Picture: return QLatin1String("image-x-generic");
    case it_File_Open: return QLatin1String("document-open");
    case it_Folder: return QLatin1String("folder-new");
    case it_Folder_Cyan: return QLatin1String("folder");
    case it_Folder_Cyan_Open: return QLatin1String("folder-open");
    case it_Folder_Green: return QLatin1String("folder-green");
    case it_Folder_Green_Open: return QLatin1String("folder-green");   //FIXME was folder_green_open
    case it_Folder_Grey: return QLatin1String("folder-grey");
    case it_Folder_Grey_Open: return QLatin1String("folder-grey");    //FIXME was folder_grey_open
    case it_Folder_Orange: return QLatin1String("folder-orange");
    case it_Folder_Orange_Open: return QLatin1String("folder-orange");  //FIXME was folder_orange_open
    case it_Folder_Red: return QLatin1String("folder-red");
    case it_Folder_Red_Open: return QLatin1String("folder-red");     //FIXME was folder_red_open
    case it_Folder_Violet: return QLatin1String("folder-violet");
    case it_Folder_Violet_Open: return QLatin1String("folder-violet");  //FIXME was folder_violet_open
    case it_ForeignKey_Constraint: return QLatin1String("foreignkey_constraint");
    case it_Fork_Join: return QLatin1String("activity-fork");
    case it_Fork_State: return QLatin1String("state-fork");
    case it_Generalisation: return QLatin1String("generalisation");
    case it_Go_Next: return QLatin1String("go-next");
    case it_Go_Previous: return QLatin1String("go-previous");
    case it_History_Deep: return QLatin1String("deep-history");
    case it_History_Shallow: return QLatin1String("shallow-history");
    case it_Home: return QLatin1String("user-home");
    case it_Implementation_Attribute: return QLatin1String("CVimplementation_var");
    case it_Implementation_Method: return QLatin1String("CVimplementation_meth");
    case it_Implements: return QLatin1String("generalisation");
    case it_Import_File: return QLatin1String("document-import");
    case it_Import_Files: return QLatin1String("document-import");
    case it_Import_Project: return QLatin1String("document-import");
    case it_InitialState: return QLatin1String("initial_state");
    case it_Instance: return QLatin1String("instance");
    case it_Interface: return QLatin1String("interface");
    case it_Interface_Requirement: return QLatin1String("interface-requirement");
    case it_Interface_Provider: return QLatin1String("interface-provider");
    case it_Join: return QLatin1String("join");
    case it_Junction: return QLatin1String("junction");
    case it_Literal_New: return QLatin1String("text-x-generic");
    case it_Message_Async: return QLatin1String("umbr-message-asynchronous");
    case it_Message_Asynchronous: return QLatin1String("umbr-coll-message-asynchronous");
    case it_Message_Creation: return QLatin1String("umbr-message-creation");
    case it_Message_Destroy: return QLatin1String("umbr-message-destroy");
    case it_Message_Found: return QLatin1String("umbr-message-found");
    case it_Message_Lost: return QLatin1String("umbr-message-lost");
    case it_Message_Sync: return QLatin1String("umbr-message-synchronous");
    case it_Message_Synchronous: return QLatin1String("umbr-coll-message-synchronous");
    case it_New: return QLatin1String("document-new");
    case it_Node: return QLatin1String("node");
    case it_Note: return QLatin1String("note");
    case it_Object: return QLatin1String("object");
    case it_Object_Node: return QLatin1String("object_node");
    case it_Operation_New: return QLatin1String("document-new");
    case it_Operation_Public_New: return QLatin1String("CVpublic_meth");
    case it_Package: return QLatin1String("package");
    case it_Parameter_New: return QLatin1String("text-x-generic");
    case it_Paste: return QLatin1String("edit-paste");
    case it_Pin: return QLatin1String("pin");
    case it_Port: return QLatin1String("port");
    case it_Precondition: return QLatin1String("precondition");
    case it_PrimaryKey_Constraint: return QLatin1String("primarykey_constraint");
    case it_Private_Attribute: return QLatin1String("CVprivate_var");
    case it_Private_Method: return QLatin1String("CVprivate_meth");
    case it_Properties: return QLatin1String("preferences-system");
    case it_Properties_Activities: return QLatin1String("text-x-generic");
    case it_Properties_Associations: return QLatin1String("preferences-other");
    case it_Properties_Attributes: return QLatin1String("preferences-other");
    case it_Properties_AutoLayout: return QLatin1String("code-class");
    case it_Properties_Class: return QLatin1String("document-properties");
    case it_Properties_CodeGeneration: return QLatin1String("document-export");
    case it_Properties_CodeImport: return QLatin1String("document-import");
    case it_Properties_CodeViewer: return QLatin1String("package_graphics_viewer");
    case it_Properties_Color: return QLatin1String("preferences-desktop-color");
    case it_Properties_Columns: return QLatin1String("preferences-other");
    case it_Properties_Contents: return QLatin1String("preferences-other");
    case it_Properties_Display: return QLatin1String("preferences-desktop-theme");
    case it_Properties_EntityAttributes: return QLatin1String("preferences-other");
    case it_Properties_EntityConstraints: return QLatin1String("preferences-other");
    case it_Properties_EnumLiterals: return QLatin1String("preferences-other");
    case it_Properties_Font: return QLatin1String("preferences-desktop-font");
    case it_Properties_General: return QLatin1String("preferences-other");
    case it_Properties_Operations: return QLatin1String("preferences-other");
    case it_Properties_Roles: return QLatin1String("preferences-other");
    case it_Properties_Templates: return QLatin1String("preferences-other");
    case it_Properties_UserInterface: return QLatin1String("preferences-desktop-theme");
    case it_Protected_Attribute: return QLatin1String("CVprotected_var");
    case it_Protected_Method: return QLatin1String("CVprotected_meth");
    case it_Public_Attribute: return QLatin1String("CVpublic_var");
    case it_Public_Method: return QLatin1String("CVpublic_meth");
    case it_Redo: return QLatin1String("edit-redo");
    case it_Refactor: return QLatin1String("refactor");
    case it_Region: return QLatin1String("region");
    case it_Relationship: return QLatin1String("relationship");
    case it_Remove: return QLatin1String("remove");
    case it_Rename: return QLatin1String("edit-rename");
    case it_Send_Signal: return QLatin1String("send_signal");
    case it_Show: return QLatin1String("document-preview");
    case it_State: return QLatin1String("state");
    case it_State_Activity: return QLatin1String("text-x-generic");
    case it_State_Transition: return QLatin1String("uniassociation");
    case it_Subsystem: return QLatin1String("subsystem");
    case it_Tab_Close: return QLatin1String("tab-close");
    case it_Tab_New: return QLatin1String("tab-new");
    case it_Template: return QLatin1String("template");
    case it_Template_Class: return QLatin1String("format-justify-fill");
    case it_Template_Interface: return QLatin1String("text-x-generic-template");
    case it_Template_New: return QLatin1String("text-x-generic-template");
    case it_Text: return QLatin1String("text");
    case it_Undo: return QLatin1String("edit-undo");
    case it_UndoView: return QLatin1String("document-save");
    case it_Uniassociation: return QLatin1String("uniassociation");
    case it_Unique_Constraint: return QLatin1String("unique_constraint");
    case it_UseCase: return QLatin1String("usecase");
    case it_View_Code: return QLatin1String("text-x-generic");
    case it_Zoom_100: return QLatin1String("zoom-original");
    case it_Zoom_Slider: return QLatin1String("zoom-original");
    default: return QString();
    }
}

}  // namespace
