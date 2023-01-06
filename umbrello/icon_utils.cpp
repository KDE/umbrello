/*
    SPDX-FileCopyrightText: 2008 Andreas Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2009-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "icon_utils.h"

#include "basictypes.h"
#define DBG_SRC QStringLiteral("Icon_Utils")
#include "debug_utils.h"
#include "optionstate.h"
#include "uml.h"  // Only needed for logDebug

#include <kiconloader.h>

// qt includes
#include <QIcon>
#include <QFile>

DEBUG_REGISTER(Icon_Utils)

namespace Icon_Utils {

#define ICON_PREFIX QStringLiteral(":/pics/")

/**
 * Returns the pixmap for the given type as small icon.
 * @param type   the identification of the icon
 * @return       the wanted pixmap
 */
QPixmap SmallIcon(IconType type)
{
    QString icon = toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    QString icon = QStringLiteral("cursor-") + toString(type);
    if (QFile::exists(ICON_PREFIX + icon + QStringLiteral(".png")))
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
    case it_Accept_Signal:                return QStringLiteral("accept_signal");
    case it_Accept_TimeEvent:             return QStringLiteral("accept_time_event");
    case it_Activity:                     return QStringLiteral("activity");
    case it_Activity_End:                 return QStringLiteral("end_state");
    case it_Activity_Final:               return QStringLiteral("final_activity");
    case it_Activity_Initial:             return QStringLiteral("initial_state");
    case it_Activity_Transition:          return QStringLiteral("uniassociation");
    case it_Actor:                        return QStringLiteral("actor");
    case it_Add_Point:                    return QStringLiteral("format-add-node");
    case it_Aggregation:                  return QStringLiteral("aggregation");
    case it_Align_Bottom:                 return QStringLiteral("align-vertical-bottom");
    case it_Align_HorizontalDistribute:   return QStringLiteral("distribute-horizontal");
    case it_Align_HorizontalMiddle:       return QStringLiteral("align-horizontal-center");
    case it_Align_Left:                   return QStringLiteral("align-horizontal-left");
    case it_Align_Right:                  return QStringLiteral("align-horizontal-right");
    case it_Align_Top:                    return QStringLiteral("align-vertical-top");
    case it_Align_VerticalDistribute:     return QStringLiteral("distribute-vertical");
    case it_Align_VerticalMiddle:         return QStringLiteral("align-vertical-center");
    case it_Anchor:                       return QStringLiteral("anchor");
    case it_And_Line:                     return QStringLiteral("andline");
    case it_Arrow:                        return QStringLiteral("arrow");
    case it_Arrow_Down:                   return QStringLiteral("arrow-down");
    case it_Arrow_Up:                     return QStringLiteral("arrow-up");
    case it_Artifact:                     return QStringLiteral("artifact");
    case it_Association:                  return QStringLiteral("association");
    case it_Attribute_New:                return QStringLiteral("CVpublic_var");
    case it_Box:                          return QStringLiteral("box");
    case it_Branch:                       return QStringLiteral("branch");
    case it_Category:                     return QStringLiteral("category");
    case it_Category_Child:               return QStringLiteral("child2category");
    case it_Category_Parent:              return QStringLiteral("category2parent");
    case it_Change_Font:                  return QStringLiteral("preferences-desktop-font");
    case it_Check_Constraint:             return QStringLiteral("check_constraint");
    case it_Choice_Rhomb:                 return QStringLiteral("choice-rhomb");
    case it_Choice_Round:                 return QStringLiteral("choice-round");
    case it_Class:                        return QStringLiteral("class");
    case it_ClassOrPackage:               return QStringLiteral("class-or-package");
    case it_Clear:                        return QStringLiteral("edit-clear");
    case it_Code_Gen_Wizard:              return QStringLiteral("umbrello");
    case it_Color_Fill:                   return QStringLiteral("fill-color");
    case it_Color_Line:                   return QStringLiteral("draw-brush");
    case it_Combined_Fragment:            return QStringLiteral("combined_fragment");
    case it_Component: return Settings::optionState().generalState.uml2 ? QStringLiteral("component") : QStringLiteral("component1");
    case it_Composition:                  return QStringLiteral("composition");
    case it_Condition_PrePost:            return QStringLiteral("PrePostCondition");
    case it_Constraint_Check:             return QStringLiteral("check_constraint");
    case it_Constraint_ForeignKey:        return QStringLiteral("foreignkey_constraint");
    case it_Constraint_PrimaryKey:        return QStringLiteral("primarykey_constraint");
    case it_Constraint_Unique:            return QStringLiteral("unique_constraint");
    case it_Containment:                  return QStringLiteral("containment");
    case it_Copy:                         return QStringLiteral("edit-copy");
    case it_Cut:                          return QStringLiteral("edit-cut");
    case it_Datatype:                     return QStringLiteral("datatype");
    case it_Delete:                       return QStringLiteral("edit-delete");
    case it_Delete_Point:                 return QStringLiteral("format-remove-node");
    case it_Dependency:                   return QStringLiteral("dependency");
    case it_Diagram:                      return QStringLiteral("CVnamespace");
    case it_Diagram_Activity:             return QStringLiteral("umbrello_diagram_activity");
    case it_Diagram_Class:                return QStringLiteral("umbrello_diagram_class");
    case it_Diagram_Collaboration:        return QStringLiteral("umbrello_diagram_collaboration");
    case it_Diagram_Component:            return QStringLiteral("umbrello_diagram_component");
    case it_Diagram_Deployment:           return QStringLiteral("umbrello_diagram_deployment");
    case it_Diagram_EntityRelationship:   return QStringLiteral("umbrello_diagram_deployment");
    case it_Diagram_Object:               return QStringLiteral("umbrello_diagram_object");
    case it_Diagram_Sequence:             return QStringLiteral("umbrello_diagram_sequence");
    case it_Diagram_State:                return QStringLiteral("umbrello_diagram_state");
    case it_Diagram_Usecase:              return QStringLiteral("umbrello_diagram_usecase");
    case it_Directional_Association:      return QStringLiteral("uniassociation");
    case it_Document_Edit:                return QStringLiteral("document-edit");
    case it_Duplicate:                    return QStringLiteral("duplicate");
    case it_EndState:                     return QStringLiteral("end_state");
    case it_Entity:                       return QStringLiteral("entity");
    case it_Entity_Attribute:             return QStringLiteral("text-x-generic");
    case it_Entity_Attribute_New:         return QStringLiteral("text-x-generic");
    case it_Enum:                         return QStringLiteral("enum");
    case it_Enum_Literal:                 return QStringLiteral("text-x-generic");
    case it_Exception:                    return QStringLiteral("exception");
    case it_Export_Files:                 return QStringLiteral("document-export");
    case it_Export_Picture:               return QStringLiteral("image-x-generic");
    case it_File_Open:                    return QStringLiteral("document-open");
    case it_Folder:                       return QStringLiteral("folder-new");
    case it_Folder_Cyan:                  return QStringLiteral("folder");
    case it_Folder_Cyan_Open:             return QStringLiteral("folder-open");
    case it_Folder_Green:                 return QStringLiteral("folder-green");
    case it_Folder_Green_Open:            return QStringLiteral("folder-green");   //FIXME was folder_green_open
    case it_Folder_Grey:                  return QStringLiteral("folder-grey");
    case it_Folder_Grey_Open:             return QStringLiteral("folder-grey");    //FIXME was folder_grey_open
    case it_Folder_Orange:                return QStringLiteral("folder-orange");
    case it_Folder_Orange_Open:           return QStringLiteral("folder-orange");  //FIXME was folder_orange_open
    case it_Folder_Red:                   return QStringLiteral("folder-red");
    case it_Folder_Red_Open:              return QStringLiteral("folder-red");     //FIXME was folder_red_open
    case it_Folder_Violet:                return QStringLiteral("folder-violet");
    case it_Folder_Violet_Open:           return QStringLiteral("folder-violet");  //FIXME was folder_violet_open
    case it_ForeignKey_Constraint:        return QStringLiteral("foreignkey_constraint");
    case it_Fork_Join:                    return QStringLiteral("activity-fork");
    case it_Fork_State:                   return QStringLiteral("state-fork");
    case it_Generalisation:               return QStringLiteral("generalisation");
    case it_Go_Next:                      return QStringLiteral("go-next");
    case it_Go_Previous:                  return QStringLiteral("go-previous");
    case it_History_Deep:                 return QStringLiteral("deep-history");
    case it_History_Shallow:              return QStringLiteral("shallow-history");
    case it_Home:                         return QStringLiteral("user-home");
    case it_Implementation_Attribute:     return QStringLiteral("CVimplementation_var");
    case it_Implementation_Method:        return QStringLiteral("CVimplementation_meth");
    case it_Implements:                   return QStringLiteral("generalisation");
    case it_Import_File:                  return QStringLiteral("document-import");
    case it_Import_Files:                 return QStringLiteral("document-import");
    case it_Import_Project:               return QStringLiteral("document-import");
    case it_InitialState:                 return QStringLiteral("initial_state");
    case it_Instance:                     return QStringLiteral("instance");
    case it_Interface:                    return QStringLiteral("interface");
    case it_Interface_Requirement:        return QStringLiteral("interface-requirement");
    case it_Interface_Provider:           return QStringLiteral("interface-provider");
    case it_Join:                         return QStringLiteral("join");
    case it_Junction:                     return QStringLiteral("junction");
    case it_Literal_New:                  return QStringLiteral("text-x-generic");
    case it_Message_Async:                return QStringLiteral("umbr-message-asynchronous");
    case it_Message_Asynchronous:         return QStringLiteral("umbr-coll-message-asynchronous");
    case it_Message_Creation:             return QStringLiteral("umbr-message-creation");
    case it_Message_Destroy:              return QStringLiteral("umbr-message-destroy");
    case it_Message_Found:                return QStringLiteral("umbr-message-found");
    case it_Message_Lost:                 return QStringLiteral("umbr-message-lost");
    case it_Message_Sync:                 return QStringLiteral("umbr-message-synchronous");
    case it_Message_Synchronous:          return QStringLiteral("umbr-coll-message-synchronous");
    case it_New:                          return QStringLiteral("document-new");
    case it_Node:                         return QStringLiteral("node");
    case it_Note:                         return QStringLiteral("note");
    case it_Object:                       return QStringLiteral("object");
    case it_Object_Node:                  return QStringLiteral("object_node");
    case it_Operation_New:                return QStringLiteral("document-new");
    case it_Operation_Public_New:         return QStringLiteral("CVpublic_meth");
    case it_Package:                      return QStringLiteral("package");
    case it_Parameter_New:                return QStringLiteral("text-x-generic");
    case it_Paste:                        return QStringLiteral("edit-paste");
    case it_Pin:                          return QStringLiteral("pin");
    case it_Port:                         return QStringLiteral("port");
    case it_Precondition:                 return QStringLiteral("precondition");
    case it_PrimaryKey_Constraint:        return QStringLiteral("primarykey_constraint");
    case it_Private_Attribute:            return QStringLiteral("CVprivate_var");
    case it_Private_Method:               return QStringLiteral("CVprivate_meth");
    case it_Properties:                   return QStringLiteral("preferences-system");
    case it_Properties_Activities:        return QStringLiteral("text-x-generic");
    case it_Properties_Associations:      return QStringLiteral("preferences-other");
    case it_Properties_Attributes:        return QStringLiteral("preferences-other");
    case it_Properties_AutoLayout:        return QStringLiteral("code-class");
    case it_Properties_Class:             return QStringLiteral("document-properties");
    case it_Properties_CodeGeneration:    return QStringLiteral("document-export");
    case it_Properties_CodeImport:        return QStringLiteral("document-import");
    case it_Properties_CodeViewer:        return QStringLiteral("package_graphics_viewer");
    case it_Properties_Color:             return QStringLiteral("preferences-desktop-color");
    case it_Properties_Columns:           return QStringLiteral("preferences-other");
    case it_Properties_Contents:          return QStringLiteral("preferences-other");
    case it_Properties_Display:           return QStringLiteral("preferences-desktop-theme");
    case it_Properties_EntityAttributes:  return QStringLiteral("preferences-other");
    case it_Properties_EntityConstraints: return QStringLiteral("preferences-other");
    case it_Properties_EnumLiterals:      return QStringLiteral("preferences-other");
    case it_Properties_Font:              return QStringLiteral("preferences-desktop-font");
    case it_Properties_General:           return QStringLiteral("preferences-other");
    case it_Properties_Operations:        return QStringLiteral("preferences-other");
    case it_Properties_Roles:             return QStringLiteral("preferences-other");
    case it_Properties_Templates:         return QStringLiteral("preferences-other");
    case it_Properties_UserInterface:     return QStringLiteral("preferences-desktop-theme");
    case it_Protected_Attribute:          return QStringLiteral("CVprotected_var");
    case it_Protected_Method:             return QStringLiteral("CVprotected_meth");
    case it_Public_Attribute:             return QStringLiteral("CVpublic_var");
    case it_Public_Method:                return QStringLiteral("CVpublic_meth");
    case it_Redo:                         return QStringLiteral("edit-redo");
    case it_Refactor:                     return QStringLiteral("refactor");
    case it_Region:                       return QStringLiteral("region");
    case it_Relationship:                 return QStringLiteral("relationship");
    case it_Remove:                       return QStringLiteral("remove");
    case it_Rename:                       return QStringLiteral("edit-rename");
    case it_Send_Signal:                  return QStringLiteral("send_signal");
    case it_Show:                         return QStringLiteral("document-preview");
    case it_State:                        return QStringLiteral("state");
    case it_State_Activity:               return QStringLiteral("text-x-generic");
    case it_State_Transition:             return QStringLiteral("uniassociation");
    case it_Subsystem:                    return QStringLiteral("subsystem");
    case it_Tab_Close:                    return QStringLiteral("tab-close");
    case it_Tab_New:                      return QStringLiteral("tab-new");
    case it_Template:                     return QStringLiteral("template");
    case it_Template_Class:               return QStringLiteral("format-justify-fill");
    case it_Template_Interface:           return QStringLiteral("text-x-generic-template");
    case it_Template_New:                 return QStringLiteral("text-x-generic-template");
    case it_Text:                         return QStringLiteral("text");
    case it_Undo:                         return QStringLiteral("edit-undo");
    case it_UndoView:                     return QStringLiteral("document-save");
    case it_Uniassociation:               return QStringLiteral("uniassociation");
    case it_Unique_Constraint:            return QStringLiteral("unique_constraint");
    case it_UseCase:                      return QStringLiteral("usecase");
    case it_View_Code:                    return QStringLiteral("text-x-generic");
    case it_Zoom_100:                     return QStringLiteral("zoom-original");
    case it_Zoom_Slider:                  return QStringLiteral("zoom-original");
    default: return QString();
    }
}

}  // namespace
