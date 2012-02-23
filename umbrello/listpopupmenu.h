/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LISTPOPUPMENU_H
#define LISTPOPUPMENU_H

#include "basictypes.h"
#include "widgetbase.h"
#include "umllistviewitem.h"
#include "umlobject.h"

#include <kmenu.h>
// #include <kaction.h>

#include <QtCore/QHash>

class UMLView;
class ClassifierWidget;
class UMLCategory;
class UMLWidget;

/**
 * A popup menu that depending on what type it is set to will
 * display a different menu.
 *
 * @short Displays a popup menu.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ListPopupMenu : public KMenu
{
    Q_OBJECT
    Q_ENUMS(MenuType)
public:

    enum MenuType  ///< This type hosts all possible menu types.
    {
        mt_Model,
        mt_Logical_View,
        mt_UseCase_View,
        mt_Component_View,
        mt_Deployment_View,
        mt_EntityRelationship_Model,
        mt_UseCase_Diagram,
        mt_Sequence_Diagram,
        mt_Class_Diagram,
        mt_Collaboration_Diagram,
        mt_State_Diagram,
        mt_Activity_Diagram,
        mt_Component_Diagram,
        mt_Deployment_Diagram,
        mt_EntityRelationship_Diagram,
        mt_On_UseCase_Diagram,
        mt_On_Sequence_Diagram,
        mt_On_Class_Diagram,
        mt_On_Collaboration_Diagram,
        mt_On_State_Diagram,
        mt_On_Activity_Diagram,
        mt_On_Component_Diagram,
        mt_On_Deployment_Diagram,
        mt_On_EntityRelationship_Diagram,
        mt_Logical_Folder,
        mt_UseCase_Folder,
        mt_Component_Folder,
        mt_Deployment_Folder,
        mt_EntityRelationship_Folder,
        mt_Class,
        mt_Package,
        mt_Subsystem,
        mt_Component,
        mt_Node,
        mt_Artifact,
        mt_Interface,
        mt_Enum,
        mt_Entity,
        mt_Datatype,
        mt_Actor,
        mt_UseCase,
        mt_Attribute,
        mt_EntityAttribute,
        mt_EnumLiteral,
        mt_UniqueConstraint,
        mt_PrimaryKeyConstraint,
        mt_ForeignKeyConstraint,
        mt_CheckConstraint,
        mt_Object,
        mt_Category,
        mt_DisjointSpecialisation,
        mt_OverlappingSpecialisation,
        mt_Union,
        mt_Initial_State,
        mt_End_State,
        mt_State,
        mt_Junction,
        mt_DeepHistory,
        mt_ShallowHistory,
        mt_Choice,
        mt_StateFork,
        mt_StateJoin,
        mt_Activity,
        mt_Initial_Activity,
        mt_End_Activity,
        mt_Operation,
        mt_Template,
        mt_New_Parameter,
        mt_New_Operation,
        mt_New_Attribute,
        mt_New_Template,
        mt_New_EnumLiteral,
        mt_New_EntityAttribute,
        mt_New_UniqueConstraint,
        mt_New_PrimaryKeyConstraint,
        mt_New_ForeignKeyConstraint,
        mt_New_CheckConstraint,
        mt_Parameter_Selected,
        mt_Operation_Selected,
        mt_Attribute_Selected,
        mt_Template_Selected,
        mt_EnumLiteral_Selected,
        mt_EntityAttribute_Selected,
        mt_UniqueConstraint_Selected,
        mt_PrimaryKeyConstraint_Selected,
        mt_ForeignKeyConstraint_Selected,
        mt_CheckConstraint_Selected,
        mt_Association_Selected,                 // Association without role names
        mt_Show_Attributes,
        mt_Show_Attributes_Selection,            // SHOWATTS, multiple items
        mt_Show_Operations,
        mt_Show_Operations_Selection,            // SHOWOPS, multiple items
        mt_Show_Packages,
        mt_Show_Packages_Selection,              // SHOWPACKAGE, multiple items
        mt_Show_Stereotypes,
        mt_Show_Stereotypes_Selection,           // SHOWSTEREOTYPE, multiple items
        mt_Visibility,
        mt_Visibility_Selection,                 // SCOPE, multiple items
        mt_DrawAsCircle,
        mt_DrawAsCircle_Selection,               // DRAWASCIRCLE, multiple items
        mt_ChangeToClass,
        mt_ChangeToClass_Selection,
        mt_ChangeToInterface,
        mt_ChangeToInterface_Selection,
        mt_Rename_Object,
        mt_Select_Operation,
        mt_Anchor,
        mt_Properties,
        mt_Rename,
        mt_Delete,
        mt_Export_Image,
        mt_Sequence_Number,
        mt_Cut,
        mt_Copy,
        mt_Paste,
        mt_Clear,
        mt_Redo,
        mt_Undo,
        mt_Link_Docs,
        mt_Show_Operation_Signature,
        mt_Show_Operation_Signature_Selection,   //SHOWOPSIG, multiple items
        mt_Show_Attribute_Signature,
        mt_Show_Attribute_Signature_Selection,   //SHOWATTSIG, multiple items
        mt_Message_Text,
        mt_Collaboration_Message,
        mt_FloatText,
        mt_MultiA,
        mt_MultiB,
        mt_Name,                                 // Association name
        mt_FullAssociation,                      // Association with role names
        mt_AttributeAssociation,                 // Rendering of an attribute as an association
        mt_RoleNameA,
        mt_RoleNameB,
        mt_Delete_Selection,
        mt_Reset_Label_Positions,
        mt_Line_Color,
        mt_Fill_Color,
        mt_Use_Fill_Color,
        mt_Default_Properties,
        mt_Rename_MultiA,
        mt_Rename_MultiB,
        mt_Rename_Name,
        mt_Rename_RoleAName,
        mt_Rename_RoleBName,
        mt_Change_Font,
        mt_Change_Font_Selection,
        mt_SnapToGrid,
        mt_ShowSnapGrid,
        mt_Activity_Selected,
        mt_New_Activity,
        mt_Up,
        mt_Down,
        mt_Branch,
        mt_Flip,

        mt_Expand_All,                           // Expand all items in the list
        mt_Collapse_All,                         // Collapse all items in the list

        mt_Refactoring,
        mt_ViewCode,                             // view code document contents
        mt_Clone,                                // Create a deep copy of the object.
        mt_Show_Public_Only,                     // (not currently used)
        mt_Show_Public_Only_Selection,           // Show public operations/attributes only.
        mt_Externalize_Folder,                   // Mark folder for saving as separate submodel
        mt_Internalize_Folder,                   // Reintegrate separate submodel into main model

        mt_AddInteractionOperand,                // add a dash line to an alternative or a parallel combined fragment

        mt_Undefined  =  - 1
    };

    static QString toString(MenuType menu);

    explicit ListPopupMenu(QWidget* parent, MenuType type = mt_Undefined, UMLView* view = 0);
    ListPopupMenu(QWidget* parent, UMLListViewItem::ListViewType type, UMLObject* object);
    ListPopupMenu(QWidget* parent, UMLWidget* object, bool multi = false, bool unique = false);

    ~ListPopupMenu();

    static UMLObject::ObjectType convert_MT_OT(MenuType mt);
    static Uml::DiagramType convert_MT_DT(MenuType mt);

    static ListPopupMenu* menuFromAction(QAction *action);

//    KAction* getAction(MenuType idx);
    QAction* getAction(MenuType idx);

    void setActionEnabled(MenuType idx, bool value);

//    MenuType getMenuType(KAction* action);
    MenuType getMenuType(QAction* action);

    WidgetBase *ownerWidget() const;

private:

    void insert(MenuType m);
    void insert(const MenuType m, KMenu* menu);
    void insert(const MenuType m, const QIcon & icon, const QString & text);
    void insert(const MenuType m, const QString & text, const bool checkable = false);
    void insert(const MenuType m, KMenu* menu, const QIcon & icon, const QString & text);
    void insert(const MenuType m, KMenu* menu, const QString & text, const bool checkable = false);

    void insertStdItems(bool insertLeadingSeparator = true,
                        WidgetBase::WidgetType type = WidgetBase::wt_UMLWidget);
    void insertContainerItems(bool folderAndDiagrams);
    void insertAssocItem(const QString &label, MenuType mt);
    void insertSubmodelAction();

    void makeMultiClassifierPopup(ClassifierWidget *c);
    void makeClassifierPopup(ClassifierWidget *c);
    KMenu* makeCategoryTypeMenu(UMLCategory* category);

    void insertSubMenuNew(MenuType type);
    void insertSubMenuColor(bool fc);

    void setupDiagramMenu(UMLView* view);
    void setupMenu(MenuType type);

    void setActionChecked(MenuType idx, bool value);
    void setupActionsData();

    union TriggerObject {  ///< The List Popup Menu is triggered either by right clicking on the View, a ListViewItem (Object), or a widget.
        UMLView* m_View;
        UMLObject* m_Object;
        UMLWidget* m_Widget;
    };

    enum TriggerObjectType {  ///< Enum to keep track on TriggerObject Type.
        tot_View,
        tot_Object,
        tot_Widget
    };

    TriggerObject m_TriggerObject;
    TriggerObjectType m_TriggerObjectType;

//    QHash<MenuType, KAction*> m_actions;
    QHash<MenuType, QAction*> m_actions;

};

/// Need this for ability to store ListPopupMenu* in a QVariant
Q_DECLARE_METATYPE(ListPopupMenu*)

#endif
