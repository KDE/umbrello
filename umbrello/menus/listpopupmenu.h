/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef LISTPOPUPMENU_H
#define LISTPOPUPMENU_H

#include "basictypes.h"
#include "umllistviewitem.h"
#include "umlobject.h"
#include "widgetbase.h"
#include <QMenu>

#include <QHash>

class UMLCategory;
class ListPopupMenuPrivate;

/**
 * A popup menu that depending on what type is set to will
 * display a different menu.
 *
 * The data field of actions is used to carry user data
 * between objects. Because different types of data are used, a map is loaded
 * with an enum as key (see @ref DataType).
 *
 * @short Displays a popup menu.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ListPopupMenu : public QMenu
{
    Q_OBJECT
    Q_ENUMS(MenuType)
    Q_ENUMS(DataType)
public:
    enum MenuType  ///< This type hosts all possible menu entry types
    {
        mt_Model,
        // diagrams
        mt_Activity_Diagram,
        mt_Class_Diagram,
        mt_Collaboration_Diagram,
        mt_Component_Diagram,
        mt_Deployment_Diagram,
        mt_EntityRelationship_Diagram,
        mt_Sequence_Diagram,
        mt_State_Diagram,
        mt_UseCase_Diagram,
        mt_Logical_Folder,
        mt_UseCase_Folder,
        mt_Component_Folder,
        mt_Deployment_Folder,
        mt_EntityRelationship_Folder,
        // widgets, uml objects
        mt_Accept_Signal,
        mt_Accept_Time_Event,
        mt_Activity,
        mt_Activity_Transition,
        mt_Actor,
        mt_Artifact,
        mt_Attribute,
        mt_Branch,
        mt_Category,
        mt_CheckConstraint,
        mt_Choice,
        mt_Class,
        mt_CombinedState,
        mt_Component,
        mt_Datatype,
        mt_DeepHistory,
        mt_DisjointSpecialisation,
        mt_EditCombinedState,
        mt_End_Activity,
        mt_End_State,
        mt_Entity,
        mt_EntityAttribute,
        mt_Enum,
        mt_EnumLiteral,
        mt_Exception,
        mt_FloatText,
        mt_Final_Activity,
        mt_ForeignKeyConstraint,
        mt_Fork,
        mt_GoToStateDiagram,
        mt_Initial_Activity,
        mt_Initial_State,
        mt_Invoke_Activity,
        mt_Instance,
        mt_InstanceAttribute,
        mt_Interface,
        mt_InterfaceComponent,
        mt_InterfaceProvided,
        mt_InterfaceRequired,
        mt_Junction,
        mt_MessageAsynchronous,
        mt_MessageCreation,
        mt_MessageDestroy,
        mt_MessageFound,
        mt_MessageLost,
        mt_MessageSynchronous,
        mt_Node,
        mt_Note,
        mt_Object,
        mt_Object_Node,
        mt_Operation,
        mt_OverlappingSpecialisation,
        mt_Package,
        mt_Param_Activity,
        mt_Pin,
        mt_Port,
        mt_PrePostCondition,
        mt_PrimaryKeyConstraint,
        mt_Region,
        mt_SelectStateDiagram,
        mt_Send_Signal,
        mt_ShallowHistory,
        mt_State,
        mt_StateFork,
        mt_StateJoin,
        mt_StateTransition,
        mt_Subsystem,
        mt_Template,
        mt_Union,
        mt_UniqueConstraint,
        mt_UseCase,
        // new from dialogs
        mt_New_Activity,
        mt_New_Attribute,
        mt_New_EntityAttribute,
        mt_New_EnumLiteral,
        mt_New_InstanceAttribute,
        mt_New_Operation,
        mt_New_Parameter,
        mt_New_Template,
        mt_RemoveStateDiagram,
        mt_ReturnToClass,
        mt_ReturnToCombinedState,
        // selection
        // visual properties
        mt_Hide_Attribute_Signature_Selection,
        mt_Hide_Attributes_Selection,            // Unset visual property on multiple widgets
        mt_Hide_Destruction_Box,
        mt_Hide_NonPublic_Selection,             // Could be named "show public only"
        mt_Hide_Operation_Signature_Selection,
        mt_Hide_Operations_Selection,            // Hide operations
        mt_Hide_Packages_Selection,
        mt_Hide_Stereotypes_Selection,
        mt_Hide_Visibility_Selection,
        mt_Show_Attribute_Signature,
        mt_Show_Attribute_Signature_Selection,
        mt_Show_Attributes_Selection,            // Set visual property on multiple widgets
        mt_Show_Attributes,                      // Toggle visual property on a widget
        mt_Show_Destruction_Box,
        mt_Show_Documentation,
        mt_Show_NonPublic_Selection,             // Could be named "hide public only" (crazy!)
        mt_Show_Operation_Signature,
        mt_Show_Operation_Signature_Selection,
        mt_Show_Operations_Selection,            // Show operations
        mt_Show_Operations,                      // Toggle 'show operations'
        mt_Show_Packages,                        // etc...
        mt_Show_Packages_Selection,
        mt_Show_Public_Only,
        mt_Show_Stereotypes,
        mt_Show_Stereotypes_Selection,
        mt_Show_Visibility_Selection,
        mt_Visibility,
        // other
        mt_DrawAsCircle,
        mt_ChangeToClass,
        mt_ChangeToInterface,
        mt_ChangeToPackage,
        mt_Open_File,
        mt_Rename_Object,
        mt_Select_Operation,
        mt_Properties,
        mt_Rename,
        mt_NameAsTooltip,
        mt_Show,
        mt_Delete,
        mt_Export_Image,
        mt_Import_Class,
        mt_Import_Project,
        mt_Cut,
        mt_Copy,
        mt_Paste,
        mt_Clear,
        mt_Redo,
        mt_Undo,
        mt_Reset_Label_Positions,
        mt_Line_Color,
        mt_Line_Color_Selection,
        mt_Fill_Color,
        mt_Fill_Color_Selection,
        mt_Use_Fill_Color,
        mt_Set_Use_Fill_Color_Selection,
        mt_Unset_Use_Fill_Color_Selection,
        mt_Rename_MultiA,
        mt_Rename_MultiB,
        mt_Rename_Name,
        mt_Rename_RoleAName,
        mt_Rename_RoleBName,
        mt_Change_Font,
        mt_Change_Font_Selection,
        mt_SnapToGrid,
        mt_SnapComponentSizeToGrid,
        mt_ShowDocumentationIndicator,
        mt_ShowSnapGrid,
        mt_AutoResize,
        mt_Resize,
        mt_Up,
        mt_Down,
        mt_FlipHorizontal,
        mt_FlipVertical,
        mt_Add_Point,
        mt_Delete_Point,
        mt_Auto_Layout_Spline,

        mt_Expand_All,                           // Expand all items in the list
        mt_Collapse_All,                         // Collapse all items in the list

        mt_Refactoring,
        mt_ViewCode,                             // view code document contents
        mt_Clone,                                // Create a deep copy of the object.
        mt_Externalize_Folder,                   // Mark folder for saving as separate submodel
        mt_Internalize_Folder,                   // Reintegrate separate submodel into main model

        mt_AddInteractionOperand,                // add a dash line to an alternative or a parallel combined fragment
        mt_Apply_Layout,                         // apply automatically created layout
        mt_Apply_Layout1,                        // apply automatically created layout
        mt_Apply_Layout2,                        // apply automatically created layout
        mt_Apply_Layout3,                        // apply automatically created layout
        mt_Apply_Layout4,                        // apply automatically created layout
        mt_Apply_Layout5,                        // apply automatically created layout
        mt_Apply_Layout6,                        // apply automatically created layout
        mt_Apply_Layout7,                        // apply automatically created layout
        mt_Apply_Layout8,                        // apply automatically created layout
        mt_Apply_Layout9,                        // apply automatically created layout

        mt_LayoutDirect,                         // associations with direct lines
        mt_LayoutSpline,                         // associations with slines
        mt_LayoutOrthogonal,                     // associations with orthogonal lines
        mt_LayoutPolyline,                       // associations with polylines

        mt_Align_Right,
        mt_Align_Left,
        mt_Align_Top,
        mt_Align_Bottom,
        mt_Align_VerticalMiddle,
        mt_Align_HorizontalMiddle,
        mt_Align_VerticalDistribute,
        mt_Align_HorizontalDistribute,
        mt_Import_from_File,
        mt_Remove,
        // add new entries above
        mt_Undefined  =  - 1
    };

    static QString toString(MenuType menu);

    enum DataType  ///< Key value of the data map used in actions.
    {
        dt_MenuPointer,
        dt_ApplyLayout
    };

    static QString toString(DataType data);
    static QVariant dataFromAction(DataType key, QAction* action);

    ListPopupMenu(QWidget* parent = 0);

    virtual ~ListPopupMenu();

    static UMLObject::ObjectType convert_MT_OT(MenuType mt);

    static ListPopupMenu* menuFromAction(QAction *action);
    static MenuType       typeFromAction(QAction *action);

    QAction* getAction(MenuType idx);

    void setActionEnabled(MenuType idx, bool value);

    MenuType getMenuType(QAction* action);

    void dumpActions(const QString &title);

    QMenu *newMenu(const QString &title, QWidget *widget);
    void addMenu(QMenu *menu);
protected:
    void insert(MenuType m);
    void insertFromActionKey(const MenuType m, QMenu *menu, const QString &action);
    void insert(const MenuType m, QMenu* menu);
    void insert(const MenuType m, QMenu* menu, const QIcon & icon, const QString & text);
    void insert(const MenuType m, QMenu* menu, const QString & text, const bool checkable = false);
    void insert(const MenuType m, const QIcon & icon, const QString & text);
    void insert(const MenuType m, const QString & text, const bool checkable = false);

    void insertContainerItems(bool folders, bool diagrams, bool packages);
    void insertContainerItems(QMenu* menu, bool folders, bool diagrams, bool packages);
    void insertAssociationTextItem(const QString &label, MenuType mt);

    QMenu *makeNewMenu();
    void insertSubMenuCategoryType(UMLCategory *category);

    void setActionChecked(MenuType idx, bool value);
    void setupActionsData();

    QHash<MenuType, QAction*> m_actions;
    ListPopupMenuPrivate *d;
};

/// Need this for ability to store ListPopupMenu* in a QVariant
Q_DECLARE_METATYPE(ListPopupMenu*)

#endif
