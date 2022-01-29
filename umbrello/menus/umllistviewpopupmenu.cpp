/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umllistviewpopupmenu.h"
#include "debug_utils.h"
#include "folder.h"
#include "optionstate.h"
#include "model_utils.h"
#include "uml.h"  // only needed for log{Warn,Error}

// kde includes
#include <KLocalizedString>

UMLListViewPopupMenu::UMLListViewPopupMenu(QWidget *parent, UMLListViewItem *item)
  : ListPopupMenu(parent)
{
    UMLObject *object = item->umlObject();
    UMLListViewItem::ListViewType type = item->type();
    switch(type) {
        case UMLListViewItem::lvt_Logical_View:
            insertContainerItems(true, true, true);
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
            insertContainerItems(true, true, true);
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

    case UMLListViewItem::lvt_Artifact:
            insert(mt_Open_File);
            insertStdItems(true);
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Class:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            addSeparator();
            if (object && object->stereotype() == QLatin1String("class-or-package")) {
                insert(mt_ChangeToClass, i18n("Change into Class"));
                insert(mt_ChangeToPackage, i18n("Change into Package"));
            }
            addSeparator();
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Package:
            insertContainerItems(true, false, true);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Subsystem:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Component:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            addSeparator();
            insert(mt_Expand_All);
            insert(mt_Collapse_All);
            break;

        case UMLListViewItem::lvt_Datatype:
        case UMLListViewItem::lvt_EnumLiteral:
        case UMLListViewItem::lvt_Port:
        case UMLListViewItem::lvt_Node:
        case UMLListViewItem::lvt_Actor:
        case UMLListViewItem::lvt_UseCase:
        case UMLListViewItem::lvt_Attribute:
        case UMLListViewItem::lvt_EntityAttribute:
        case UMLListViewItem::lvt_InstanceAttribute:
        case UMLListViewItem::lvt_Operation:
        case UMLListViewItem::lvt_Template:
            insertStdItems(false);
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Interface:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Enum:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Category:
            insertSubMenuCategoryType(object->asUMLCategory());
            insertStdItems(false);
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Entity:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Instance:
            insertSubMenuNew(type);
            insertStdItems();
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_UniqueConstraint:
        case UMLListViewItem::lvt_PrimaryKeyConstraint:
        case UMLListViewItem::lvt_ForeignKeyConstraint:
        case UMLListViewItem::lvt_CheckConstraint:
            insert(mt_Rename);
            insert(mt_Delete);
            insert(mt_Show);
            insert(mt_Properties);
            break;

        case UMLListViewItem::lvt_Model:
            insert(mt_Model, i18n("Rename..."));
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
        case UMLListViewItem::lvt_Class:
            insert(mt_Attribute, menu);
            insert(mt_Operation, menu);
            insert(mt_Template, menu);
            insertContainerItems(menu, false, false, false);
            break;
        case UMLListViewItem::lvt_Component:
            insert(mt_Component, menu);
            if (Settings::optionState().generalState.uml2)
                insert(mt_Port, menu);
            insert(mt_Artifact, menu);
            break;
        case UMLListViewItem::lvt_Interface:
            insert(mt_Operation, menu);
            insert(mt_Template, menu);
            insertContainerItems(menu, false, false, false);
            break;
        case UMLListViewItem::lvt_Entity:
            insert(mt_EntityAttribute, menu);
            insert(mt_PrimaryKeyConstraint, menu);
            insert(mt_UniqueConstraint, menu);
            insert(mt_ForeignKeyConstraint, menu);
            insert(mt_CheckConstraint, menu);
            break;
        case UMLListViewItem::lvt_Enum:
            insert(mt_EnumLiteral, menu);
            break;
        //case UMLListViewItem::lvt_Object:
        //    break;
        case UMLListViewItem::lvt_Subsystem:
            insert(mt_Subsystem, menu);
            insert(mt_Component, menu);
            insert(mt_Artifact, menu);
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

/**
 * Inserts a menu item for externalization/de-externalization
 * of a folder.
 */
void UMLListViewPopupMenu::insertSubmodelAction()
{
    const Settings::OptionState& ostat = Settings::optionState();
    if (ostat.generalState.tabdiagrams) {
        // Umbrello currently does not support External Folders
        // in combination with Tabbed Diagrams.
        // If you need external folders then disable the tabbed diagrams
        // in the General Settings.
        return;
    }
    UMLObject *o = Model_Utils::treeViewGetCurrentObject();
    if (o == 0) {
        logError0("UMLListViewPopupMenu::insertSubmodelAction: "
                  "Model_Utils::treeViewGetCurrentObject() returns NULL");
        return;
    }
    const UMLFolder *f = o->asUMLFolder();
    if (f == 0) {
        logError1("UMLListViewPopupMenu::insertSubmodelAction: %1 is not a Folder", o->name());
        return;
    }
    QString submodelFile = f->folderFile();
    if (submodelFile.isEmpty()) {
        insert(mt_Externalize_Folder);
    }
    else {
        insert(mt_Internalize_Folder);
    }
}
