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

UMLListViewPopupMenu::UMLListViewPopupMenu(QWidget *parent, UMLListViewItem *item)
  : ListPopupMenu(parent)
{
    m_isListView = true;
    MenuType mt = mt_Undefined;
    UMLListViewItem::ListViewType type = item->type();
    switch(type) {
        case UMLListViewItem::lvt_Logical_View:
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

        case UMLListViewItem::lvt_Component_View:
            insertSubMenuNew(type);
            addSeparator();
            insert(mt_Paste);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Deployment_View:
            insertSubMenuNew(type);
            addSeparator();
            insert(mt_Paste);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_EntityRelationship_Model:
            insertSubMenuNew(type);
            addSeparator();
            insert(mt_Paste);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_UseCase_View:
            insertSubMenuNew(type);
            addSeparator();
            //  insert(mt_Cut);
            //  insert(mt_Copy);
            insert(mt_Paste);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Logical_Folder:
            insertContainerItems(true);
            insertStdItems();
            insert(mt_Import_Project);
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Datatype_Folder:
            insertSubMenuNew(type);
            insertStdItems();
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Component_Folder:
            insertSubMenuNew(type);
            insertStdItems();
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Deployment_Folder:
            insertSubMenuNew(type);
            insertStdItems();
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_UseCase_Folder:
            insertSubMenuNew(type);
            insertStdItems();
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_EntityRelationship_Folder:
            insertSubMenuNew(type);
            insertStdItems();
            insertSubmodelAction();
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_UseCase_Diagram:
        case UMLListViewItem::lvt_Sequence_Diagram:
        case UMLListViewItem::lvt_Class_Diagram:
        case UMLListViewItem::lvt_Object_Diagram:
        case UMLListViewItem::lvt_Collaboration_Diagram:
        case UMLListViewItem::lvt_State_Diagram:
        case UMLListViewItem::lvt_Activity_Diagram:
        case UMLListViewItem::lvt_Component_Diagram:
        case UMLListViewItem::lvt_Deployment_Diagram:
        case UMLListViewItem::lvt_EntityRelationship_Diagram:
            insertStdItems(false);
            insert(mt_Clone);
            insert(mt_Export_Image);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Actor:
            mt = mt_Actor;
            break;

        case UMLListViewItem::lvt_UseCase:
            mt = mt_UseCase;
            break;

        case UMLListViewItem::lvt_Class:
            mt = mt_Class;
            break;

        case UMLListViewItem::lvt_Package:
            mt = mt_Package;
            break;

        case UMLListViewItem::lvt_Subsystem:
            mt = mt_Subsystem;
            break;

        case UMLListViewItem::lvt_Component:
            mt = mt_Component;
            break;

        case UMLListViewItem::lvt_Port:
            mt = mt_Port;
            break;

        case UMLListViewItem::lvt_Node:
            mt = mt_Node;
            break;

        case UMLListViewItem::lvt_Artifact:
            mt = mt_Artifact;
            break;

        case UMLListViewItem::lvt_Interface:
            mt = mt_Interface;
            break;

        case UMLListViewItem::lvt_Enum:
            mt = mt_Enum;
            break;

        case UMLListViewItem::lvt_EnumLiteral:
            mt = mt_EnumLiteral;
            break;

        case UMLListViewItem::lvt_Datatype:
            mt = mt_Datatype;
            break;

        case UMLListViewItem::lvt_Attribute:
            mt = mt_Attribute;
            break;

        case UMLListViewItem::lvt_Operation:
            mt = mt_Operation;
            break;

        case UMLListViewItem::lvt_Template:
            mt = mt_Template;
            break;

        case UMLListViewItem::lvt_Category:
            mt = mt_Category;
            break;

        case UMLListViewItem::lvt_Entity:
            mt = mt_Entity;
            break;

        case UMLListViewItem::lvt_EntityAttribute:
            mt = mt_EntityAttribute;
            break;

        case UMLListViewItem::lvt_Instance:
            mt = mt_Instance;
            break;

        case UMLListViewItem::lvt_InstanteAttribute:
            mt = mt_InstanceAttribute;
            break;

        case UMLListViewItem::lvt_UniqueConstraint:
            mt = mt_UniqueConstraint;
            break;

        case UMLListViewItem::lvt_PrimaryKeyConstraint:
            mt = mt_PrimaryKeyConstraint;
            break;

        case UMLListViewItem::lvt_ForeignKeyConstraint:
            mt = mt_ForeignKeyConstraint;
            break;

        case UMLListViewItem::lvt_CheckConstraint:
            mt = mt_CheckConstraint;
            break;

        case UMLListViewItem::lvt_Model:
            mt = mt_Model;
            break;

        case UMLListViewItem::lvt_Properties:
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Properties_AutoLayout:
        case UMLListViewItem::lvt_Properties_Class:
        case UMLListViewItem::lvt_Properties_CodeImport:
        case UMLListViewItem::lvt_Properties_CodeGeneration:
        case UMLListViewItem::lvt_Properties_CodeViewer:
        case UMLListViewItem::lvt_Properties_Font:
        case UMLListViewItem::lvt_Properties_General:
        case UMLListViewItem::lvt_Properties_UserInterface:
            insert(mt_Properties);
            break;
#ifndef CHECK_SWITCH
        default:
            break;
#endif
    }

    if (mt != mt_Undefined)
        setupMenu(mt);
    setupActionsData();
}

/**
 * Override of related method from class ListPopupMenu
 * @param type list view type
 * @param menu pointer to menu instance or 0 to create a new instance
 */
void UMLListViewPopupMenu::insertSubMenuNew(UMLListViewItem::ListViewType type, KMenu *menu)
{
    if (!menu) {
        menu = makeNewMenu();
    }

    switch(type) {
        case UMLListViewItem::lvt_Deployment_View:
            insert(mt_Deployment_Folder, menu);
            insert(mt_Node, menu);
            insert(mt_Deployment_Diagram, menu);
            break;
        case UMLListViewItem::lvt_EntityRelationship_Model:
            insert(mt_EntityRelationship_Folder, menu);
            insert(mt_Entity, menu);
            insert(mt_Category, menu);
            insert(mt_EntityRelationship_Diagram, menu);
            break;
        case UMLListViewItem::lvt_UseCase_View:
            insert(mt_UseCase_Folder, menu);
            insert(mt_Actor, menu);
            insert(mt_UseCase, menu);
            insert(mt_UseCase_Diagram, menu);
            break;
        case UMLListViewItem::lvt_Component_View:
        case UMLListViewItem::lvt_Component_Folder:
            insert(mt_Component_Folder, menu);
            insert(mt_Subsystem, menu);
            insert(mt_Component, menu);
            insert(mt_Artifact, menu);
            insert(mt_Component_Diagram, menu);
            break;
        case UMLListViewItem::lvt_Datatype_Folder:
            insert(mt_Datatype, menu);
            break;
        case UMLListViewItem::lvt_Deployment_Folder:
            insert(mt_Deployment_Folder, menu);
            insert(mt_Node, menu);
            insert(mt_Deployment_Diagram, menu);
            break;
        case UMLListViewItem::lvt_UseCase_Folder:
            insert(mt_UseCase_Folder, menu);
            insert(mt_Actor, menu);
            insert(mt_UseCase, menu);
            insert(mt_UseCase_Diagram, menu);
            break;
        case UMLListViewItem::lvt_EntityRelationship_Folder:
            insert(mt_EntityRelationship_Folder, menu);
            insert(mt_Entity, menu);
            insert(mt_EntityRelationship_Diagram, menu);
            break;
       default:
            delete menu;
            return;
    }
    addMenu(menu);
}

void UMLListViewPopupMenu::insertStdItems(bool insertLeadingSeparator)
{
    if (insertLeadingSeparator)
        addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    insert(mt_Delete);
}

