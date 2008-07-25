/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "listpopupmenu.h"

// qt/kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kactioncollection.h>

// app includes
#include "umlwidget.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "classifierwidget.h"
#include "classifier.h"
#include "floatingtextwidget.h"
#include "uml.h"
#include "model_utils.h"
#include "widget_utils.h"
#include "folder.h"
#include "umlview.h"
#include "statewidget.h"
#include "signalwidget.h"
#include "activitywidget.h"
#include "preconditionwidget.h"
#include "combinedfragmentwidget.h"
#include "objectnodewidget.h"
#include "forkjoinwidget.h"
#include "objectwidget.h"
#include "category.h"
#include "umlscene.h"

const bool CHECKABLE = true;

//ListPopupMenu for a UMLView (diagram)
ListPopupMenu::ListPopupMenu(QWidget *parent, Menu_Type type, UMLView * view)
        : KMenu(parent)
{
    m_TriggerObject.m_View = view;
    m_TriggerObjectType = tot_View;
    setupMenu(type);
}

//ListPopupMenu for the tree list view
ListPopupMenu::ListPopupMenu(QWidget *parent, Uml::ListView_Type type, UMLObject* object)
        : KMenu(parent)
{
    m_TriggerObject.m_Object = object;
    m_TriggerObjectType = tot_Object;
    Menu_Type mt = mt_Undefined;
    switch(type)
    {
    case Uml::lvt_Logical_View:
        mt = mt_Logical_View;
        break;

    case Uml::lvt_UseCase_View:
        mt = mt_UseCase_View;
        break;

    case Uml::lvt_Component_View:
        mt = mt_Component_View;
        break;

    case Uml::lvt_EntityRelationship_Model:
        mt = mt_EntityRelationship_Model;
        break;

    case Uml::lvt_Deployment_View:
        mt = mt_Deployment_View;
        break;

    case Uml::lvt_Logical_Folder:
        mt = mt_Logical_Folder;
        break;

    case Uml::lvt_UseCase_Folder:
        mt = mt_UseCase_Folder;
        break;

    case Uml::lvt_Component_Folder:
        mt = mt_Component_Folder;
        break;

    case Uml::lvt_Deployment_Folder:
        mt = mt_Deployment_Folder;
        break;

    case Uml::lvt_EntityRelationship_Folder:
        mt = mt_EntityRelationship_Folder;
        break;

    case Uml::lvt_UseCase_Diagram:
        mt = mt_UseCase_Diagram;
        break;

    case Uml::lvt_Class_Diagram:
        mt = mt_Class_Diagram;
        break;

    case Uml::lvt_Collaboration_Diagram:
        mt = mt_Collaboration_Diagram;
        break;

    case Uml::lvt_Sequence_Diagram:
        mt = mt_Sequence_Diagram;
        break;

    case Uml::lvt_State_Diagram:
        mt = mt_State_Diagram;
        break;

    case Uml::lvt_Activity_Diagram:
        mt = mt_Activity_Diagram;
        break;

    case Uml::lvt_Component_Diagram:
        mt = mt_Component_Diagram;
        break;

    case Uml::lvt_Deployment_Diagram:
        mt = mt_Deployment_Diagram;
        break;

    case Uml::lvt_EntityRelationship_Diagram:
        mt = mt_EntityRelationship_Diagram;
        break;

    case Uml::lvt_Actor:
        mt = mt_Actor;
        break;

    case Uml::lvt_UseCase:
        mt = mt_UseCase;
        break;

    case Uml::lvt_Class:
        mt = mt_Class;
        break;

    case Uml::lvt_Package:
        mt = mt_Package;
        break;

    case Uml::lvt_Subsystem:
        mt = mt_Subsystem;
        break;

    case Uml::lvt_Component:
        mt = mt_Component;
        break;

    case Uml::lvt_Node:
        mt = mt_Node;
        break;

    case Uml::lvt_Artifact:
        mt = mt_Artifact;
        break;

    case Uml::lvt_Interface:
        mt = mt_Interface;
        break;

    case Uml::lvt_Enum:
        mt = mt_Enum;
        break;

    case Uml::lvt_EnumLiteral:
        mt = mt_EnumLiteral;
        break;

    case Uml::lvt_Datatype:
        mt = mt_Datatype;
        break;

    case Uml::lvt_Datatype_Folder:
        // let it mt_Undefined
        break;

    case Uml::lvt_Attribute:
        mt = mt_Attribute;
        break;

    case Uml::lvt_Operation:
        mt = mt_Operation;
        break;

    case Uml::lvt_Template:
        mt = mt_Template;
        break;

    case Uml::lvt_Category:
        mt = mt_Category;
        break;

    case Uml::lvt_Entity:
        mt = mt_Entity;
        break;

    case Uml::lvt_EntityAttribute:
        mt = mt_EntityAttribute;
        break;

    case Uml::lvt_UniqueConstraint:
        mt = mt_UniqueConstraint;
        break;

    case Uml::lvt_PrimaryKeyConstraint:
        mt = mt_PrimaryKeyConstraint;
        break;

    case Uml::lvt_ForeignKeyConstraint:
        mt = mt_ForeignKeyConstraint;
        break;

    case Uml::lvt_CheckConstraint:
        mt = mt_CheckConstraint;
        break;

    case Uml::lvt_Model:
        mt = mt_Model;
        break;

    default:
        uWarning() << "unhandled ListView_Type " << type;
        break;
    }
    setupMenu(mt);
}

//ListPopupMenu for a canvas widget
ListPopupMenu::ListPopupMenu(QWidget * parent, NewUMLWidget * object,
                             bool multi, bool unique)
        : KMenu(parent)
{
    m_TriggerObject.m_Widget = object;
    m_TriggerObjectType = tot_Widget;
    //make the right menu for the type
    //make menu for logical view
    if (!object)
        return;
    Uml::Widget_Type type = object->getBaseType();
    // uDebug() << "ListPopupMenu created with multi=" << multi << " , unique="
    //          << unique << " for Widget_Type=" << type;

    if (multi) {
        ClassifierWidget *c = NULL;
        if (unique && (type == Uml::wt_Class || type == Uml::wt_Interface)) {
            c = static_cast<ClassifierWidget *>( object );
            makeMultiClassifierPopup(c);
        }
        insertSubMenuColor(object->getUseFillColour());
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
            if (type == Uml::wt_Interface) {
                insert(mt_DrawAsCircle_Selection, i18n("Draw as Circle"), CHECKABLE);
                setActionChecked(mt_DrawAsCircle_Selection, c->getDrawAsCircle());
                insert(mt_ChangeToClass_Selection, i18n("Change into Class"));
            } else if (type == Uml::wt_Class) {
                UMLClassifier *umlc = c->getClassifier();
                if (umlc->getAbstract() && umlc->attributes() == 0)
                    insert(mt_ChangeToInterface_Selection, i18n("Change into Interface"));
            }
        }

        return;
    }

    switch (type) {
    case Uml::wt_Actor:
    case Uml::wt_UseCase:
        insertSubMenuColor(object->getUseFillColour());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case Uml::wt_Category:
       {
         KMenu* m = makeCategoryTypeMenu(
                        static_cast<UMLCategory*>(object->getUMLObject()));
         m->setTitle(i18n("Category Type"));
         addMenu(m);
         insertSubMenuColor(object->getUseFillColour());
         insertStdItems(true, type);
         insert(mt_Rename);
         insert(mt_Change_Font);
         break;
       }
    case Uml::wt_Class:
    case Uml::wt_Interface:
        makeClassifierPopup(static_cast<ClassifierWidget*>(object));
        break;

    case Uml::wt_Enum:
        insertSubMenuNew(mt_Enum);
        insertSubMenuColor(object->getUseFillColour());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case Uml::wt_Entity:
        insertSubMenuNew(mt_Entity);
        insertSubMenuColor(object->getUseFillColour());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case Uml::wt_Datatype:
    case Uml::wt_Package:
    case Uml::wt_Component:
    case Uml::wt_Node:
    case Uml::wt_Artifact:
        insertSubMenuColor(object->getUseFillColour());
        insertStdItems(false, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case Uml::wt_Object:
        {
            UMLView * pView = static_cast<UMLView *>(parent);
            //Used for sequence diagram and collaboration diagram widgets
            insertSubMenuColor( object->getUseFillColour() );
            // [PORT]
            if( pView->umlScene()->getType() == Uml::dt_Sequence ) {
                addSeparator();
                Menu_Type tabUp = mt_Up;
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

    case Uml::wt_Message:
        insertStdItems(false, type);
        //insert(mt_Change_Font);
        //insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
        //insert(mt_Select_Operation, i18n("Select Operation..."));
        break;

    case Uml::wt_Note:
        insertSubMenuColor(object->getUseFillColour());
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

    case Uml::wt_Box:
        insertStdItems(false, type);
        insert(mt_Line_Color);
        break;

    case Uml::wt_State:
        {
            StateWidget* pState = static_cast< StateWidget *>( object );
            if (pState->stateType() == StateWidget::Normal) {
                insertSubMenuNew(mt_New_Activity);
            }
            insertSubMenuColor( object->getUseFillColour() );
            insertStdItems(false, type);
            if (pState->stateType() == StateWidget::Normal) {
                insert(mt_Rename, i18n("Change State Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case Uml::wt_ForkJoin:
        {
            ForkJoinWidget *pForkJoin = static_cast<ForkJoinWidget*>(object);
            if (pForkJoin->orientation() == Qt::Vertical) {
                insert(mt_Flip, i18n("Flip Horizontal"));
			}
            else {
                insert(mt_Flip, i18n("Flip Vertical"));
			}
			m_actions[mt_Fill_Color] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill),
												 i18n("Fill Color..."));
        }
        break;

    case Uml::wt_Activity:
        {
            ActivityWidget* pActivity = static_cast<ActivityWidget *>(object);
            if( pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insertSubMenuColor( object->getUseFillColour() );
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

    case Uml::wt_ObjectNode:
        {
            ObjectNodeWidget* objWidget = static_cast<ObjectNodeWidget *>(object);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
              || objWidget->objectNodeType() == ObjectNodeWidget::Data
              || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insertSubMenuColor( object->getUseFillColour() );
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

    case Uml::wt_Pin:
    case Uml::wt_Signal:
    case Uml::wt_FloatingDashLine:
    case Uml::wt_Precondition:
        insertSubMenuColor(object->getUseFillColour());
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

    case Uml::wt_CombinedFragment:
        // for alternative and parallel combined fragments
        if ((static_cast<CombinedFragmentWidget*>(object))->getCombinedFragmentType() == CombinedFragmentWidget::Alt ||
            (static_cast<CombinedFragmentWidget*>(object))->getCombinedFragmentType() == CombinedFragmentWidget::Par) {
            insert(mt_AddInteractionOperand, i18n("Add Interaction Operand"));
            addSeparator();
        }
        insertSubMenuColor(object->getUseFillColour());
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

    case Uml::wt_Text:
        switch( (static_cast<FloatingTextWidget*>(object))->getRole() ) {
        case Uml::tr_MultiB:
            insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiB);
            break;
        case Uml::tr_MultiA:
            insertAssocItem(i18n("Change Multiplicity..."), mt_Rename_MultiA);
            break;
        case Uml::tr_Name:
            insertAssocItem(i18n("Change Name"), mt_Rename_Name);
            break;
        case Uml::tr_RoleAName:
            insertAssocItem(i18n("Change Role A Name..."), mt_Rename_RoleAName);
            break;
        case Uml::tr_RoleBName:
            insertAssocItem(i18n("Change Role B Name..."), mt_Rename_RoleBName);
            break;
        case Uml::tr_ChangeA:
        case Uml::tr_ChangeB:
            insert(mt_Change_Font);
            insert(mt_Reset_Label_Positions);
            insert(mt_Properties);
            break;

        case Uml::tr_Coll_Message_Self:
        case Uml::tr_Coll_Message:
        case Uml::tr_Seq_Message_Self:
        case Uml::tr_Seq_Message:
            insert(mt_Change_Font);
            insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
            insert(mt_Select_Operation, i18n("Select Operation..."));
            break;

        case Uml::tr_Floating:
        default:
            insertStdItems(false, type);
            insert(mt_Rename, i18n("Change Text..."));
            insert(mt_Change_Font);
            break;
        }
        break;
    default:
        uWarning() << "unhandled Widget_Type " << type;
        break;
    }//end switch

    bool bCutState = UMLApp::app()->getCutCopyState();
    setActionEnabled( mt_Cut, bCutState );
    setActionEnabled( mt_Copy, bCutState );
    setActionEnabled( mt_Paste, false );
}

ListPopupMenu::~ListPopupMenu()
{
    foreach (QAction* action, m_actions) {
        delete action;
    }
    m_actions.clear();
}

void ListPopupMenu::insert(Menu_Type m)
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
    case mt_Import_Classes:
        m_actions[m] = addAction(Icon_Utils::BarIcon(Icon_Utils::it_Import_Class), i18n("Import Classes..."));
        break;
    case mt_Import_Project:
        m_actions[m] = addAction(Icon_Utils::BarIcon(Icon_Utils::it_Import_Project), i18n("Import Project..."));
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
        uWarning() << "called on unimplemented Menu_Type " << m;
        break;
    }
}

void ListPopupMenu::insert(Menu_Type m, KMenu* menu)
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
        uWarning() << "called on unimplemented Menu_Type " << m;
        break;
    }
}

void ListPopupMenu::insert(const Menu_Type m, const QIcon & icon, const QString & text)
{
    m_actions[m] = addAction(icon, text);
}

void ListPopupMenu::insert(const Menu_Type m, const QString & text, const bool checkable)
{
    m_actions[m] = addAction(text);
    if (checkable) {
        QAction* action = getAction(m);
        if (action)
            action->setCheckable(checkable);
    }
}

void ListPopupMenu::insert(const Menu_Type m, KMenu* menu, const QIcon & icon, const QString & text)
{
    m_actions[m] = menu->addAction(icon, text);
}

void ListPopupMenu::insert(const Menu_Type m, KMenu* menu, const QString & text, const bool checkable)
{
    m_actions[m] = menu->addAction(text);
    if (checkable) {
        QAction* action = getAction(m);
        if (action)
            action->setCheckable(checkable);
    }
}

void ListPopupMenu::insertStdItems(bool insertLeadingSeparator /* = true */,
                                   Uml::Widget_Type type /* = wt_NewUMLRectWidget */)
{
    if (insertLeadingSeparator)
        addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    if (type == Uml::wt_NewUMLRectWidget)
        insert(mt_Rename);
    else if (Model_Utils::isCloneable(type))
        insert(mt_Clone);
    insert(mt_Delete);
}

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

void ListPopupMenu::insertAssocItem(const QString &label, Menu_Type mt)
{
    insert(mt, label);
    insert(mt_Change_Font);
    insert(mt_Reset_Label_Positions);
    insert(mt_Properties);
}

void ListPopupMenu::insertSubmodelAction()
{
    const Settings::OptionState& ostat = Settings::getOptionState();
    if (ostat.generalState.tabdiagrams) {
        // Umbrello currently does not support External Folders
        // in combination with Tabbed Diagrams.
        // If you need external folders then disable the tabbed diagrams
        // in the General Settings.
        return;
    }
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem *current = static_cast<UMLListViewItem*>(listView->currentItem());
    UMLObject *o = current->getUMLObject();
    if (o == NULL) {
        uError() << current->getText() << " getUMLObject()  returns NULL";
        return;
    }
    UMLFolder *f = dynamic_cast<UMLFolder*>(o);
    if (f == NULL) {
        uError() << "current->getUMLObject (" << o->getName() << ") is not a Folder";
        return;
    }
    QString submodelFile = f->getFolderFile();
    if (submodelFile.isEmpty())
        insert(mt_Externalize_Folder);
    else
        insert(mt_Internalize_Folder);
}

void ListPopupMenu::makeMultiClassifierPopup(ClassifierWidget *c)
{
    Uml::Widget_Type type = c->getBaseType();
    ClassifierWidget *cls = NULL;

    KMenu* show = new KMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));
    if (type == Uml::wt_Class) {
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
    bool sig = (c->getShowOpSigs() == Uml::st_SigNoVis ||
                c->getShowOpSigs() == Uml::st_ShowSig);
    setActionChecked(mt_Show_Operation_Signature_Selection, sig);
    if (type == Uml::wt_Class) {
        insert(mt_Show_Attribute_Signature_Selection, show, i18n("Attribute Signature"), CHECKABLE);
        sig = (cls->getShowAttSigs() == Uml::st_SigNoVis ||
               cls->getShowAttSigs() == Uml::st_ShowSig);
        setActionChecked(mt_Show_Attribute_Signature_Selection, sig);
    }
    insert(mt_Show_Packages_Selection, show, i18n("Package"), CHECKABLE);
    setActionChecked(mt_Show_Packages_Selection, c->getShowPackage());
    if (type == Uml::wt_Class) {
        insert(mt_Show_Stereotypes_Selection, show, i18n("Stereotype"), CHECKABLE);
        setActionChecked(mt_Show_Stereotypes_Selection, cls->getShowStereotype());
    }
    addMenu(show);
}

void ListPopupMenu::makeClassifierPopup(ClassifierWidget *c)
{
    Uml::Widget_Type type = c->getBaseType();
    KMenu* menu = new KMenu(i18nc("new classifier menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    if (type == Uml::wt_Class)
        insert(mt_Attribute, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Attribute), i18n("Attribute..."));
    insert(mt_Operation, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method), i18n("Operation..."));
    insert(mt_Template, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n( "Template..." ));
    addMenu(menu);

    makeMultiClassifierPopup(c);

    insertSubMenuColor(c->getUseFillColour());
    insertStdItems(true, type);
    insert(mt_Rename);
    insert(mt_Change_Font);
    if (type == Uml::wt_Interface) {
        insert(mt_DrawAsCircle, i18n("Draw as Circle"), CHECKABLE);
        setActionChecked(mt_DrawAsCircle, c->getDrawAsCircle());
        insert(mt_ChangeToClass, i18n("Change into Class"));
    } else {
        insert(mt_Refactoring, Icon_Utils::SmallIcon(Icon_Utils::it_Refactor), i18n("Refactor"));
        insert(mt_ViewCode, Icon_Utils::SmallIcon(Icon_Utils::it_View_Code), i18n("View Code"));
        UMLClassifier *umlc = c->getClassifier();
        if (umlc->getAbstract() && umlc->attributes() == 0)
            insert(mt_ChangeToInterface, i18n("Change into Interface"));
    }
    insert(mt_Properties);
}

void ListPopupMenu::insertSubMenuColor(bool fc)
{
    KMenu* color = new KMenu(i18nc("color menu", "Color"), this);
    insert(mt_Line_Color, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Line), i18n("Line Color..."));
    insert(mt_Fill_Color, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill), i18n("Fill Color..."));
    insert(mt_Use_Fill_Color, color, i18n("Use Fill Color"), CHECKABLE);
    setActionChecked(mt_Use_Fill_Color, fc);
    addMenu(color);
}

Uml::Diagram_Type ListPopupMenu::convert_MT_DT(Menu_Type mt)
{
    Uml::Diagram_Type type =  Uml::dt_Undefined;

    switch (mt) {
    case mt_UseCase_Diagram:
        type = Uml::dt_UseCase;
        break;
    case mt_Class_Diagram:
        type = Uml::dt_Class;
        break;
    case mt_Sequence_Diagram:
        type = Uml::dt_Sequence;
        break;
    case mt_Collaboration_Diagram:
        type = Uml::dt_Collaboration;
        break;
    case mt_State_Diagram:
        type = Uml::dt_State;
        break;
    case mt_Activity_Diagram:
        type = Uml::dt_Activity;
        break;
    case mt_Component_Diagram:
        type = Uml::dt_Component;
        break;
    case mt_Deployment_Diagram:
        type = Uml::dt_Deployment;
        break;
    case mt_EntityRelationship_Diagram:
        type = Uml::dt_EntityRelationship;
        break;
    default:
        break;
    }
    return type;
}

Uml::Object_Type ListPopupMenu::convert_MT_OT(Menu_Type mt)
{
    Uml::Object_Type type =  Uml::ot_UMLObject;

    switch (mt) {
    case mt_UseCase:
        type = Uml::ot_UseCase;
        break;
    case mt_Actor:
        type = Uml::ot_Actor;
        break;
    case mt_Class:
        type = Uml::ot_Class;
        break;
    case mt_Attribute:
        type = Uml::ot_Attribute;
        break;
    case mt_Template:
        type = Uml::ot_Template;
        break;
    case mt_EnumLiteral:
        type = Uml::ot_EnumLiteral;
        break;
    case mt_EntityAttribute:
        type = Uml::ot_EntityAttribute;
        break;
    case mt_Operation:
        type = Uml::ot_Operation;
        break;
    case mt_Category:
        type = Uml::ot_Category;
        break;
    default:
        break;
    }
    return type;
}

void ListPopupMenu::insertSubMenuNew(Menu_Type type)
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

void ListPopupMenu::setupMenu(Menu_Type type)
{
    // uDebug() << "ListPopupMenu created for Menu_Type=" << type;

    switch (type) {
    case mt_Logical_View:
        insertContainerItems(true);
        addSeparator();
        insert(mt_Paste);
        addSeparator();
        insert(mt_Import_Classes);
        insert(mt_Import_Project);
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
        insert(mt_Import_Classes);
        insert(mt_Import_Project);
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
        bool bCutState = UMLApp::app()->getCutCopyState();
        setActionEnabled( mt_Undo, UMLApp::app()->getUndoEnabled() );
        setActionEnabled( mt_Redo, UMLApp::app()->getRedoEnabled() );
        setActionEnabled( mt_Cut, bCutState );
        setActionEnabled( mt_Copy, bCutState );
        setActionEnabled( mt_Paste, UMLApp::app()->getPasteState() );
    }

}

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
    setActionChecked(mt_SnapToGrid, view->umlScene()->getSnapToGrid() );
    insert(mt_ShowSnapGrid, i18n("Show Grid"), CHECKABLE);
    setActionChecked(mt_ShowSnapGrid, view->umlScene()->getShowSnapGrid() );
    insert(mt_Properties);
}

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

QAction* ListPopupMenu::getAction(Menu_Type idx)
{
    return m_actions.value(idx, NULL);
}

// ListPopupMenu::Menu_Type ListPopupMenu::getMenuType(KAction* action)
// {
//     return m_actions.key(action);
// }

ListPopupMenu::Menu_Type ListPopupMenu::getMenuType(QAction* action)
{
    QList<Menu_Type> keyList = m_actions.keys( action );
    if ( keyList.empty() || /* all key-value pairs are unique*/ keyList.count() > 1 ) {
        return mt_Undefined;
    } else {
        // we return the first ( only ) value
        return keyList.first();
    }
}

void ListPopupMenu::setActionChecked(Menu_Type idx, bool value)
{
    QAction* action = getAction(idx);
    if (action && action->isCheckable())
        action->setChecked(value);
#ifdef VERBOSE_DEBUGGING
    else
        uWarning() << "called on unknown Menu_Type " << idx;
#endif
}

void ListPopupMenu::setActionEnabled(Menu_Type idx, bool value)
{
    QAction* action = getAction(idx);
    if (action)
        action->setEnabled(value);
#ifdef VERBOSE_DEBUGGING
    else
        uWarning() << "called on unknown Menu_Type " << idx;
#endif
}

#include "listpopupmenu.moc"
