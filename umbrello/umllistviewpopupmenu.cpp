/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umllistviewpopupmenu.h"
#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

UMLListViewPopupMenu::UMLListViewPopupMenu(QWidget *parent, UMLListViewItem::ListViewType type, UMLObject *object)
  : ListPopupMenu(parent)
{
    m_isListView = true;
    m_TriggerObject.m_Object = object;
    m_TriggerObjectType = tot_Object;
    MenuType mt = mt_Undefined;
    switch(type) {
    case UMLListViewItem::lvt_Actor: mt = mt_Actor; break;
    case UMLListViewItem::lvt_Artifact: mt = mt_Artifact; break;
    case UMLListViewItem::lvt_Attribute: mt = mt_Attribute; break;
    case UMLListViewItem::lvt_Category: mt = mt_Category; break;
    case UMLListViewItem::lvt_CheckConstraint: mt = mt_CheckConstraint; break;
    case UMLListViewItem::lvt_Class: mt = mt_Class; break;
    case UMLListViewItem::lvt_Component: mt = mt_Component; break;
    case UMLListViewItem::lvt_Datatype: mt = mt_Datatype; break;
    case UMLListViewItem::lvt_EntityAttribute: mt = mt_EntityAttribute; break;
    case UMLListViewItem::lvt_Entity: mt = mt_Entity; break;
    case UMLListViewItem::lvt_EnumLiteral: mt = mt_EnumLiteral; break;
    case UMLListViewItem::lvt_Enum: mt = mt_Enum; break;
    case UMLListViewItem::lvt_ForeignKeyConstraint: mt = mt_ForeignKeyConstraint; break;
    case UMLListViewItem::lvt_Instance: mt = mt_Instance; break;
    case UMLListViewItem::lvt_InstanteAttribute: mt = mt_InstanceAttribute; break;
    case UMLListViewItem::lvt_Interface: mt = mt_Interface; break;
    case UMLListViewItem::lvt_Model: mt = mt_Model; break;
    case UMLListViewItem::lvt_Node: mt = mt_Node; break;
    case UMLListViewItem::lvt_Operation: mt = mt_Operation; break;
    case UMLListViewItem::lvt_Package: mt = mt_Package; break;
    case UMLListViewItem::lvt_Port: mt = mt_Port; break;
    case UMLListViewItem::lvt_PrimaryKeyConstraint: mt = mt_PrimaryKeyConstraint; break;
    case UMLListViewItem::lvt_Subsystem: mt = mt_Subsystem; break;
    case UMLListViewItem::lvt_Template: mt = mt_Template; break;
    case UMLListViewItem::lvt_UniqueConstraint: mt = mt_UniqueConstraint; break;
    case UMLListViewItem::lvt_UseCase: mt = mt_UseCase; break;
    case UMLListViewItem::lvt_Component_Folder: mt = mt_Component_Folder; break;
    case UMLListViewItem::lvt_Datatype_Folder: break; // let it mt_Undefined
    case UMLListViewItem::lvt_Deployment_Folder: mt = mt_Deployment_Folder; break;
    case UMLListViewItem::lvt_EntityRelationship_Folder: mt = mt_EntityRelationship_Folder; break;
    case UMLListViewItem::lvt_Logical_Folder: mt = mt_Logical_Folder; break;
    case UMLListViewItem::lvt_UseCase_Folder: mt = mt_UseCase_Folder; break;
    case UMLListViewItem::lvt_Activity_Diagram: mt = mt_Activity_Diagram; break;
    case UMLListViewItem::lvt_Class_Diagram: mt = mt_Class_Diagram; break;
    case UMLListViewItem::lvt_Collaboration_Diagram: mt = mt_Collaboration_Diagram; break;
    case UMLListViewItem::lvt_Component_Diagram: mt = mt_Component_Diagram; break;
    case UMLListViewItem::lvt_Deployment_Diagram: mt = mt_Deployment_Diagram; break;
    case UMLListViewItem::lvt_EntityRelationship_Diagram: mt = mt_EntityRelationship_Diagram; break;
    case UMLListViewItem::lvt_Object_Diagram: mt = mt_Object_Diagram; break;
    case UMLListViewItem::lvt_Sequence_Diagram: mt = mt_Sequence_Diagram; break;
    case UMLListViewItem::lvt_State_Diagram: mt = mt_State_Diagram; break;
    case UMLListViewItem::lvt_UseCase_Diagram: mt = mt_UseCase_Diagram; break;
    case UMLListViewItem::lvt_EntityRelationship_Model: mt = mt_EntityRelationship_Model; break;
    case UMLListViewItem::lvt_Component_View: mt = mt_Component_View; break;
    case UMLListViewItem::lvt_Deployment_View: mt = mt_Deployment_View; break;
    case UMLListViewItem::lvt_Logical_View: mt = mt_Logical_View; break;
    case UMLListViewItem::lvt_UseCase_View: mt = mt_UseCase_View; break;
    case UMLListViewItem::lvt_Properties_AutoLayout: mt = mt_Properties_AutoLayout; break;
    case UMLListViewItem::lvt_Properties_Class: mt = mt_Properties_Class; break;
    case UMLListViewItem::lvt_Properties_CodeGeneration: mt = mt_Properties_CodeGeneration; break;
    case UMLListViewItem::lvt_Properties_CodeImport: mt = mt_Properties_CodeImport; break;
    case UMLListViewItem::lvt_Properties_CodeViewer: mt = mt_Properties_CodeViewer; break;
    case UMLListViewItem::lvt_Properties_Font: mt = mt_Properties_Font; break;
    case UMLListViewItem::lvt_Properties_General: mt = mt_Properties_General; break;
    case UMLListViewItem::lvt_Properties: mt = mt_Properties; break;
    case UMLListViewItem::lvt_Properties_UserInterface: mt = mt_Properties_UserInterface; break;
    default:
        uWarning() << "unhandled ListViewType " << UMLListViewItem::toString(type);
        break;
    }
    setupMenu(mt);
    setupActionsData();
}

void UMLListViewPopupMenu::setupMenu(MenuType type)
{
    switch (type) {
    case mt_Component_Folder:
        insertSubMenuNew(type);
        insertStdItems();
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Deployment_Folder:
        insertSubMenuNew(type);
        insertStdItems();
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_EntityRelationship_Folder:
        insertSubMenuNew(type);
        insertStdItems();
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Logical_Folder:
        insertContainerItems(true);
        insertStdItems();
        insert(mt_Import_Project);
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_UseCase_Folder:
        insertSubMenuNew(type);
        insertStdItems();
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_EntityRelationship_Model:
        insertSubMenuNew(type);
        addSeparator();
        insert(mt_Paste);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Component_View:
        insertSubMenuNew(type);
        addSeparator();
        insert(mt_Paste);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Deployment_View:
        insertSubMenuNew(type);
        addSeparator();
        insert(mt_Paste);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Logical_View:
        insertContainerItems(true);
        addSeparator();
        insert(mt_Paste);
        addSeparator();
        insert(mt_Import_Project);
        insert(mt_Import_Class);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_UseCase_View:
        insertSubMenuNew(type);
        addSeparator();
        //  insert(mt_Cut);
        //  insert(mt_Copy);
        insert(mt_Paste);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Activity_Diagram:
    case mt_Class_Diagram:
    case mt_Collaboration_Diagram:
    case mt_Component_Diagram:
    case mt_Deployment_Diagram:
    case mt_EntityRelationship_Diagram:
    case mt_Object_Diagram:
    case mt_UseCase_Diagram:
    case mt_Sequence_Diagram:
    case mt_State_Diagram:
        insertStdItems(false);
        insert(mt_Clone);
        insert(mt_Export_Image);
        insert(mt_Properties);
        break;
    default:
        break;
    }
    ListPopupMenu::setupMenu(type);
}

/**
 * Override of related method from class ListPopupMenu
 * @param type menu type
 * @param menu pointer to menu instance or 0 to create a new instance
 */
void UMLListViewPopupMenu::insertSubMenuNew(MenuType type, KMenu *menu)
{
    if (!menu) {
        menu = makeNewMenu();
    }
    switch (type) {
    case mt_Deployment_View:
        insert(mt_Deployment_Folder, menu);
        insert(mt_Node, menu);
        insert(mt_Deployment_Diagram, menu);
    break;
    case mt_EntityRelationship_Model:
        insert(mt_EntityRelationship_Folder, menu);
        insert(mt_Entity, menu);
        insert(mt_Category, menu);
        insert(mt_EntityRelationship_Diagram, menu);
        break;
    case mt_UseCase_View:
        insert(mt_UseCase_Folder, menu);
        insert(mt_Actor, menu);
        insert(mt_UseCase, menu);
        insert(mt_UseCase_Diagram, menu);
        break;
    case mt_Component_View:
    case mt_Component_Folder:
        insert(mt_Component_Folder, menu);
        insert(mt_Subsystem, menu);
        insert(mt_Component, menu);
        insert(mt_Artifact, menu);
        insert(mt_Component_Diagram, menu);
        break;
    case mt_Deployment_Folder:
        insert(mt_Deployment_Folder, menu);
        insert(mt_Node, menu);
        insert(mt_Deployment_Diagram, menu);
        break;
    case mt_UseCase_Folder:
        insert(mt_UseCase_Folder, menu);
        insert(mt_Actor, menu);
        insert(mt_UseCase, menu);
        insert(mt_UseCase_Diagram, menu);
        break;
    case mt_EntityRelationship_Folder:
        insert(mt_EntityRelationship_Folder, menu);
        insert(mt_Entity, menu);
        insert(mt_EntityRelationship_Diagram, menu);
        break;
    default:
        ListPopupMenu::insertSubMenuNew(type, menu);
        return;
    }
    addMenu(menu);
}

void UMLListViewPopupMenu::insertStdItems(bool insertLeadingSeparator, WidgetBase::WidgetType type)
{
    if (insertLeadingSeparator)
        addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    insert(mt_Delete);
}

