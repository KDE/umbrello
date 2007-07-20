/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "listpopupmenu.h"

// qt/kde includes
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

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
#include "activitywidget.h"
#include "forkjoinwidget.h"
#include "objectwidget.h"

//ListPopupMenu for a UMLView (diagram)
ListPopupMenu::ListPopupMenu(QWidget *parent, Menu_Type type, UMLView * view)
        : KPopupMenu(parent) {
    init();
    setupMenu(type, view);
}

//ListPopupMenu for the tree list view
ListPopupMenu::ListPopupMenu(QWidget *parent, Uml::ListView_Type type)
        : KPopupMenu(parent) {
    init();
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

    case Uml::lvt_Attribute:
        mt = mt_Attribute;
        break;

    case Uml::lvt_Operation:
        mt = mt_Operation;
        break;

    case Uml::lvt_Template:
        mt = mt_Template;
        break;

    case Uml::lvt_Entity:
        mt = mt_Entity;
        break;

    case Uml::lvt_EntityAttribute:
        mt = mt_EntityAttribute;
        break;

    case Uml::lvt_Model:
        mt = mt_Model;
        break;

    default:   ;
        //break;
    }
    setupMenu(mt);
}

//ListPopupMenu for a canvas widget
ListPopupMenu::ListPopupMenu(QWidget * parent, UMLWidget * object,
                             bool multi, bool unique)
        : KPopupMenu(parent)
{
    init();
    //make the right menu for the type
    //make menu for logical view
    if(!object)
        return;
    Uml::Widget_Type type = object -> getBaseType();

    if(multi) {
        ClassifierWidget *c = NULL;
        if (unique && (type == Uml::wt_Class || type == Uml::wt_Interface)) {
            c = static_cast<ClassifierWidget *>( object );
            makeMultiClassifierPopup(c);
        }
        setupColorSelection(object -> getUseFillColour());
        insertSeparator();
        insertStdItem(mt_Cut);
        insertStdItem(mt_Copy);
        insertStdItem(mt_Paste);
        insertSeparator();
        insertItem(SmallIcon( "fonts" ), i18n( "Change Font..." ),
                   mt_Change_Font_Selection );
        insertItem(SmallIcon( "editdelete" ), i18n("Delete Selected Items"),
                   mt_Delete_Selection);

        // add this here and not above with the other stuff of the interface
        // user might expect it at this position of the context menu
        if (unique) {
            if (type == Uml::wt_Interface) {
                insertItem(i18n("Draw as Circle"), mt_DrawAsCircle_Selection);
                setItemChecked( mt_DrawAsCircle_Selection,
                                c->getDrawAsCircle() );
                insertItem(i18n("Change into Class"), mt_ChangeToClass_Selection);
            } else if (type == Uml::wt_Class) {
                UMLClassifier *umlc = c->getClassifier();
                if (umlc->getAbstract() && umlc->attributes() == 0)
                    insertItem(i18n("Change into Interface"), mt_ChangeToInterface_Selection);
            }
        }

        if(m_pInsert)
            connect(m_pInsert, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
        if(m_pShow)
            connect(m_pShow, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
        if(m_pColor)
            connect(m_pColor, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
        return;
    }

    StateWidget *pState;
    ActivityWidget *pActivity;
    UMLView * pView = static_cast<UMLView *>( parent );

    switch(type) {
    case Uml::wt_Actor:
    case Uml::wt_UseCase:
        setupColor(object -> getUseFillColour());
        insertStdItems(true, type);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Properties);
        break;

    case Uml::wt_Class:
    case Uml::wt_Interface:
        makeClassifierPopup(static_cast<ClassifierWidget*>(object));
        break;

    case Uml::wt_Enum:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(SmallIcon("source"), i18n("Enum Literal..."), mt_EnumLiteral);
        insertFileNew();
        setupColor(object->getUseFillColour());
        insertStdItems(true, type);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Properties);
        break;

    case Uml::wt_Entity:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(SmallIcon("source"), i18n("Entity Attribute..."), mt_EntityAttribute);
        insertFileNew();
        setupColor(object->getUseFillColour());
        insertStdItems(true, type);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Properties);
        break;

    case Uml::wt_Datatype:
    case Uml::wt_Package:
    case Uml::wt_Component:
    case Uml::wt_Node:
    case Uml::wt_Artifact:
        setupColor(object->getUseFillColour());
        insertStdItems(false, type);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Properties);
        break;

    case Uml::wt_Object:
        //Used for sequence diagram and collaboration diagram widgets
        setupColor( object->getUseFillColour() );
        if( pView->getType() == Uml::dt_Sequence ) {
            insertSeparator();
            int tabUp = insertItem( SmallIcon( "1uparrow"), i18n("Move Up"), mt_Up);
            insertItem( SmallIcon( "1downarrow"), i18n("Move Down"), mt_Down);
            if ( !(static_cast<ObjectWidget*>(object))->canTabUp() ) {
                setItemEnabled(tabUp, false);
            }
        }
        insertStdItems(true, type);
        insertItem(i18n("Rename Class..."), mt_Rename);
        insertItem(i18n("Rename Object..."), mt_Rename_Object);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Properties);
        break;

    case Uml::wt_Message:
        insertStdItems(false, type);
        insertStdItem(mt_Change_Font);
        insertItem(SmallIcon( "filenew"), i18n("New Operation..."), mt_Operation);
        insertItem(i18n("Select Operation..."), mt_Select_Operation);
        break;

    case Uml::wt_Note:
        setupColor(object -> getUseFillColour());
        insertSeparator();
        insertStdItem(mt_Cut);
        insertStdItem(mt_Copy);
        insertStdItem(mt_Paste);
        insertItem(SmallIcon( "editdelete"), i18n("Clear"), mt_Clear);
        insertSeparator();
        insertItem(i18n("Change Text..."), mt_Rename);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Change_Font);
        break;

    case Uml::wt_Box:
        insertStdItems(false, type);
        insertStdItem(mt_Line_Color);
        break;

    case Uml::wt_State:
        pState = static_cast< StateWidget *>( object );
        if( pState -> getStateType() == StateWidget::Normal ) {
            m_pInsert = new KPopupMenu(this);
            m_pInsert -> insertItem(SmallIcon( "filenew"), i18n("Activity..."), mt_New_Activity);
            insertFileNew();
        }
        setupColor( object -> getUseFillColour() );
        insertStdItems(false, type);
        if( pState -> getStateType() == StateWidget::Normal ) {
            insertItem(i18n("Change State Name..."), mt_Rename);
            insertStdItem(mt_Change_Font);
            insertStdItem(mt_Properties);
        }
        break;

    case Uml::wt_ForkJoin:
        {
            ForkJoinWidget *pForkJoin = static_cast<ForkJoinWidget*>(object);
            if (pForkJoin->getDrawVertical())
                insertItem(i18n("Flip Horizontal"), mt_Flip);
            else
                insertItem(i18n("Flip Vertical"), mt_Flip);
        }
        break;

    case Uml::wt_Activity:
        pActivity = static_cast<ActivityWidget *>( object );
        if( pActivity -> getActivityType() == ActivityWidget::Normal )
            setupColor( object -> getUseFillColour() );
        insertStdItems(false, type);
        if( pActivity -> getActivityType() == ActivityWidget::Normal ) {
            insertItem(i18n("Change Activity Name..."), mt_Rename);
            insertStdItem(mt_Change_Font);
            insertStdItem(mt_Properties);
        }
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
            insertStdItem(mt_Change_Font);
            insertStdItem(mt_Reset_Label_Positions);
            insertStdItem(mt_Properties);
            break;

        case Uml::tr_Coll_Message_Self:
        case Uml::tr_Coll_Message:
        case Uml::tr_Seq_Message_Self:
        case Uml::tr_Seq_Message:
            insertStdItem(mt_Change_Font);
            insertItem(SmallIcon( "filenew"), i18n("New Operation..."), mt_Operation);
            insertItem(i18n("Select Operation..."), mt_Select_Operation);
            break;

        case Uml::tr_Floating:
        default:
            insertStdItems(false, type);
            insertItem(i18n("Change Text..."), mt_Rename);
            insertStdItem(mt_Change_Font);
            break;
        }
        break;
    default:
        break;
    }//end switch

    if(m_pInsert)
        connect(m_pInsert, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
    if(m_pShow)
        connect(m_pShow, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
    if(m_pColor)
        connect(m_pColor, SIGNAL(activated(int)), this, SIGNAL(activated(int)));

    bool bCutState = UMLApp::app() -> getCutCopyState();
    setItemEnabled( mt_Cut, bCutState );
    setItemEnabled( mt_Copy, bCutState );
    setItemEnabled( mt_Paste, false );
}

ListPopupMenu::~ListPopupMenu() {}

void ListPopupMenu::init() {
    m_pInsert = 0;
    m_pShow = 0;
    m_pColor = 0;
}

void ListPopupMenu::insertFileNew() {
    insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
}

void ListPopupMenu::insertStdItem(Menu_Type m)
{
    switch (m) {
    case mt_Properties:
        insertItem(SmallIcon("info"), i18n("Properties"), mt_Properties);
        break;
    case mt_Rename:
        insertItem(i18n("Rename..."), mt_Rename);
        break;
    case mt_Delete:
        insertItem(SmallIcon("editdelete"), i18n("Delete"), mt_Delete);
        break;
    case mt_Cut:
        insertItem(SmallIcon("editcut"), i18n("Cut"), mt_Cut);
        break;
    case mt_Copy:
        insertItem(SmallIcon("editcopy"), i18n("Copy"), mt_Copy);
        break;
    case mt_Paste:
        insertItem(SmallIcon("editpaste"), i18n("Paste"), mt_Paste);
        break;
    case mt_Change_Font:
        insertItem(SmallIcon("fonts"), i18n("Change Font..."), mt_Change_Font);
        break;
    case mt_Line_Color:
        insertItem(SmallIcon("color_line"), i18n("Line Color..."), mt_Line_Color);
        break;
    case mt_Expand_All:
        insertItem(i18n("Expand All"), mt_Expand_All);
        break;
    case mt_Collapse_All:
        insertItem(i18n("Collapse All"), mt_Collapse_All);
        break;
    case mt_Clone:
        insertItem(i18n("Duplicate"), mt_Clone);
        break;
    case mt_Externalize_Folder:
        insertItem(i18n("Externalize Folder..."), mt_Externalize_Folder);
        break;
    case mt_Internalize_Folder:
        insertItem(i18n("Internalize Folder"), mt_Internalize_Folder);
        break;
    case mt_Import_Classes:
        insertItem(BarIcon("source_cpp"), i18n("Import Classes..."), mt_Import_Classes);
        break;
    case mt_Package:
        m_pInsert->insertItem(m_pixmap[pm_Package], i18n("Package"), mt_Package);
    case mt_Subsystem:
        m_pInsert->insertItem(m_pixmap[pm_Subsystem], i18n("Subsystem"), mt_Subsystem);
        break;
    case mt_Component:
        m_pInsert->insertItem(m_pixmap[pm_Component], i18n("Component"), mt_Component);
        break;
    case mt_Artifact:
        m_pInsert->insertItem(m_pixmap[pm_Artifact], i18n("Artifact"), mt_Artifact);
        break;
    case mt_Component_Diagram:
        m_pInsert->insertItem(BarIcon("umbrello_diagram_component"), i18n("Component Diagram..."),
                              mt_Component_Diagram);
        break;
    case mt_Node:
        m_pInsert->insertItem(m_pixmap[pm_Node], i18n("Node"), mt_Node);
        break;
    case mt_Deployment_Diagram:
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_Deployment), i18n("Deployment Diagram..."),
                              mt_Deployment_Diagram);
        break;
    case mt_Deployment_Folder:
    case mt_Component_Folder:
    case mt_UseCase_Folder:
    case mt_EntityRelationship_Folder:
        m_pInsert->insertItem(BarIcon("folder_new"), i18n("Folder"), m);
        break;
    case mt_Entity:
        m_pInsert->insertItem(m_pixmap[pm_Entity], i18n("Entity"), mt_Entity);
        break;
    case mt_EntityRelationship_Diagram:
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_EntityRelationship), i18n("Entity Relationship Diagram..."),
                              mt_EntityRelationship_Diagram);
        break;
    case mt_Actor:
        m_pInsert->insertItem(m_pixmap[pm_Actor], i18n("Actor"), mt_Actor);
        break;
    case mt_UseCase:
        m_pInsert->insertItem(m_pixmap[pm_Usecase], i18n("Use Case"), mt_UseCase);
        break;
    case mt_UseCase_Diagram:
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_UseCase), i18n("Use Case Diagram..."),
                              mt_UseCase_Diagram);
        break;
    case mt_FloatText:
        m_pInsert->insertItem(m_pixmap[pm_Text], i18n("Text Line..." ), mt_FloatText);
        break;
    case mt_Reset_Label_Positions:
        insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
        break;
    case mt_New_Parameter:
        insertItem(SmallIcon("source"), i18n("New Parameter..."), mt_New_Parameter);
        break;
    case mt_New_Operation:
        insertItem(SmallIcon("CVpublic_meth"),i18n("New Operation..."), mt_New_Operation);
        break;
    case mt_New_Attribute:
        insertItem(SmallIcon("CVpublic_var"), i18n("New Attribute..."), mt_New_Attribute);
        break;
    case mt_New_Template:
        insertItem(SmallIcon("source"), i18n("New Template..."), mt_New_Template);
        break;
    case mt_New_EnumLiteral:
        insertItem(SmallIcon("source"), i18n("New Literal..."), mt_New_EnumLiteral);
        break;
    case mt_New_EntityAttribute:
        insertItem(SmallIcon("source"), i18n("New Entity Attribute..."), mt_New_EntityAttribute);
        break;
    case mt_New_Activity:
        m_pInsert->insertItem(SmallIcon("source"), i18n("Activity..."), mt_New_Activity);
        break;
    case mt_Export_Image:
        insertItem(SmallIcon("image"), i18n("Export as Picture..."), mt_Export_Image);
        break;
    default:
        kWarning() << "ListPopupMenu::insertStdItem called on unimplemented Menu_Type " << m << endl;
        break;
    }
}

void ListPopupMenu::insertStdItems(bool insertLeadingSeparator /* = true */,
                                   Uml::Widget_Type type /* = wt_UMLWidget */)
{
    if (insertLeadingSeparator)
        insertSeparator();
    insertStdItem(mt_Cut);
    insertStdItem(mt_Copy);
    insertStdItem(mt_Paste);
    insertSeparator();
    if (type == Uml::wt_UMLWidget)
        insertStdItem(mt_Rename);
    else if (Model_Utils::isCloneable(type))
        insertStdItem(mt_Clone);
    insertStdItem(mt_Delete);
}

void ListPopupMenu::insertContainerItems(bool folderAndDiagrams) {
    if (folderAndDiagrams)
        m_pInsert -> insertItem(BarIcon("folder_new"), i18n("Folder"), mt_Logical_Folder);
    m_pInsert -> insertItem(m_pixmap[pm_Class], i18n("Class"), mt_Class);
    m_pInsert -> insertItem(m_pixmap[pm_Interface], i18n("Interface"), mt_Interface);
    m_pInsert -> insertItem(m_pixmap[pm_Datatype], i18n("Datatype"), mt_Datatype);
    m_pInsert -> insertItem(m_pixmap[pm_Enum], i18n("Enum"), mt_Enum);
    insertStdItem(mt_Package);
    if (folderAndDiagrams) {
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_Class), i18n("Class Diagram..."), mt_Class_Diagram);
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_State), i18n("State Diagram..."), mt_State_Diagram);
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_Activity), i18n("Activity Diagram..."), mt_Activity_Diagram);
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_Sequence), i18n("Sequence Diagram..."), mt_Sequence_Diagram);
        m_pInsert->insertItem(Widget_Utils::iconSet(Uml::dt_Collaboration), i18n("Collaboration Diagram..."), mt_Collaboration_Diagram);
    }
    insertFileNew();
}

void ListPopupMenu::insertAssocItem(const QString &label, Menu_Type mt) {
    insertItem(label, mt);
    insertStdItem(mt_Change_Font);
    insertStdItem(mt_Reset_Label_Positions);
    insertStdItem(mt_Properties);
}

void ListPopupMenu::insertSubmodelAction() {
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
        kError() << "ListPopupMenu::insertSubmodelAction: "
            << current->getText() << " getUMLObject()  returns NULL" << endl;
        return;
    }
    UMLFolder *f = dynamic_cast<UMLFolder*>(o);
    if (f == NULL) {
        kError() << "ListPopupMenu::insertSubmodelAction: "
            << "current->getUMLObject (" << o->getName() << ") is not a Folder" << endl;
        return;
    }
    QString submodelFile = f->getFolderFile();
    if (submodelFile.isEmpty())
        insertStdItem(mt_Externalize_Folder);
    else
        insertStdItem(mt_Internalize_Folder);
}

void ListPopupMenu::makeMultiClassifierPopup(ClassifierWidget *c)
{
    Uml::Widget_Type type = c->getBaseType();
    ClassifierWidget *cls = NULL;

    m_pShow = new KPopupMenu(this);
    m_pShow->setCheckable(true);
    if (type == Uml::wt_Class) {
        cls = static_cast<ClassifierWidget*>(c);
        m_pShow->insertItem( i18n("Attributes"), mt_Show_Attributes_Selection);
        m_pShow->setItemChecked(mt_Show_Attributes_Selection,
                                cls->getShowAtts());
    }
    m_pShow->insertItem(i18n("Operations"), mt_Show_Operations_Selection);
    m_pShow->setItemChecked(mt_Show_Operations_Selection, c->getShowOps());
    m_pShow->insertItem(i18n("Public Only"), mt_Show_Public_Only_Selection);
    m_pShow->setItemChecked(mt_Show_Public_Only_Selection, c->getShowPublicOnly());
    m_pShow->insertItem(i18n("Visibility"), mt_Visibility_Selection);
    m_pShow->setItemChecked(mt_Visibility_Selection, c->getShowVisibility());
    m_pShow->insertItem(i18n("Operation Signature"),
                        mt_Show_Operation_Signature_Selection);
    bool sig = (c->getShowOpSigs() == Uml::st_SigNoVis ||
                c->getShowOpSigs() == Uml::st_ShowSig);
    m_pShow->setItemChecked(mt_Show_Operation_Signature_Selection, sig);
    if (type == Uml::wt_Class) {
        m_pShow->insertItem(i18n("Attribute Signature"),
                            mt_Show_Attribute_Signature_Selection);
        sig = (cls->getShowAttSigs() == Uml::st_SigNoVis ||
               cls->getShowAttSigs() == Uml::st_ShowSig);
        m_pShow->setItemChecked(mt_Show_Attribute_Signature_Selection, sig);
    }
    m_pShow->insertItem(i18n("Package"), mt_Show_Packages_Selection);
    m_pShow->setItemChecked(mt_Show_Packages_Selection, c->getShowPackage());
    if (type == Uml::wt_Class) {
        m_pShow->insertItem(i18n("Stereotype"), mt_Show_Stereotypes_Selection);
        m_pShow->setItemChecked(mt_Show_Stereotypes_Selection,
                                cls->getShowStereotype());
    }
    insertItem(i18n("Show"), m_pShow);
}

void ListPopupMenu::makeClassifierPopup(ClassifierWidget *c)
{
    Uml::Widget_Type type = c->getBaseType();
    m_pInsert = new KPopupMenu(this);
    if (type == Uml::wt_Class)
        m_pInsert->insertItem(SmallIcon( "CVpublic_var" ), i18n("Attribute..."), mt_Attribute);
    m_pInsert->insertItem( SmallIcon( "CVpublic_meth"), i18n("Operation..."), mt_Operation);
    insertFileNew();

    makeMultiClassifierPopup(c);

    setupColor(c->getUseFillColour());
    insertStdItems(true, type);
    insertStdItem(mt_Rename);
    insertStdItem(mt_Change_Font);
    if (type == Uml::wt_Interface) {
        insertItem(i18n("Draw as Circle"), mt_DrawAsCircle);
        setItemChecked( mt_DrawAsCircle, c->getDrawAsCircle() );
        insertItem(i18n("Change into Class"), mt_ChangeToClass);
    } else {
        insertItem(i18n("Refactor"), mt_Refactoring);
        insertItem(i18n("View Code"), mt_ViewCode);
        UMLClassifier *umlc = c->getClassifier();
        if (umlc->getAbstract() && umlc->attributes() == 0)
            insertItem(i18n("Change into Interface"), mt_ChangeToInterface);
    }
    insertStdItem(mt_Properties);
}

void ListPopupMenu::setupColor(bool fc)
{
    m_pColor = new KPopupMenu(this);
    m_pColor -> insertItem(SmallIcon( "color_line"), i18n("Line Color..."), mt_Line_Color);
    m_pColor -> insertItem(SmallIcon( "color_fill"), i18n("Fill Color..."), mt_Fill_Color);
    m_pColor -> insertItem( i18n("Use Fill Color"), mt_Use_Fill_Color);

    m_pColor -> setItemChecked(mt_Use_Fill_Color, fc);
    insertItem(SmallIcon( "colorize"), i18n("Color"), m_pColor);
}

void ListPopupMenu::setupColorSelection(bool fc)
{
    m_pColor = new KPopupMenu(this);
    m_pColor -> insertItem(SmallIcon( "color_line"), i18n("Line Color..."), mt_Line_Color_Selection);
    m_pColor -> insertItem(SmallIcon( "color_fill"), i18n("Fill Color..."), mt_Fill_Color_Selection);
    m_pColor -> insertItem( i18n("Use Fill Color"), mt_Use_Fill_Color);

    m_pColor -> setItemChecked(mt_Use_Fill_Color, fc);
    insertItem(SmallIcon( "colorize"), i18n("Color"), m_pColor);
}

Uml::Diagram_Type ListPopupMenu::convert_MT_DT(Menu_Type mt) {
    Uml::Diagram_Type type =  Uml::dt_Undefined;

    switch(mt) {
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

Uml::Object_Type ListPopupMenu::convert_MT_OT(Menu_Type mt) {
    Uml::Object_Type type =  Uml::ot_UMLObject;

    switch(mt) {
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
    case mt_EnumLiteral:
        type = Uml::ot_EnumLiteral;
        break;
    case mt_EntityAttribute:
        type = Uml::ot_EntityAttribute;
        break;
    case mt_Operation:
        type = Uml::ot_Operation;
        break;
    default:
        break;
    }
    return type;
}

void ListPopupMenu::setupMenu(Menu_Type type, UMLView* view) {
    //make the right menu for the type
    //make menu for logical view
    m_pInsert = 0;

    m_pShow = 0;
    m_pColor = 0;

    KStandardDirs* dirs = KGlobal::dirs();
    QString dataDir = dirs->findResourceDir("data", "umbrello/pics/object.png");
    dataDir += "/umbrello/pics/";
    m_pixmap[pm_Class]       .load(dataDir+"class.png",         "PNG");
    m_pixmap[pm_Package]     .load(dataDir+"package.png",       "PNG");
    m_pixmap[pm_Interface]   .load(dataDir+"interface.png",     "PNG");
    m_pixmap[pm_Datatype]    .load(dataDir+"datatype.png",      "PNG");
    m_pixmap[pm_Enum]        .load(dataDir+"enum.png",          "PNG");
    m_pixmap[pm_Actor]       .load(dataDir+"actor.png",         "PNG");
    m_pixmap[pm_Usecase]     .load(dataDir+"usecase.png",       "PNG");
    m_pixmap[pm_InitialState].load(dataDir+"initial_state.png", "PNG");
    m_pixmap[pm_EndState]    .load(dataDir+"end_state.png",     "PNG");
    m_pixmap[pm_Branch]      .load(dataDir+"branch.png",        "PNG");
    m_pixmap[pm_Object]      .load(dataDir+"object.png",        "PNG");
    m_pixmap[pm_Component]   .load(dataDir+"component.png",     "PNG");
    m_pixmap[pm_Node]        .load(dataDir+"node.png",          "PNG");
    m_pixmap[pm_Entity]      .load(dataDir+"entity.png",        "PNG");
    m_pixmap[pm_Artifact]    .load(dataDir+"artifact.png",      "PNG");
    m_pixmap[pm_Text]        .load(dataDir+"text.png",          "PNG");
    m_pixmap[pm_Subsystem]   .load(dataDir+"subsystem.png",     "PNG");

    switch(type) {
    case mt_Logical_View:
        m_pInsert = new KPopupMenu(this);
        insertContainerItems(true);
        insertSeparator();
        insertStdItem(mt_Paste);
        insertSeparator();
        insertStdItem(mt_Import_Classes);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Component_View:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Component_Folder);
        insertStdItem(mt_Subsystem);
        insertStdItem(mt_Component);
        insertStdItem(mt_Artifact);
        insertStdItem(mt_Component_Diagram);
        insertFileNew();
        insertSeparator();
        insertStdItem(mt_Paste);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Deployment_View:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Deployment_Folder);
        insertStdItem(mt_Node);
        insertStdItem(mt_Deployment_Diagram);
        insertFileNew();
        insertSeparator();
        insertStdItem(mt_Paste);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_EntityRelationship_Model:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_EntityRelationship_Folder);
        insertStdItem(mt_Entity);
        insertStdItem(mt_EntityRelationship_Diagram);
        insertFileNew();
        insertSeparator();
        insertStdItem(mt_Paste);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_UseCase_View:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_UseCase_Folder);
        insertStdItem(mt_Actor);
        insertStdItem(mt_UseCase);
        insertStdItem(mt_UseCase_Diagram);
        insertFileNew();
        insertSeparator();
        //                      insertStdItem(mt_Cut);
        //                      insertStdItem(mt_Copy);
        insertStdItem(mt_Paste);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Logical_Folder:
        m_pInsert = new KPopupMenu(this);
        insertContainerItems(true);
        insertStdItems();
        insertStdItem(mt_Import_Classes);
        insertSubmodelAction();
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Component_Folder:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Component_Folder);
        insertStdItem(mt_Subsystem);
        insertStdItem(mt_Component);
        insertStdItem(mt_Artifact);
        insertStdItem(mt_Component_Diagram);
        insertFileNew();
        insertStdItems();
        insertSubmodelAction();
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Deployment_Folder:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Deployment_Folder);
        insertStdItem(mt_Node);
        insertStdItem(mt_Deployment_Diagram);
        insertFileNew();
        insertStdItems();
        insertSubmodelAction();
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_UseCase_Folder:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_UseCase_Folder);
        insertStdItem(mt_Actor);
        insertStdItem(mt_UseCase);
        insertStdItem(mt_UseCase_Diagram);
        insertFileNew();
        insertStdItems();
        insertSubmodelAction();
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_EntityRelationship_Folder:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_EntityRelationship_Folder);
        insertStdItem(mt_Entity);
        insertStdItem(mt_EntityRelationship_Diagram);
        insertFileNew();
        insertStdItems();
        insertSubmodelAction();
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
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
        //don't insert standard items because cut/copy not currently
        // possible with tabbed diagrams (it didn't work anyway)
        //insertStdItems(false);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Export_Image);
        insertStdItem(mt_Properties);
        break;

        //FIXME a lot of these insertItem()s could be insertStandardItem()s
    case mt_On_UseCase_Diagram:
        m_pInsert = new KPopupMenu( this );
        m_pInsert -> insertItem(m_pixmap[pm_Actor], i18n( "Actor..." ), mt_Actor );
        m_pInsert -> insertItem(m_pixmap[pm_Usecase], i18n( "Use Case..."), mt_UseCase );
        insertStdItem(mt_FloatText );
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_Class_Diagram:
        m_pInsert = new KPopupMenu( this );
        m_pInsert -> insertItem(m_pixmap[pm_Class], i18n("Class..."), mt_Class);
        m_pInsert->insertItem(m_pixmap[pm_Interface], i18n("Interface..."), mt_Interface);
        m_pInsert->insertItem(m_pixmap[pm_Datatype], i18n("Datatype..."), mt_Datatype);
        m_pInsert->insertItem(m_pixmap[pm_Enum], i18n("Enum..."), mt_Enum);
        m_pInsert -> insertItem(m_pixmap[pm_Package], i18n("Package..."), mt_Package);
        insertStdItem(mt_FloatText);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_State_Diagram:
        m_pInsert = new KPopupMenu( this );
        m_pInsert -> insertItem(m_pixmap[pm_InitialState], i18n("Initial State"), mt_Initial_State );
        m_pInsert -> insertItem(m_pixmap[pm_EndState], i18n("End State"), mt_End_State );
        m_pInsert -> insertItem(m_pixmap[pm_Usecase], i18n("State..."), mt_State );
        insertStdItem(mt_FloatText);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_Activity_Diagram:
        m_pInsert = new KPopupMenu( this );
        m_pInsert -> insertItem(m_pixmap[pm_InitialState], i18n("Initial Activity"), mt_Initial_Activity );
        m_pInsert -> insertItem(m_pixmap[pm_EndState], i18n("End Activity"), mt_End_Activity );
        m_pInsert -> insertItem(m_pixmap[pm_Usecase], i18n("Activity..."), mt_Activity );
        m_pInsert -> insertItem(m_pixmap[pm_Branch], i18n("Branch/Merge"), mt_Branch );
        insertStdItem(mt_FloatText);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_Component_Diagram:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(m_pixmap[pm_Subsystem], i18n("Subsystem..."), mt_Subsystem);
        m_pInsert->insertItem(m_pixmap[pm_Component], i18n("Component..."), mt_Component);
        m_pInsert->insertItem(m_pixmap[pm_Artifact], i18n("Artifact..."), mt_Artifact);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_Deployment_Diagram:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(m_pixmap[pm_Node], i18n("Node..."), mt_Node);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_EntityRelationship_Diagram:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(m_pixmap[pm_Entity], i18n("Entity..."), mt_Entity);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_On_Sequence_Diagram:
    case mt_On_Collaboration_Diagram:
        m_pInsert = new KPopupMenu( this );
        m_pInsert -> insertItem(m_pixmap[pm_Object], i18n("Object..."), mt_Object);
        insertStdItem(mt_FloatText);
        insertFileNew();
        insertSeparator();
        setupDiagramMenu(view);
        break;

    case mt_Class:
        m_pInsert = new KPopupMenu(this);
        m_pInsert -> insertItem(SmallIcon( "CVpublic_var"), i18n("Attribute"), mt_Attribute);
        m_pInsert -> insertItem(SmallIcon( "CVpublic_meth"), i18n("Operation"), mt_Operation);
        m_pInsert -> insertItem(SmallIcon("source"), i18n("Template"), mt_Template);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        break;

    case mt_Interface:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(SmallIcon("CVpublic_meth"), i18n("Operation"), mt_Operation);
        m_pInsert -> insertItem(SmallIcon("source"), i18n("Template"), mt_Template);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        break;

    case mt_Package:
        m_pInsert = new KPopupMenu(this);
        insertContainerItems(false);
        insertStdItems();
        insertStdItem(mt_Properties);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Subsystem:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Subsystem);
        insertStdItem(mt_Component);
        insertStdItem(mt_Artifact);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Component:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_Component);
        insertStdItem(mt_Artifact);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        insertSeparator();
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;

    case mt_Entity:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(SmallIcon("source"), i18n("Entity Attribute..."), mt_EntityAttribute);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        break;

    case mt_EnumLiteral:
        insertStdItems(false);
        break;

    case mt_Enum:
        m_pInsert = new KPopupMenu(this);
        m_pInsert->insertItem(SmallIcon("source"), i18n("Enum Literal..."), mt_EnumLiteral);
        insertFileNew();
        insertStdItems();
        insertStdItem(mt_Properties);
        break;

    case mt_Datatype:
    case mt_Node:
    case mt_Artifact:
    case mt_Actor:
    case mt_UseCase:
    case mt_Attribute:
    case mt_EntityAttribute:
    case mt_Operation:
    case mt_Template:
        insertStdItems(false);
        insertStdItem(mt_Properties);
        break;

    case mt_New_Parameter:
        insertStdItem(mt_New_Parameter);
        break;

    case mt_New_Operation:
        insertStdItem(mt_New_Operation);
        break;

    case mt_New_Attribute:
        insertStdItem(mt_New_Attribute);
        break;

    case mt_New_Template:
        insertStdItem(mt_New_Template);
        break;

    case mt_New_EnumLiteral:
        insertStdItem(mt_New_EnumLiteral);
        break;

    case mt_New_EntityAttribute:
        insertStdItem(mt_New_EntityAttribute);
        break;

    case mt_New_Activity:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_New_Activity);
        insertFileNew();
        break;

    case mt_Activity_Selected:
        m_pInsert = new KPopupMenu(this);
        insertStdItem(mt_New_Activity);
        insertFileNew();
        insertStdItem(mt_Rename);
        insertStdItem(mt_Delete);
        break;

    case mt_Parameter_Selected:
        insertStdItem(mt_New_Parameter);
        insertStdItem(mt_Rename);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_Operation_Selected:
        insertStdItem(mt_New_Operation);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_Attribute_Selected:
        insertStdItem(mt_New_Attribute);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_Template_Selected:
        insertItem(SmallIcon("source"),i18n("New Template..."), mt_New_Attribute);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_EnumLiteral_Selected:
        insertStdItem(mt_New_EnumLiteral);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_EntityAttribute_Selected:
        insertStdItem(mt_New_EntityAttribute);
        insertStdItem(mt_Delete);
        insertStdItem(mt_Properties);
        break;

    case mt_Association_Selected:
        insertStdItem(mt_Delete);
        insertStdItem(mt_Line_Color);
        insertStdItem(mt_Properties);
        break;

    case mt_Anchor:
        insertItem(SmallIcon( "editdelete"),i18n("Delete Anchor"), mt_Delete);
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
        insertStdItem(mt_Delete);
        insertItem(i18n("Change Association Name..."), mt_Rename_Name);
        insertItem(i18n("Change Role A Name..."), mt_Rename_RoleAName);
        insertItem(i18n("Change Role B Name..."), mt_Rename_RoleBName);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_Reset_Label_Positions);
        insertStdItem(mt_Line_Color);
        insertStdItem(mt_Properties);
        break;

    case mt_AttributeAssociation:
        insertStdItem(mt_Delete);  // @todo add more items
        break;

    case mt_Collaboration_Message:
        //                      insertStdItem(mt_Cut);
        //                      insertStdItem(mt_Copy);
        //                      insertStdItem(mt_Paste);
        //                      insertSeparator();
        insertStdItem(mt_Delete);
        insertStdItem(mt_Change_Font);
        insertStdItem(mt_New_Operation);
        insertItem(i18n("Select Operation..."), mt_Select_Operation);
        break;

    case mt_Model:
        insertItem(i18n("Rename..."), mt_Model);
        break;

    default:
        insertStdItem(mt_Expand_All);
        insertStdItem(mt_Collapse_All);
        break;
    }//end switch

    if( view ) {
        bool bCutState = UMLApp::app() -> getCutCopyState();
        setItemEnabled( mt_Undo, UMLApp::app()->getUndoEnabled() );
        setItemEnabled( mt_Redo, UMLApp::app()->getRedoEnabled() );
        setItemEnabled( mt_Cut, bCutState );
        setItemEnabled( mt_Copy, bCutState );
        setItemEnabled( mt_Paste, UMLApp::app() -> getPasteState() );
    }
    if(m_pInsert)
        connect(m_pInsert, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
    if(m_pShow)
        connect(m_pShow, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
    if(m_pColor)
        connect(m_pColor, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
}

void ListPopupMenu::setupDiagramMenu(UMLView* view) {
    insertItem(SmallIcon("undo"), i18n("Undo"), mt_Undo);
    insertItem(SmallIcon("redo"), i18n("Redo"), mt_Redo);
    insertSeparator();
    insertStdItem(mt_Cut);
    insertStdItem(mt_Copy);
    insertStdItem(mt_Paste);
    insertSeparator();
    insertItem(SmallIcon("editclear"), i18n("Clear Diagram"), mt_Clear);
    insertStdItem(mt_Export_Image);
    insertSeparator();
    insertItem(i18n("Snap to Grid"), mt_SnapToGrid);
    setItemChecked(mt_SnapToGrid, view->getSnapToGrid() );
    insertItem(i18n("Show Grid"), mt_ShowSnapGrid );
    setItemChecked(mt_ShowSnapGrid, view->getShowSnapGrid() );
    insertStdItem(mt_Properties);
}

