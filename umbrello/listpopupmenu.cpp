/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "listpopupmenu.h"

// app includes
#include "activitywidget.h"
#include "category.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "combinedfragmentwidget.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "folder.h"
#include "forkjoinwidget.h"
#include "model_utils.h"
#include "objectnodewidget.h"
#include "objectwidget.h"
#include "preconditionwidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlwidget.h"
#include "widget_utils.h"

// kde includes
#include <klocale.h>
#include <kactioncollection.h>

const bool CHECKABLE = true;

/**
 * Constructs the popup menu for a UMLView (diagram).
 *
 * @param parent   The parent to ListPopupMenu.
 * @param type     The type of menu to display.
 * @param view     The UMLView object
 */
ListPopupMenu::ListPopupMenu(QWidget *parent, MenuType type, UMLView * view)
  : KMenu(parent)
{
    m_TriggerObject.m_View = view;
    m_TriggerObjectType = tot_View;
    setupMenu(type);
}

/**
 * Constructs the popup menu for a tree list view item.
 *
 * @param parent   The parent to ListPopupMenu.
 * @param type     The type of menu to display.
 * @param object   The UMLObject of the ListViewItem
 */
ListPopupMenu::ListPopupMenu(QWidget *parent, UMLListViewItem::ListViewType type, UMLObject* object)
  : KMenu(parent)
{
    m_TriggerObject.m_Object = object;
    m_TriggerObjectType = tot_Object;
    MenuType mt = mt_Undefined;
    switch(type) {
    case UMLListViewItem::lvt_Logical_View:
        mt = mt_Logical_View;
        break;

    case UMLListViewItem::lvt_UseCase_View:
        mt = mt_UseCase_View;
        break;

    case UMLListViewItem::lvt_Component_View:
        mt = mt_Component_View;
        break;

    case UMLListViewItem::lvt_EntityRelationship_Model:
        mt = mt_EntityRelationship_Model;
        break;

    case UMLListViewItem::lvt_Deployment_View:
        mt = mt_Deployment_View;
        break;

    case UMLListViewItem::lvt_Logical_Folder:
        mt = mt_Logical_Folder;
        break;

    case UMLListViewItem::lvt_UseCase_Folder:
        mt = mt_UseCase_Folder;
        break;

    case UMLListViewItem::lvt_Component_Folder:
        mt = mt_Component_Folder;
        break;

    case UMLListViewItem::lvt_Deployment_Folder:
        mt = mt_Deployment_Folder;
        break;

    case UMLListViewItem::lvt_EntityRelationship_Folder:
        mt = mt_EntityRelationship_Folder;
        break;

    case UMLListViewItem::lvt_UseCase_Diagram:
        mt = mt_UseCase_Diagram;
        break;

    case UMLListViewItem::lvt_Class_Diagram:
        mt = mt_Class_Diagram;
        break;

    case UMLListViewItem::lvt_Collaboration_Diagram:
        mt = mt_Collaboration_Diagram;
        break;

    case UMLListViewItem::lvt_Sequence_Diagram:
        mt = mt_Sequence_Diagram;
        break;

    case UMLListViewItem::lvt_State_Diagram:
        mt = mt_State_Diagram;
        break;

    case UMLListViewItem::lvt_Activity_Diagram:
        mt = mt_Activity_Diagram;
        break;

    case UMLListViewItem::lvt_Component_Diagram:
        mt = mt_Component_Diagram;
        break;

    case UMLListViewItem::lvt_Deployment_Diagram:
        mt = mt_Deployment_Diagram;
        break;

    case UMLListViewItem::lvt_EntityRelationship_Diagram:
        mt = mt_EntityRelationship_Diagram;
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

    case UMLListViewItem::lvt_Datatype_Folder:
        // let it mt_Undefined
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

    default:
        uWarning() << "unhandled ListViewType "
                   << UMLListViewItem::toString(type);
        break;
    }
    setupMenu(mt);
}

/**
 * Constructs the popup menu for a canvas widget.
 *
 * @param parent   The parent to ListPopupMenu.
 * @param object   The UMLWidget to represent a menu for.
 * @param multi    True if multiple items are selected.
 * @param unique   True if multiple selected items all have
 *                 the same type (e.g. Class, Interface)
 */
ListPopupMenu::ListPopupMenu(QWidget * parent, UMLWidget * object,
                             bool multi, bool unique)
  : KMenu(parent)
{
    m_TriggerObject.m_Widget = object;
    m_TriggerObjectType = tot_Widget;
    //make the right menu for the type
    //make menu for logical view
    if (!object)
        return;
    UMLWidget::WidgetType type = object->baseType();
    // uDebug() << "ListPopupMenu created with multi=" << multi << " , unique="
    //          << unique << " for WidgetType=" << type;

    if (multi) {
        ClassifierWidget *c = NULL;
        if (unique && (type == WidgetBase::wt_Class || type == WidgetBase::wt_Interface)) {
            c = static_cast<ClassifierWidget *>( object );
            makeMultiClassifierPopup(c);
        }
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        addSeparator();
        insert(mt_Change_Font_Selection, Icon_Utils::SmallIcon(Icon_Utils::it_Change_Font), i18n("Change Font..."));
        insert(mt_Delete_Selection, Icon_Utils::SmallIcon(Icon_Utils::it_Delete), i18n("Delete Selected Items"));

        // add this here and not above with the other stuff of the interface
        // user might expect it at this position of the context menu
        if (unique) {
            if (type == WidgetBase::wt_Interface) {
                insert(mt_DrawAsCircle_Selection, i18n("Draw as Circle"), CHECKABLE);
                setActionChecked(mt_DrawAsCircle_Selection, c->getDrawAsCircle());
                insert(mt_ChangeToClass_Selection, i18n("Change into Class"));
            } else if (type == WidgetBase::wt_Class) {
                UMLClassifier *umlc = c->classifier();
                if (umlc->isAbstract() && umlc->attributes() == 0)
                    insert(mt_ChangeToInterface_Selection, i18n("Change into Interface"));
            }
        }

        return;
    }

    switch (type) {
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Category:
       {
         KMenu* m = makeCategoryTypeMenu(
                        static_cast<UMLCategory*>(object->umlObject()));
         m->setTitle(i18n("Category Type"));
         addMenu(m);
         insertSubMenuColor(object->useFillColor());
         insertStdItems(true, type);
         insert(mt_Rename);
         insert(mt_Change_Font);
         break;
       }
    case WidgetBase::wt_Class:
    case WidgetBase::wt_Interface:
        makeClassifierPopup(static_cast<ClassifierWidget*>(object));
        break;

    case WidgetBase::wt_Enum:
        insertSubMenuNew(mt_Enum);
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Entity:
        insertSubMenuNew(mt_Entity);
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Datatype:
    case WidgetBase::wt_Package:
    case WidgetBase::wt_Component:
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(false, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Object:
        {
            UMLView * pView = static_cast<UMLView *>(parent);
            //Used for sequence diagram and collaboration diagram widgets
            insertSubMenuColor( object->useFillColor() );
            if( pView->type() == Uml::DiagramType::Sequence ) {
                addSeparator();
                MenuType tabUp = mt_Up;
                insert(mt_Up, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Up), i18n("Move Up"));
                insert(mt_Down, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Down), i18n("Move Down"));
                if ( !(static_cast<ObjectWidget*>(object))->canTabUp() ) {
                    setActionEnabled(tabUp, false);
                }
            }
            insertStdItems(true, type);
            insert(mt_Rename, i18n("Rename Class..."));
            insert(mt_Rename_Object, i18n("Rename Object..."));
            insert(mt_Change_Font);
            insert(mt_Properties);
        }
        break;

    case WidgetBase::wt_Message:
        insertStdItems(false, type);
        //insert(mt_Change_Font);
        //insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
        //insert(mt_Select_Operation, i18n("Select Operation..."));
        break;

    case WidgetBase::wt_Note:
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear note", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        break;

    case WidgetBase::wt_Box:
        insertStdItems(false, type);
        insert(mt_Line_Color);
        break;

    case WidgetBase::wt_State:
        {
            StateWidget* pState = static_cast< StateWidget *>( object );
            if (pState->stateType() == StateWidget::Normal) {
                insertSubMenuNew(mt_New_Activity);
            }
            insertSubMenuColor( object->useFillColor() );
            insertStdItems(false, type);
            if (pState->stateType() == StateWidget::Normal) {
                insert(mt_Rename, i18n("Change State Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case WidgetBase::wt_ForkJoin:
        {
            ForkJoinWidget *pForkJoin = static_cast<ForkJoinWidget*>(object);
            if (pForkJoin->getDrawVertical()) {
                insert(mt_Flip, i18n("Flip Horizontal"));
            }
            else {
                insert(mt_Flip, i18n("Flip Vertical"));
            }
            m_actions[mt_Fill_Color] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill),
                                                 i18n("Fill Color..."));
        }
        break;

    case WidgetBase::wt_Activity:
        {
            ActivityWidget* pActivity = static_cast<ActivityWidget *>(object);
            if( pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insertSubMenuColor( object->useFillColor() );
            }
            insertStdItems(false, type);
            if( pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insert(mt_Rename, i18n("Change Activity Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case WidgetBase::wt_ObjectNode:
        {
            ObjectNodeWidget* objWidget = static_cast<ObjectNodeWidget *>(object);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
              || objWidget->objectNodeType() == ObjectNodeWidget::Data
              || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insertSubMenuColor( object->useFillColor() );
            }
            insertStdItems(false, type);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
                || objWidget->objectNodeType() == ObjectNodeWidget::Data
                || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insert(mt_Rename, i18n("Change Object Node Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case WidgetBase::wt_Pin:
    case WidgetBase::wt_Signal:
    case WidgetBase::wt_FloatingDashLine:
    case WidgetBase::wt_Precondition:
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear precondition", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        break;

    case WidgetBase::wt_CombinedFragment:
        // for alternative and parallel combined fragments
        if ((static_cast<CombinedFragmentWidget*>(object))->combinedFragmentType() == CombinedFragmentWidget::Alt ||
            (static_cast<CombinedFragmentWidget*>(object))->combinedFragmentType() == CombinedFragmentWidget::Par) {
            insert(mt_AddInteractionOperand, i18n("Add Interaction Operand"));
            addSeparator();
        }
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear combined fragment", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        break;

    case WidgetBase::wt_Text:
        switch( (static_cast<FloatingTextWidget*>(object))->textRole() ) {
        case Uml::TextRole::MultiB:
            insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiB);
            break;
        case Uml::TextRole::MultiA:
            insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiA);
            break;
        case Uml::TextRole::Name:
            insertAssocItem(i18n("Change Name"), mt_Rename_Name);
            break;
        case Uml::TextRole::RoleAName:
            insertAssocItem(i18n("Change Role A Name..."), mt_Rename_RoleAName);
            break;
        case Uml::TextRole::RoleBName:
            insertAssocItem(i18n("Change Role B Name..."), mt_Rename_RoleBName);
            break;
        case Uml::TextRole::ChangeA:
        case Uml::TextRole::ChangeB:
            insert(mt_Change_Font);
            insert(mt_Reset_Label_Positions);
            insert(mt_Properties);
            break;

        case Uml::TextRole::Coll_Message_Self:
        case Uml::TextRole::Coll_Message:
        case Uml::TextRole::Seq_Message_Self:
        case Uml::TextRole::Seq_Message:
            insert(mt_Change_Font);
            insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
            insert(mt_Select_Operation, i18n("Select Operation..."));
            break;

        case Uml::TextRole::Floating:
        default:
            insertStdItems(false, type);
            insert(mt_Rename, i18n("Change Text..."));
            insert(mt_Change_Font);
            break;
        }
        break;
    default:
        uWarning() << "unhandled WidgetType " << type;
        break;
    }//end switch

    bool bCutState = UMLApp::app()->isCutCopyState();
    setActionEnabled( mt_Cut, bCutState );
    setActionEnabled( mt_Copy, bCutState );
    setActionEnabled( mt_Paste, false );
}

/**
 * Standard destructor.
 */
ListPopupMenu::~ListPopupMenu()
{
    foreach (QAction* action, m_actions) {
        delete action;
    }
    m_actions.clear();
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m   The MenuType for which to insert a menu item.
 */
void ListPopupMenu::insert(MenuType m)
{
    switch (m) {
    case mt_Properties:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Properties), i18n("Properties"));
        break;
    case mt_Rename:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Rename), i18n("Rename..."));
        break;
    case mt_Delete:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Delete), i18n("Delete"));
        break;
    case mt_Cut:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Cut), i18n("Cut"));
        break;
    case mt_Copy:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Copy), i18n("Copy"));
        break;
    case mt_Paste:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Paste), i18n("Paste"));
        break;
    case mt_Change_Font:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Change_Font), i18n("Change Font..."));
        break;
    case mt_Line_Color:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Color_Line), i18n("Line Color..."));
        break;
    case mt_Expand_All:
        m_actions[m] = addAction(i18n("Expand All"));
        break;
    case mt_Collapse_All:
        m_actions[m] = addAction(i18n("Collapse All"));
        break;
    case mt_Clone:
        m_actions[m] = addAction(i18nc("duplicate action", "Duplicate"));
        break;
    case mt_Externalize_Folder:
        m_actions[m] = addAction(i18n("Externalize Folder..."));
        break;
    case mt_Internalize_Folder:
        m_actions[m] = addAction(i18n("Internalize Folder"));
        break;
    case mt_Reset_Label_Positions:
        m_actions[m] = addAction(i18n("Reset Label Positions"));
        break;
    case mt_New_Parameter:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Parameter_New), i18n("New Parameter..."));
        break;
    case mt_New_Operation:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Operation_Public_New),i18n("New Operation..."));
        break;
    case mt_New_Attribute:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Attribute_New), i18n("New Attribute..."));
        break;
    case mt_New_Template:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n("New Template..."));
        break;
    case mt_New_EnumLiteral:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Literal_New), i18n("New Literal..."));
        break;
    case mt_New_EntityAttribute:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Entity_Attribute_New), i18n("New Entity Attribute..."));
        break;
    case mt_Export_Image:
        m_actions[m] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture), i18n("Export as Picture..."));
        break;
    default:
        uWarning() << "called on unimplemented MenuType " << toString(m);
        break;
    }
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The KMenu for which to insert a menu item.
 */
void ListPopupMenu::insert(const MenuType m, KMenu* menu)
{
    Q_ASSERT(menu != NULL);
    switch (m) {
    case mt_Subsystem:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Subsystem), i18n("Subsystem"));
        break;
    case mt_Component:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Component), i18n("Component"));
        break;
    case mt_Artifact:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Artifact), i18n("Artifact"));
        break;
    case mt_Component_Diagram:
        {
            QAction* act = UMLApp::app()->actionCollection()->action("new_component_diagram");
            //don't keep a local copy of pointer which resides somewhere else ( in this case - in actionCollection() )
            //m_actions[m] = act;
            menu->addAction(act);
        }
        break;
    case mt_Node:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Node), i18n("Node"));
        break;
    case mt_Deployment_Diagram:
        {
            QAction* act = UMLApp::app()->actionCollection()->action("new_deployment_diagram");
            //m_actions[m] = act;
            menu->addAction(act);
        }
        break;
    case mt_Deployment_Folder:
    case mt_Component_Folder:
    case mt_UseCase_Folder:
    case mt_EntityRelationship_Folder:
        m_actions[m] = menu->addAction(Icon_Utils::BarIcon(Icon_Utils::it_Folder), i18n("Folder"));
        break;
    case mt_Entity:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Entity), i18n("Entity"));
        break;
    case mt_EntityRelationship_Diagram:
        {
            QAction* act = UMLApp::app()->actionCollection()->action("new_entityrelationship_diagram");
            //m_actions[m] = act;
            menu->addAction(act);
        }
        break;
    case mt_Category:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Category), i18n("Category"));
        break;
    case mt_Actor:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Actor), i18n("Actor"));
        break;
    case mt_UseCase:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18n("Use Case"));
        break;
    case mt_UseCase_Diagram:
        {
            QAction* act = UMLApp::app()->actionCollection()->action("new_use_case_diagram");
            //m_actions[m] = act;
            menu->addAction(act);
        }
        break;
    case mt_FloatText:
        m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Text), i18n("Text Line..." ));
        break;
    default:
        uWarning() << "called on unimplemented MenuType " << toString(m);
        break;
    }
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param icon   The icon for this action.
 * @param text   The text for this action.
 */
void ListPopupMenu::insert(const MenuType m, const QIcon & icon, const QString & text)
{
    m_actions[m] = addAction(icon, text);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m           The MenuType for which to insert a menu item.
 * @param text        The text for this action.
 * @param checkable   Sets the action to checkable.
 */
void ListPopupMenu::insert(const MenuType m, const QString & text, const bool checkable)
{
    m_actions[m] = addAction(text);
    if (checkable) {
        QAction* action = getAction(m);
        if (action)
            action->setCheckable(checkable);
    }
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The KMenu for which to insert a menu item.
 * @param icon   The icon for this action.
 * @param text   The text for this action.
 */
void ListPopupMenu::insert(const MenuType m, KMenu* menu, const QIcon & icon, const QString & text)
{
    m_actions[m] = menu->addAction(icon, text);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The KMenu for which to insert a menu item.
 * @param text   The text for this action.
 * @param checkable   Sets the action to checkable.
 */
void ListPopupMenu::insert(const MenuType m, KMenu* menu, const QString & text, const bool checkable)
{
    m_actions[m] = menu->addAction(text);
    if (checkable) {
        QAction* action = getAction(m);
        if (action)
            action->setCheckable(checkable);
    }
}

/**
 * Shortcut for the frequently used insertStdItem() calls.
 *
 * @param insertLeadingSeparator   Set this true if the group shall
 *                                 start with a separator.
 * @param type      The WidgetType for which to insert the menu items.
 *                  If no argument is supplied then a Rename item will be
 *                  included.
 */
void ListPopupMenu::insertStdItems(bool insertLeadingSeparator /* = true */,
                                   WidgetBase::WidgetType type /* = wt_UMLWidget */)
{
    if (insertLeadingSeparator)
        addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    if (type == WidgetBase::wt_UMLWidget)
        insert(mt_Rename);
    else if (Model_Utils::isCloneable(type))
        insert(mt_Clone);
    insert(mt_Delete);
}

/**
 * Shortcut for inserting standard model items (Class, Interface,
 * Datatype, Enum, Package) as well as diagram choices.
 *
 * @param folderAndDiagrams Set this true if folders and diagram
 *                          types shall be included as choices.
 */
void ListPopupMenu::insertContainerItems(bool folderAndDiagrams)
{
    KMenu* menu = new KMenu(i18nc("new container menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    if (folderAndDiagrams)
        insert(mt_Logical_Folder, menu, Icon_Utils::BarIcon(Icon_Utils::it_Folder), i18n("Folder"));
    insert(mt_Class, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Class), i18nc("new class menu item", "Class"));
    insert(mt_Interface, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Interface), i18n("Interface"));
    insert(mt_Datatype, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Datatype), i18n("Datatype"));
    insert(mt_Enum, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Enum), i18n("Enum"));
    insert(mt_Package, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Package), i18n("Package"));
    if (folderAndDiagrams) {
        menu->addAction(UMLApp::app()->actionCollection()->action("new_class_diagram"));
        menu->addAction(UMLApp::app()->actionCollection()->action("new_sequence_diagram"));
        menu->addAction(UMLApp::app()->actionCollection()->action("new_collaboration_diagram"));
        menu->addAction(UMLApp::app()->actionCollection()->action("new_state_diagram"));
        menu->addAction(UMLApp::app()->actionCollection()->action("new_activity_diagram"));
    }
    addMenu(menu);
}

/**
 * Inserts a menu item for an association related text
 * (such as name, role, multiplicity etc.)
 *
 * @param label   The menu text.
 * @param mt      The menu type.
 */
void ListPopupMenu::insertAssocItem(const QString &label, MenuType mt)
{
    insert(mt, label);
    insert(mt_Change_Font);
    insert(mt_Reset_Label_Positions);
    insert(mt_Properties);
}

/**
 * Inserts a menu item for externalization/de-externalization
 * of a folder.
 */
void ListPopupMenu::insertSubmodelAction()
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
    if (o == NULL) {
        uError() << " Model_Utils::treeViewGetCurrentObject() returns NULL";
        return;
    }
    UMLFolder *f = dynamic_cast<UMLFolder*>(o);
    if (f == NULL) {
        uError() << o->name() << " is not a Folder";
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

/**
 * Creates a popup menu for a multiple selection of class and
 * interface widgets.
 */
void ListPopupMenu::makeMultiClassifierPopup(ClassifierWidget *c)
{
    WidgetBase::WidgetType type = c->baseType();
    ClassifierWidget *cls = NULL;

    KMenu* show = new KMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));
    if (type == WidgetBase::wt_Class) {
        cls = static_cast<ClassifierWidget*>(c);
        insert(mt_Show_Attributes_Selection, show, i18n("Attributes"), CHECKABLE);
        setActionChecked(mt_Show_Attributes_Selection, cls->getShowAtts());
    }
    insert(mt_Show_Operations_Selection, show, i18n("Operations"), CHECKABLE);
    setActionChecked(mt_Show_Operations_Selection, c->getShowOps());
    insert(mt_Show_Public_Only_Selection, show, i18n("Public Only"), CHECKABLE);
    setActionChecked(mt_Show_Public_Only_Selection, c->getShowPublicOnly());
    insert(mt_Visibility_Selection, show, i18n("Visibility"), CHECKABLE);
    setActionChecked(mt_Visibility_Selection, c->getShowVisibility());
    insert(mt_Show_Operation_Signature_Selection, show, i18n("Operation Signature"), CHECKABLE);
    bool sig = (c->operationSignatureType() == Uml::SignatureType::SigNoVis ||
                c->operationSignatureType() == Uml::SignatureType::ShowSig);
    setActionChecked(mt_Show_Operation_Signature_Selection, sig);
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Attribute_Signature_Selection, show, i18n("Attribute Signature"), CHECKABLE);
        sig = (cls->attributeSignatureType() == Uml::SignatureType::SigNoVis ||
               cls->attributeSignatureType() == Uml::SignatureType::ShowSig);
        setActionChecked(mt_Show_Attribute_Signature_Selection, sig);
    }
    insert(mt_Show_Packages_Selection, show, i18n("Package"), CHECKABLE);
    setActionChecked(mt_Show_Packages_Selection, c->getShowPackage());
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Stereotypes_Selection, show, i18n("Stereotype"), CHECKABLE);
        setActionChecked(mt_Show_Stereotypes_Selection, cls->getShowStereotype());
    }
    addMenu(show);
}

/**
 * Creates a popup menu for a single class or interface widgets.
 */
void ListPopupMenu::makeClassifierPopup(ClassifierWidget *c)
{
    WidgetBase::WidgetType type = c->baseType();
    KMenu* menu = new KMenu(i18nc("new classifier menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    if (type == WidgetBase::wt_Class)
        insert(mt_Attribute, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Attribute), i18n("Attribute..."));
    insert(mt_Operation, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method), i18n("Operation..."));
    insert(mt_Template, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n( "Template..." ));
    addMenu(menu);

    makeMultiClassifierPopup(c);

    insertSubMenuColor(c->useFillColor());
    insertStdItems(true, type);
    insert(mt_Rename);
    insert(mt_Change_Font);
    if (type == WidgetBase::wt_Interface) {
        insert(mt_DrawAsCircle, i18n("Draw as Circle"), CHECKABLE);
        setActionChecked(mt_DrawAsCircle, c->getDrawAsCircle());
        insert(mt_ChangeToClass, i18n("Change into Class"));
    } else {
        insert(mt_Refactoring, Icon_Utils::SmallIcon(Icon_Utils::it_Refactor), i18n("Refactor"));
        insert(mt_ViewCode, Icon_Utils::SmallIcon(Icon_Utils::it_View_Code), i18n("View Code"));
        UMLClassifier *umlc = c->classifier();
        if (umlc->isAbstract() && umlc->attributes() == 0)
            insert(mt_ChangeToInterface, i18n("Change into Interface"));
    }
    insert(mt_Properties);
}

/**
 * Shortcut for commonly used sub menu initializations.
 *
 * @param fc   The "Use Fill Color" is checked.
 */
void ListPopupMenu::insertSubMenuColor(bool fc)
{
    KMenu* color = new KMenu(i18nc("color menu", "Color"), this);
    insert(mt_Line_Color, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Line), i18n("Line Color..."));
    insert(mt_Fill_Color, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill), i18n("Fill Color..."));
    insert(mt_Use_Fill_Color, color, i18n("Use Fill Color"), CHECKABLE);
    setActionChecked(mt_Use_Fill_Color, fc);
    addMenu(color);
}

/**
 * Utility: Convert a MenuType value to a Diagram_Type value.
 */
Uml::DiagramType ListPopupMenu::convert_MT_DT(MenuType mt)
{
    Uml::DiagramType type = Uml::DiagramType::Undefined;

    switch (mt) {
    case mt_UseCase_Diagram:
        type = Uml::DiagramType::UseCase;
        break;
    case mt_Class_Diagram:
        type = Uml::DiagramType::Class;
        break;
    case mt_Sequence_Diagram:
        type = Uml::DiagramType::Sequence;
        break;
    case mt_Collaboration_Diagram:
        type = Uml::DiagramType::Collaboration;
        break;
    case mt_State_Diagram:
        type = Uml::DiagramType::State;
        break;
    case mt_Activity_Diagram:
        type = Uml::DiagramType::Activity;
        break;
    case mt_Component_Diagram:
        type = Uml::DiagramType::Component;
        break;
    case mt_Deployment_Diagram:
        type = Uml::DiagramType::Deployment;
        break;
    case mt_EntityRelationship_Diagram:
        type = Uml::DiagramType::EntityRelationship;
        break;
    default:
        break;
    }
    return type;
}

/**
 * Utility: Convert a MenuType value to an ObjectType value.
 */
UMLObject::ObjectType ListPopupMenu::convert_MT_OT(MenuType mt)
{
    UMLObject::ObjectType type =  UMLObject::ot_UMLObject;

    switch (mt) {
    case mt_UseCase:
        type = UMLObject::ot_UseCase;
        break;
    case mt_Actor:
        type = UMLObject::ot_Actor;
        break;
    case mt_Class:
        type = UMLObject::ot_Class;
        break;
    case mt_Attribute:
        type = UMLObject::ot_Attribute;
        break;
    case mt_Template:
        type = UMLObject::ot_Template;
        break;
    case mt_EnumLiteral:
        type = UMLObject::ot_EnumLiteral;
        break;
    case mt_EntityAttribute:
        type = UMLObject::ot_EntityAttribute;
        break;
    case mt_Operation:
        type = UMLObject::ot_Operation;
        break;
    case mt_Category:
        type = UMLObject::ot_Category;
        break;
    default:
        break;
    }
    return type;
}

/**
 * Convenience method to extract the ListPopupMenu pointer stored in QAction
 * objects belonging to ListPopupMenu.
 */
ListPopupMenu* ListPopupMenu::menuFromAction(QAction *action)
{
    if (action) {
        QVariant data = action->data();
        if (qVariantCanConvert<ListPopupMenu*>(data)) {
            return qvariant_cast<ListPopupMenu*>(data);
        }
    }
    return 0;
}

/**
 * Shortcut for commonly used sub menu initializations.
 *
 * @param type   The MenuType for which to set up the menu.
 */
void ListPopupMenu::insertSubMenuNew(MenuType type)
{
    KMenu* menu = new KMenu(i18nc("new sub menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
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
        case mt_Component:
            insert(mt_Component, menu);
            insert(mt_Artifact, menu);
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
        case mt_On_UseCase_Diagram:
            insert(mt_Actor, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Actor), i18n( "Actor..." ));
            insert(mt_UseCase, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18n( "Use Case..."));
            insert(mt_FloatText, menu);
            break;
        case mt_On_Class_Diagram:
            insert(mt_Class, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Class), i18nc("new class menu item", "Class..."));
            insert(mt_Interface, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Interface), i18n("Interface..."));
            insert(mt_Datatype, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Datatype), i18n("Datatype..."));
            insert(mt_Enum, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Enum), i18n("Enum..."));
            insert(mt_Package, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Package), i18n("Package..."));
            insert(mt_FloatText, menu);
            break;
        case mt_On_State_Diagram:
            insert(mt_Initial_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_InitialState), i18n("Initial State"));
            insert(mt_End_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_EndState), i18n("End State"));
            insert(mt_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18nc("add new state", "State..."));
            insert(mt_Junction, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Junction), i18n("Junction"));
            insert(mt_DeepHistory, menu, Icon_Utils::SmallIcon(Icon_Utils::it_History_Deep), i18n("Deep History"));
            insert(mt_ShallowHistory, menu, Icon_Utils::SmallIcon(Icon_Utils::it_History_Shallow), i18n("Shallow History"));
            insert(mt_Choice, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Choice_Rhomb), i18n("Choice"));
            insert(mt_StateFork, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Fork_State), i18n("Fork"));
            insert(mt_StateJoin, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Join), i18n("Join"));
            insert(mt_FloatText, menu);
            break;
        case mt_On_Activity_Diagram:
            insert(mt_Initial_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_InitialState), i18n("Initial Activity"));
            insert(mt_End_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_EndState), i18n("End Activity"));
            insert(mt_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18n("Activity..."));
            insert(mt_Branch, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Branch), i18n("Branch/Merge"));
            insert(mt_FloatText, menu);
            break;
        case mt_On_Component_Diagram:
            insert(mt_Subsystem, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Subsystem), i18n("Subsystem..."));
            insert(mt_Component, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Component), i18n("Component..."));
            insert(mt_Artifact, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Artifact), i18n("Artifact..."));
            break;
        case mt_On_Deployment_Diagram:
            insert(mt_Node, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Node), i18n("Node..."));
            break;
        case mt_On_EntityRelationship_Diagram:
            insert(mt_Entity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Entity), i18n("Entity..."));
            insert(mt_Category, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Category), i18n("Category..."));
            break;
        case mt_On_Sequence_Diagram:
        case mt_On_Collaboration_Diagram:
            insert(mt_Object, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Object), i18n("Object..."));
            insert(mt_FloatText, menu);
            break;
        case mt_Class:
            insert(mt_Attribute, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Attribute), i18n("Attribute"));
            insert(mt_Operation, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method), i18n("Operation"));
            insert(mt_Template, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Template_Class), i18n("Template"));
            break;
        case mt_Interface:
            insert(mt_Operation, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method), i18n("Operation"));
            insert(mt_Template, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Template_Interface), i18n("Template"));
            break;
        case mt_Entity:
            insert(mt_EntityAttribute, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Entity_Attribute), i18n("Entity Attribute..."));
            insert(mt_PrimaryKeyConstraint, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Constraint_PrimaryKey),i18n("Primary Key Constraint..."));
            insert(mt_UniqueConstraint, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Constraint_Unique),i18n("Unique Constraint..."));
            insert(mt_ForeignKeyConstraint, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Constraint_ForeignKey),i18n("Foreign Key Constraint..."));
            insert(mt_CheckConstraint, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Constraint_Check),i18n("Check Constraint..."));
            break;
        case mt_Enum:
            insert(mt_EnumLiteral, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Enum_Literal), i18n("Enum Literal..."));
            break;
        case mt_New_Activity:
        case mt_Activity_Selected:
            insert(mt_New_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_State_Activity), i18n("Activity..."));
            break;
        case mt_Subsystem:
            insert(mt_Subsystem, menu);
            insert(mt_Component, menu);
            insert(mt_Artifact, menu);
            break;
        default:
            break;
    }
    addMenu(menu);
}

/**
 * Shortcut for commonly used menu initializations.
 *
 * @param type   The MenuType for which to set up the menu.
 */
void ListPopupMenu::setupMenu(MenuType type)
{
    // uDebug() << "ListPopupMenu created for MenuType=" << toString(type);

    switch (type) {
    case mt_Logical_View:
        insertContainerItems(true);
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

    case mt_EntityRelationship_Model:
        insertSubMenuNew(type);
        addSeparator();
        insert(mt_Paste);
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

    case mt_Logical_Folder:
        insertContainerItems(true);
        insertStdItems();
        insertSubmodelAction();
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

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

    case mt_UseCase_Folder:
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

    case mt_UseCase_Diagram:
    case mt_Sequence_Diagram:
    case mt_Class_Diagram:
    case mt_Collaboration_Diagram:
    case mt_State_Diagram:
    case mt_Activity_Diagram:
    case mt_Component_Diagram:
    case mt_Deployment_Diagram:
    case mt_EntityRelationship_Diagram:
        //don't insert standard items because cut/copy are not currently
        // possible with tabbed diagrams (it didn't work anyway)
        //insertStdItems(false);
        insert(mt_Rename);
        insert(mt_Delete);
        insert(mt_Export_Image);
        insert(mt_Properties);
        break;

    case mt_On_UseCase_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_Class_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_State_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_Activity_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_Component_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_Deployment_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_EntityRelationship_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_On_Sequence_Diagram:
    case mt_On_Collaboration_Diagram:
        insertSubMenuNew(type);
        addSeparator();
        if ( m_TriggerObjectType != tot_View ) {
            uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
            return;
        }
        setupDiagramMenu(m_TriggerObject.m_View);
        break;

    case mt_Class:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        break;

    case mt_Interface:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        break;

    case mt_Package:
        insertContainerItems(false);
        insertStdItems();
        insert(mt_Properties);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Subsystem:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Component:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        addSeparator();
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;

    case mt_Entity:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        break;

    case mt_Enum:
        insertSubMenuNew(type);
        insertStdItems();
        insert(mt_Properties);
        break;

    case mt_Datatype:
    case mt_EnumLiteral:
    case mt_Node:
    case mt_Artifact:
    case mt_Actor:
    case mt_UseCase:
    case mt_Attribute:
    case mt_EntityAttribute:
    case mt_Operation:
    case mt_Template:
        insertStdItems(false);
        insert( mt_Properties );
        break;

    case mt_Category:
        {
            if (m_TriggerObjectType != tot_Object) {
                uError() << "Invalid Trigger Object Type Set for Use Case Diagram " << m_TriggerObjectType;
                return;
            }
            KMenu* menu = makeCategoryTypeMenu(static_cast<UMLCategory*>(m_TriggerObject.m_Object));
            menu->setTitle(i18n("Category Type"));
            addMenu(menu);
            insertStdItems(false);
        }
        break;

    case mt_UniqueConstraint:
    case mt_PrimaryKeyConstraint:
    case mt_ForeignKeyConstraint:
    case mt_CheckConstraint:
        insert(mt_Rename);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_New_Parameter:
        insert(mt_New_Parameter);
        break;

    case mt_New_Operation:
        insert(mt_New_Operation);
        break;

    case mt_New_Attribute:
        insert(mt_New_Attribute);
        break;

    case mt_New_Template:
        insert(mt_New_Template);
        break;

    case mt_New_EnumLiteral:
        insert(mt_New_EnumLiteral);
        break;

    case mt_New_EntityAttribute:
        insert(mt_New_EntityAttribute);
        break;

    case mt_New_UniqueConstraint:
        insert(mt_New_UniqueConstraint);
        break;

    case mt_New_PrimaryKeyConstraint:
        insert(mt_New_PrimaryKeyConstraint);
        break;

    case mt_New_ForeignKeyConstraint:
        insert(mt_New_ForeignKeyConstraint);
        break;

    case mt_New_CheckConstraint:
        insert(mt_New_CheckConstraint);
        break;

    case mt_New_Activity:
        insertSubMenuNew(type);
        break;

    case mt_Activity_Selected:
        insertSubMenuNew(type);
        insert(mt_Rename);
        insert(mt_Delete);
        break;

    case mt_Parameter_Selected:
        insert(mt_New_Parameter);
        insert(mt_Rename);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_Operation_Selected:
        insert(mt_New_Operation);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_Attribute_Selected:
        insert(mt_New_Attribute);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_Template_Selected:
        insert(mt_New_Attribute, Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n("New Template..."));
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_EnumLiteral_Selected:
        insert(mt_New_EnumLiteral);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_EntityAttribute_Selected:
        insert(mt_New_EntityAttribute);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_UniqueConstraint_Selected:
        insert(mt_New_UniqueConstraint);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_PrimaryKeyConstraint_Selected:
        insert(mt_New_PrimaryKeyConstraint);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_ForeignKeyConstraint_Selected:
        insert(mt_New_ForeignKeyConstraint);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_CheckConstraint_Selected:
        insert(mt_New_ForeignKeyConstraint);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case mt_Association_Selected:
        insert(mt_Delete);
        insert(mt_Line_Color);
        insert(mt_Properties);
        break;

    case mt_Anchor:
        insert(mt_Delete, Icon_Utils::SmallIcon(Icon_Utils::it_Delete), i18n("Delete Anchor"));
        break;

    case mt_RoleNameA:
        insertAssocItem(i18n("Change Role A Name..."), mt_Rename_RoleAName);
        break;

    case mt_RoleNameB:
        insertAssocItem(i18n("Change Role B Name..."), mt_Rename_RoleBName);
        break;

    case mt_MultiB:
        insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiB);
        break;

    case mt_MultiA:
        insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiA);
        break;

    case mt_Name:
        insertAssocItem(i18n("Change Name"), mt_Rename_Name);
        break;

    case mt_FullAssociation:
        insert(mt_Delete);
        insert(mt_Rename_Name, i18n("Change Association Name..."));
        insert(mt_Rename_RoleAName, i18n("Change Role A Name..."));
        insert(mt_Rename_RoleBName, i18n("Change Role B Name..."));
        insert(mt_Change_Font);
        insert(mt_Reset_Label_Positions);
        insert(mt_Line_Color);
        insert(mt_Properties);
        break;

    case mt_AttributeAssociation:
        insert(mt_Delete);  // @todo add more items
        break;

    case mt_Collaboration_Message:
        // insert(mt_Cut);
        // insert(mt_Copy);
        // insert(mt_Paste);
        // addSeparator();
        insert(mt_Delete);
        insert(mt_Change_Font);
        insert(mt_New_Operation);
        insert(mt_Select_Operation, i18n("Select Operation..."));
        break;

    case mt_Model:
        insert(mt_Model, i18n("Rename..."));
        break;

    default:
        insert(mt_Expand_All);
        insert(mt_Collapse_All);
        break;
    }//end switch

    if( m_TriggerObjectType == tot_View ) {
        bool bCutState = UMLApp::app()->isCutCopyState();
        setActionEnabled( mt_Undo, UMLApp::app()->isUndoEnabled() );
        setActionEnabled( mt_Redo, UMLApp::app()->isRedoEnabled() );
        setActionEnabled( mt_Cut, bCutState );
        setActionEnabled( mt_Copy, bCutState );
        setActionEnabled( mt_Paste, UMLApp::app()->isPasteState() );
    }

}

/**
 * Shortcut for diagramm menu initializations.
 */
void ListPopupMenu::setupDiagramMenu(UMLView* view)
{
    insert(mt_Undo, Icon_Utils::SmallIcon(Icon_Utils::it_Undo), i18n("Undo"));
    insert(mt_Redo, Icon_Utils::SmallIcon(Icon_Utils::it_Redo), i18n("Redo"));
    addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18n("Clear Diagram"));
    insert(mt_Export_Image);
    addSeparator();
    insert(mt_SnapToGrid, i18n("Snap to Grid"), CHECKABLE);
    setActionChecked(mt_SnapToGrid, view->getSnapToGrid() );
    insert(mt_ShowSnapGrid, i18n("Show Grid"), CHECKABLE);
    setActionChecked(mt_ShowSnapGrid, view->getShowSnapGrid() );
    insert(mt_Properties);
}

/**
 * Creates a popup menu for a single category Object
 * @param category The UMLCategory for which the category menu is created
 */
KMenu* ListPopupMenu::makeCategoryTypeMenu(UMLCategory* category)
{
    KMenu* catTypeMenu = new KMenu(this);
    insert(mt_DisjointSpecialisation, catTypeMenu, i18n("Disjoint(Specialisation)"), CHECKABLE);
    insert(mt_OverlappingSpecialisation, catTypeMenu, i18n("Overlapping(Specialisation)"), CHECKABLE);
    insert(mt_Union, catTypeMenu, i18n("Union"), CHECKABLE);
    setActionChecked(mt_DisjointSpecialisation, category->getType()==UMLCategory::ct_Disjoint_Specialisation );
    setActionChecked(mt_OverlappingSpecialisation, category->getType()==UMLCategory::ct_Overlapping_Specialisation );
    setActionChecked(mt_Union, category->getType()==UMLCategory::ct_Union );

    return catTypeMenu;
}

/**
 * Get the action from the menu type as index.
 */
QAction* ListPopupMenu::getAction(MenuType idx)
{
    return m_actions.value(idx, NULL);
}

// /**
//  * Get the MenuType from the action.
//  */
// ListPopupMenu::MenuType ListPopupMenu::getMenuType(KAction* action)
// {
//     return m_actions.key(action);
// }

/**
 * Get the MenuType from the action.
 */
ListPopupMenu::MenuType ListPopupMenu::getMenuType(QAction* action)
{
    QList<MenuType> keyList = m_actions.keys( action );
    if ( keyList.empty() || /* all key-value pairs are unique*/ keyList.count() > 1 ) {
        return mt_Undefined;
    } else {
        // we return the first ( only ) value
        return keyList.first();
    }
}

/**
 * Utility method to fetch owner of the menu.
 *
 * @return The owned WidgetBase if this menu is owned by a
 *         widget. Otherwise returns 0.
 */
WidgetBase* ListPopupMenu::ownerWidget() const
{
    if (m_TriggerObjectType == tot_Widget) {
        return m_TriggerObject.m_Widget;
    }
    return 0;
}

/**
 * Checks the action item.
 *
 * @param idx     The MenuType for which to check the menu item.
 * @param value   The value.
 */
void ListPopupMenu::setActionChecked(MenuType idx, bool value)
{
    QAction* action = getAction(idx);
    if (action && action->isCheckable())
        action->setChecked(value);
#ifdef VERBOSE_DEBUGGING
    else
        uWarning() << "called on unknown MenuType " << toString(idx);
#endif
}

/**
 * Enables the action item.
 *
 * @param idx     The MenuType for which to enable the menu item.
 * @param value   The value.
 */
void ListPopupMenu::setActionEnabled(MenuType idx, bool value)
{
    QAction* action = getAction(idx);
    if (action)
        action->setEnabled(value);
#ifdef VERBOSE_DEBUGGING
    else
        uWarning() << "called on unknown MenuType " << toString(idx);
#endif
}

/**
 * Sets up actions added to the ListPopupMenu to have their data field set to
 * pointer to this ListPopupMenu object, so that this menu pointer can be
 * retrieved in UMLWidget::slotMenuSelection
 *
 * @note This might seem like an ugly hack, but this is the only solution which
 *       helped in avoiding storage of ListPopupMenu pointer in each UMLWidget.
 */
void ListPopupMenu::setupActionsData()
{
    foreach (QAction *action, m_actions) {
        action->setData(qVariantFromValue(this));
    }

}

QString ListPopupMenu::toString(MenuType menu)
{
    return QLatin1String(ENUM_NAME(ListPopupMenu, MenuType, menu));
}
