/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "listpopupmenu.h"
#include "umlwidget.h"
#include "umldoc.h"
#include "classwidget.h"
#include "interfacewidget.h"
#include "floatingtext.h"
#include "uml.h"
#include "umlview.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "objectwidget.h"

//ListPopupMenu for a UMLView (diagram)
ListPopupMenu::ListPopupMenu(QWidget *parent, Menu_Type type, UMLView * view)
  : KPopupMenu(parent) {
	setupMenu(type, view);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//ListPopupMenu for the tree list view
ListPopupMenu::ListPopupMenu(QWidget *parent, Uml::ListView_Type type)
  : KPopupMenu(parent) {
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

		case Uml::lvt_Datatype:
			mt = mt_Datatype;
			break;

		case Uml::lvt_Attribute:
			mt = mt_Attribute;
			break;

		case Uml::lvt_Operation:
			mt = mt_Operation;
			break;
		default:   ;
			//break;
	}
	setupMenu(mt);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//ListPopupMenu for a canvas widget
ListPopupMenu::ListPopupMenu(QWidget * parent, UMLWidget * object,
			     bool multi, bool unique)
  : KPopupMenu(parent)
{
	//make the right menu for the type
	//make menu for logical view
	m_pInsert = 0;
	m_pShow = 0;
	m_pColor = 0;
	bool sig = false;
	if(!object)
		return;
	StateWidget *pState;
	ActivityWidget *pActivity;
	ClassWidget *c;
	InterfaceWidget *interfaceWidget = 0;
	UMLView * pView = static_cast<UMLView *>( parent );
	Uml::UMLWidget_Type type = object -> getBaseType();

	if(multi) {
		if (unique == true) {
			switch (type) {
				case Uml::wt_Class:
					c = static_cast<ClassWidget *>( object );
					m_pShow = new KPopupMenu(this, "Show");
					m_pShow -> setCheckable(true);
					m_pShow -> insertItem( i18n("Attributes"),
											mt_Show_Attributes_Selection);
					m_pShow -> setItemChecked(mt_Show_Attributes_Selection,
											c -> getShowAtts());
					m_pShow -> insertItem( i18n("Operations"),
											mt_Show_Operations_Selection);
					m_pShow -> setItemChecked(mt_Show_Operations_Selection,
											c -> getShowOps());
					m_pShow -> insertItem(i18n("Visibility"), mt_Scope_Selection);
					m_pShow -> setItemChecked(mt_Scope_Selection,
											c -> getShowScope());
					m_pShow -> insertItem(i18n("Operation Signature"),
											mt_Show_Operation_Signature_Selection);
					sig = false;
					if( c -> getShowOpSigs() == Uml::st_SigNoScope ||
					        c -> getShowOpSigs() == Uml::st_ShowSig)
						sig = true;
					m_pShow -> setItemChecked(mt_Show_Operation_Signature_Selection,
											sig);
					m_pShow -> insertItem(i18n("Attribute Signature"),
											mt_Show_Attribute_Signature_Selection);
					sig = false;
					if( c -> getShowAttSigs() == Uml::st_SigNoScope ||
					        c -> getShowAttSigs() == Uml::st_ShowSig)
						sig = true;
					m_pShow -> setItemChecked(mt_Show_Attribute_Signature_Selection,
											sig);
					m_pShow->insertItem(i18n("Package"), mt_Show_Packages_Selection);
					m_pShow->setItemChecked(mt_Show_Packages_Selection,
											c -> getShowPackage());
					m_pShow->insertItem(i18n("Stereotype"),
											mt_Show_Stereotypes_Selection);
					m_pShow->setItemChecked(mt_Show_Stereotypes_Selection,
											c -> getShowStereotype());
					insertItem(SmallIcon( "info"),i18n("Show"), m_pShow);
					break;
				case Uml::wt_Interface:
					interfaceWidget = static_cast<InterfaceWidget*>(object);
					if (! interfaceWidget)
						break;
					m_pShow = new KPopupMenu(this, "Show");
					m_pShow->setCheckable(true);
					m_pShow->insertItem(i18n("Operations"),
											mt_Show_Operations_Selection);
					m_pShow->setItemChecked(mt_Show_Operations_Selection,
											interfaceWidget->getShowOps());
					m_pShow->insertItem(i18n("Visibility"), mt_Scope_Selection);
					m_pShow->setItemChecked(mt_Scope_Selection,
											interfaceWidget->getShowScope());
					m_pShow->insertItem(i18n("Operation Signature"),
											mt_Show_Operation_Signature_Selection);
					sig = false;
					if( interfaceWidget->getShowOpSigs() == Uml::st_SigNoScope ||
					        interfaceWidget->getShowOpSigs() == Uml::st_ShowSig)
						sig = true;
					m_pShow->setItemChecked(mt_Show_Operation_Signature_Selection,
											sig);
					m_pShow->insertItem(i18n("Package"), mt_Show_Packages_Selection);
					m_pShow->setItemChecked(mt_Show_Packages_Selection,
											interfaceWidget->getShowPackage());
					insertItem(SmallIcon("info"),i18n("Show"), m_pShow);
				default: break;
			} // switch (type)
		} // if (unique == true)
		setupColorSelection(object -> getUseFillColour());
		insertSeparator();
		insertStdItem(mt_Cut);
		insertStdItem(mt_Copy);
		insertStdItem(mt_Paste);
		insertSeparator();
		insertItem(SmallIcon( "fonts" ), i18n( "Change Font..." ), mt_Change_Font_Selection );
		insertItem(SmallIcon( "editdelete" ), i18n("Delete Selected Items"), mt_Delete_Selection);

		// add this here and not above with the other stuff of the interface
		// user might expect it at this position of the context menu
		if (unique == true && type == Uml::wt_Interface) {
			insertItem(i18n("Draw as Circle"), mt_DrawAsCircle_Selection);
			setItemChecked( mt_DrawAsCircle_Selection, interfaceWidget->getDrawAsCircle() );
		}

		if(m_pInsert)
			connect(m_pInsert, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
		if(m_pShow)
			connect(m_pShow, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
		if(m_pColor)
			connect(m_pColor, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
		return;
	}

	switch(type) {
		case Uml::wt_Actor:
		case Uml::wt_UseCase:
			setupColor(object -> getUseFillColour());
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			insertStdItem(mt_Rename);
			insertStdItem(mt_Change_Font);
			insertStdItem(mt_Properties);
			break;

		case Uml::wt_Class:
			c = static_cast<ClassWidget *>(object);
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "source" ), i18n("Attribute"), mt_Attribute);
			m_pInsert -> insertItem( SmallIcon( "source"), i18n("Operation"), mt_Operation);
			insertItem(SmallIcon( "filenew"),i18n("New"), m_pInsert);

			m_pShow = new KPopupMenu(this, "Show");
			m_pShow -> setCheckable(true);
			m_pShow -> insertItem( i18n("Attributes"), mt_Show_Attributes);
			m_pShow -> setItemChecked(mt_Show_Attributes, c -> getShowAtts());
			m_pShow -> insertItem( i18n("Operations"), mt_Show_Operations);
			m_pShow -> setItemChecked(mt_Show_Operations, c -> getShowOps());
			m_pShow -> insertItem(i18n("Visibility"), mt_Scope);
			m_pShow -> setItemChecked(mt_Scope, c -> getShowScope());
			m_pShow -> insertItem(i18n("Operation Signature"), mt_Show_Operation_Signature);
			sig = false;
			if( c -> getShowOpSigs() == Uml::st_SigNoScope ||
			        c -> getShowOpSigs() == Uml::st_ShowSig)
				sig = true;
			m_pShow -> setItemChecked(mt_Show_Operation_Signature, sig);
			m_pShow -> insertItem(i18n("Attribute Signature"), mt_Show_Attribute_Signature);
			sig = false;
			if( c -> getShowAttSigs() == Uml::st_SigNoScope ||
			        c -> getShowAttSigs() == Uml::st_ShowSig)
				sig = true;
			m_pShow -> setItemChecked(mt_Show_Attribute_Signature, sig);
			m_pShow->insertItem(i18n("Package"), mt_Show_Packages);
			m_pShow->setItemChecked(mt_Show_Packages, c -> getShowPackage());
			m_pShow->insertItem(i18n("Stereotype"), mt_Show_Stereotypes);
			m_pShow->setItemChecked(mt_Show_Stereotypes, c -> getShowStereotype());
			insertItem(SmallIcon( "info"),i18n("Show"), m_pShow);

			setupColor(object -> getUseFillColour());
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			insertStdItem(mt_Rename);
			insertStdItem(mt_Change_Font);
			insertItem(i18n("Refactor"), mt_Refactoring);
			insertItem(i18n("View Code"), mt_ViewCode);
			insertStdItem(mt_Properties);
			break;

		case Uml::wt_Interface:
			interfaceWidget = static_cast<InterfaceWidget*>(object);
			if (! interfaceWidget)
				break;
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert->insertItem(SmallIcon("source"), i18n("Operation"), mt_Operation);
			insertItem(SmallIcon("filenew"),i18n("New"), m_pInsert);

			m_pShow = new KPopupMenu(this, "Show");
			m_pShow->setCheckable(true);
			m_pShow->insertItem(i18n("Operations"), mt_Show_Operations);
			m_pShow->setItemChecked(mt_Show_Operations, interfaceWidget->getShowOps());
			m_pShow->insertItem(i18n("Visibility"), mt_Scope);
			m_pShow->setItemChecked(mt_Scope, interfaceWidget->getShowScope());
			m_pShow->insertItem(i18n("Operation Signature"), mt_Show_Operation_Signature);
			sig = false;
			if( interfaceWidget->getShowOpSigs() == Uml::st_SigNoScope ||
			        interfaceWidget->getShowOpSigs() == Uml::st_ShowSig)
				sig = true;
			m_pShow->setItemChecked(mt_Show_Operation_Signature, sig);
			m_pShow->insertItem(i18n("Package"), mt_Show_Packages);
			m_pShow->setItemChecked(mt_Show_Packages, interfaceWidget->getShowPackage());
			insertItem(SmallIcon("info"),i18n("Show"), m_pShow);

			setupColor(object->getUseFillColour());
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			insertStdItem(mt_Rename);
			insertStdItem(mt_Change_Font);
			insertItem( SmallIcon( "unknown"), i18n("View Code"),mt_ViewCode);
			insertItem(i18n("Draw as Circle"), mt_DrawAsCircle);
			setItemChecked( mt_DrawAsCircle, interfaceWidget->getDrawAsCircle() );
			insertStdItem(mt_Properties);
			break;

		case Uml::wt_Enum:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert->insertItem(SmallIcon("source"), i18n("Enum Literal"), mt_EnumLiteral);
			insertItem(SmallIcon("filenew"),i18n("New"), m_pInsert);

			setupColor(object->getUseFillColour());
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
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
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
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
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			insertItem( SmallIcon( "charset"), i18n("Rename Class..."), mt_Rename);
			insertItem(SmallIcon( "charset"), i18n("Rename Object..."), mt_Rename_Object);
			insertStdItem(mt_Change_Font);
			insertStdItem(mt_Properties);
			break;

		case Uml::wt_Message:
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			insertStdItem(mt_Change_Font);
			insertItem(SmallIcon( "filenew"), i18n("New Operation..."), mt_Operation);
			insertItem(SmallIcon( "charset"), i18n("Select Operation..."), mt_Select_Operation);
			insertItem(SmallIcon( "charset"), i18n("Set Sequence Number..."), mt_Sequence_Number);
			break;

		case Uml::wt_Note:
			setupColor(object -> getUseFillColour());
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertItem(SmallIcon( "editdelete"), i18n("Clear"), mt_Clear);
			insertSeparator();
			insertItem(SmallIcon( "charset"), i18n("Change Text..."), mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Change_Font);
			//insertItem(i18n("Link Documentation"), mt_Link_Docs);
			//setItemChecked(mt_Link_Docs, ((NoteWidget*)object)->getLinkState());
			break;

		case Uml::wt_Box:
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			break;

		case Uml::wt_State:
			pState = static_cast< StateWidget *>( object );
			if( pState -> getStateType() == StateWidget::Normal ) {
				m_pInsert = new KPopupMenu(this,"New");
				m_pInsert -> insertItem(SmallIcon( "filenew"), i18n("Activity..."), mt_New_Activity);
				insertItem(SmallIcon( "filenew"),i18n("New"), m_pInsert);
			}
			setupColor( object -> getUseFillColour() );
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			if( pState -> getStateType() == StateWidget::Normal ) {
				insertItem(SmallIcon( "charset"), i18n("Change State Name..."), mt_Rename);
				insertStdItem(mt_Change_Font);
				insertStdItem(mt_Properties);
			}
			break;

		case Uml::wt_Activity:
			pActivity = static_cast<ActivityWidget *>( object );
			if( pActivity -> getActivityType() == ActivityWidget::Normal )
				setupColor( object -> getUseFillColour() );
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Delete);
			if( pActivity -> getActivityType() == ActivityWidget::Normal ) {
				insertItem(SmallIcon( "charset"), i18n("Change Activity Name"), mt_Rename);
				insertStdItem(mt_Change_Font);
				insertStdItem(mt_Properties);
			}
			break;

		case Uml::wt_Text:
			switch( (static_cast<FloatingText*>(object))->getRole() ) {
				case Uml::tr_MultiB:
				case Uml::tr_MultiA:
				case Uml::tr_Name:
				case Uml::tr_RoleAName:
				case Uml::tr_RoleBName:
				case Uml::tr_ChangeA:
				case Uml::tr_ChangeB:
					insertItem(SmallIcon("editdelete"), i18n("Delete Association"), mt_Delete_Association);
					insertStdItem(mt_Change_Font);
					insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
					insertItem(SmallIcon( "info"), i18n("Properties..."), mt_Properties);
					break;

				case Uml::tr_Coll_Message_Self:
				case Uml::tr_Coll_Message:
				case Uml::tr_Seq_Message_Self:
				case Uml::tr_Seq_Message:
//					insertStdItem(mt_Cut);
//					insertStdItem(mt_Copy);
//					insertStdItem(mt_Paste);
//					insertSeparator();
					insertItem(SmallIcon( "editdelete"), i18n("Delete"), mt_Delete_Message);
					insertStdItem(mt_Change_Font);
					insertItem(SmallIcon( "filenew"), i18n("New Operation..."), mt_Operation);
					insertItem(SmallIcon( "charset"), i18n("Select Operation..."), mt_Select_Operation);
					insertItem(SmallIcon( "charset"), i18n("Set Sequence Number..."), mt_Sequence_Number);
					break;

				case Uml::tr_Floating:
				default:
					insertStdItem(mt_Cut);
					insertStdItem(mt_Copy);
					insertStdItem(mt_Paste);
					insertSeparator();
					insertStdItem(mt_Delete);
					insertItem(SmallIcon( "charset"), i18n("Change Text..."), mt_Rename);
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

	bool bCutState = pView -> getDocument() -> getCutCopyState();
	setItemEnabled( mt_Cut, bCutState );
	setItemEnabled( mt_Copy, bCutState );
	setItemEnabled( mt_Paste, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ListPopupMenu::~ListPopupMenu() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ListPopupMenu::insertStdItem(Menu_Type m)
{
	switch (m) {
	case mt_Properties:
		insertItem(SmallIcon("info"), i18n("Properties"), mt_Properties);
		break;
	case mt_Rename:
		insertItem(SmallIcon("charset"), i18n("Rename..."), mt_Rename);
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
	case mt_Expand_All:
		insertItem(i18n("Expand All"), mt_Expand_All);
		break;
	case mt_Collapse_All:
		insertItem(i18n("Collapse All"), mt_Collapse_All);
		break;
	default:
		kdWarning() << "ListPopupMenu::insertStdItem called on unimplemented Menu_Type " << m << endl;
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ListPopupMenu::setupColor(bool fc)
{
	m_pColor = new KPopupMenu( this, "Colour");
	m_pColor -> insertItem(SmallIcon( "color_line"), i18n("Line Color"), mt_Line_Color);
	m_pColor -> insertItem(SmallIcon( "color_fill"), i18n("Fill Color"), mt_Fill_Color);
	m_pColor -> insertItem( i18n("Use Fill Color"), mt_Use_Fill_Color);

	m_pColor -> setItemChecked(mt_Use_Fill_Color, fc);
	insertItem(SmallIcon( "colorize"), i18n("Color"), m_pColor);
}

void ListPopupMenu::setupColorSelection(bool fc)
{
	m_pColor = new KPopupMenu( this, "Colour");
	m_pColor -> insertItem(SmallIcon( "color_line"), i18n("Line Color"), mt_Line_Color_Selection);
	m_pColor -> insertItem(SmallIcon( "color_fill"), i18n("Fill Color"), mt_Fill_Color_Selection);
	m_pColor -> insertItem( i18n("Use Fill Color"), mt_Use_Fill_Color);

	m_pColor -> setItemChecked(mt_Use_Fill_Color, fc);
	insertItem(SmallIcon( "colorize"), i18n("Color"), m_pColor);
}

Uml::Diagram_Type ListPopupMenu::convert_MT_DT(Menu_Type mt) {
	Uml::Diagram_Type type =  Uml::dt_Class;

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

		default:
			break;
	}
	return type;
}

Uml::UMLObject_Type ListPopupMenu::convert_MT_OT(Menu_Type mt) {
	Uml::UMLObject_Type type =  Uml::ot_UMLObject;

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
	QString dataDir = dirs->findResourceDir("data", "umbrello/pics/object.xpm");
	dataDir += "/umbrello/pics/";
	QPixmap classPixmap(dataDir+"umlclass.xpm");
	QPixmap packagePixmap(dataDir+"package.xpm");
	QPixmap interfacePixmap(dataDir+"interface.xpm");
	QPixmap datatypePixmap(dataDir+"datatype.xpm");
	QPixmap enumPixmap(dataDir+"enum.xpm");
	QPixmap actorPixmap(dataDir+"actor.xpm");
	QPixmap usecasePixmap(dataDir+"case.xpm");
	QPixmap initialStatePixmap(dataDir+"initial_state.xpm");
	QPixmap endStatePixmap(dataDir+"end_state.xpm");
	QPixmap branchPixmap(dataDir+"branch.xpm");
	QPixmap objectPixmap(dataDir+"object.xpm");
	QPixmap componentPixmap(dataDir+"component.xpm");
	QPixmap nodePixmap(dataDir+"node.xpm");
	QPixmap artifactPixmap(dataDir+"artifact.xpm");

	switch(type) {
		case mt_Logical_View:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert -> insertItem(SmallIcon( "folder_new"), i18n("Folder"), mt_Logical_Folder);
			m_pInsert -> insertItem(classPixmap, i18n("Class..."), mt_Class);
			m_pInsert -> insertItem(interfacePixmap, i18n("Interface..."), mt_Interface);
			m_pInsert -> insertItem(datatypePixmap, i18n("Datatype..."), mt_Datatype);
			m_pInsert -> insertItem(enumPixmap, i18n("Enum..."), mt_Enum);
			m_pInsert->insertItem(packagePixmap, i18n("Package..."), mt_Package);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Class Diagram"), mt_Class_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("State Diagram"), mt_State_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Activity Diagram"), mt_Activity_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Sequence Diagram"), mt_Sequence_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Collaboration Diagram"), mt_Collaboration_Diagram);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
//			insertStdItem(mt_Cut);
//			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
//			insertStdItem(mt_Delete);
			insertItem(SmallIcon( "source_cpp"), i18n("Import Classes..."), mt_Import_Classes);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;


		case mt_Component_View:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(SmallIcon("folder_new"), i18n("Folder"), mt_Component_Folder);
			m_pInsert->insertItem(componentPixmap, i18n("Component..."), mt_Component);
			m_pInsert->insertItem(artifactPixmap, i18n("Artifact..."), mt_Artifact);
			m_pInsert->insertItem(SmallIcon("folder_red"),i18n("Component Diagram"),
					      mt_Component_Diagram);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_Deployment_View:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(SmallIcon("folder_new"), i18n("Folder"), mt_Component_Folder);
			m_pInsert->insertItem(nodePixmap, i18n("Node..."), mt_Node);
			m_pInsert->insertItem(SmallIcon("folder_violet"),i18n("Deployment Diagram"),
					      mt_Deployment_Diagram);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_UseCase_View:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "folder_new"),i18n("Folder"), mt_UseCase_Folder);
			m_pInsert -> insertItem(actorPixmap, i18n("Actor"), mt_Actor);
			m_pInsert -> insertItem(usecasePixmap, i18n("Use Case"), mt_UseCase);
			m_pInsert -> insertItem(SmallIcon( "folder_grey"),i18n("Use Case Diagram"), mt_UseCase_Diagram);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
//			insertStdItem(mt_Cut);
//			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_Logical_Folder:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert -> insertItem(SmallIcon( "folder_new"),i18n("Folder"), mt_Logical_Folder);
			m_pInsert -> insertItem(classPixmap, i18n("Class..."), mt_Class);
			m_pInsert -> insertItem(packagePixmap, i18n("Package..."), mt_Package);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Class Diagram"), mt_Class_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("State Diagram"), mt_State_Diagram);

			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Activity Diagram"), mt_Activity_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Sequence Diagram"), mt_Sequence_Diagram);
			m_pInsert -> insertItem(SmallIcon( "folder_green"),i18n("Collaboration Diagram"), mt_Collaboration_Diagram);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);

			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);

			insertItem(SmallIcon( "source_cpp"), i18n("Import Classes..."), mt_Import_Classes);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_Component_Folder:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(SmallIcon("folder_new"),i18n("Folder"), mt_Component_Folder);
			m_pInsert->insertItem(componentPixmap, i18n("Component..."), mt_Component);
			m_pInsert->insertItem(artifactPixmap, i18n("Artifact..."), mt_Artifact);
			m_pInsert->insertItem(SmallIcon("folder_red"),i18n("Component Diagram"),
					      mt_Component_Diagram);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_Deployment_Folder:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(SmallIcon("folder_new"),i18n("Folder"), mt_Component_Folder);
			m_pInsert->insertItem(nodePixmap, i18n("Node..."), mt_Node);
			m_pInsert->insertItem(SmallIcon("folder_violet"),i18n("Deployment Diagram"),
					      mt_Deployment_Diagram);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_UseCase_Folder:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "folder_new"),i18n("Folder"), mt_UseCase_Folder);
			m_pInsert -> insertItem(actorPixmap, i18n("Actor"), mt_Actor);
			m_pInsert -> insertItem(usecasePixmap, i18n("Use Case"), mt_UseCase);
			m_pInsert -> insertItem(SmallIcon( "folder_grey"),i18n("Use Case Diagram"), mt_UseCase_Diagram);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
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
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertItem(SmallIcon( "image"), i18n("Export as Picture..."), mt_Export_Image);
			insertStdItem(mt_Properties);
			break;

		case mt_On_UseCase_Diagram:
			m_pInsert = new KPopupMenu( this, "New" );
			m_pInsert -> insertItem(actorPixmap, i18n( "Actor..." ), mt_Actor );
			m_pInsert -> insertItem(usecasePixmap, i18n( "Use Case..."), mt_UseCase );
			m_pInsert -> insertItem(SmallIcon( "text"), i18n( "Text Line..." ), mt_FloatText );
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_Class_Diagram:
			m_pInsert = new KPopupMenu( this, "New" );
			m_pInsert -> insertItem(classPixmap, i18n("Class..."), mt_Class);
			m_pInsert->insertItem(interfacePixmap, i18n("Interface..."), mt_Interface);
			m_pInsert->insertItem(datatypePixmap, i18n("Datatype..."), mt_Datatype);
			m_pInsert->insertItem(enumPixmap, i18n("Enum..."), mt_Enum);
			m_pInsert -> insertItem(packagePixmap, i18n("Package..."), mt_Package);
			m_pInsert -> insertItem(SmallIcon( "text"), i18n( "Text Line..." ), mt_FloatText );
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_State_Diagram:
			m_pInsert = new KPopupMenu( this, "New" );
			m_pInsert -> insertItem(initialStatePixmap, i18n("Initial State"), mt_Initial_State );
			m_pInsert -> insertItem(endStatePixmap, i18n("End State"), mt_End_State );
			m_pInsert -> insertItem(usecasePixmap, i18n("State..."), mt_State );
			m_pInsert -> insertItem(SmallIcon( "text"), i18n( "Text Line..." ), mt_FloatText );
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_Activity_Diagram:
			m_pInsert = new KPopupMenu( this, "New" );
			m_pInsert -> insertItem(initialStatePixmap, i18n("Initial Activity"), mt_Initial_Activity );
			m_pInsert -> insertItem(endStatePixmap, i18n("End Activity"), mt_End_Activity );
			m_pInsert -> insertItem(usecasePixmap, i18n("Activity..."), mt_Activity );
			m_pInsert -> insertItem(branchPixmap, i18n("Branch/Merge"), mt_Branch );
			m_pInsert -> insertItem(SmallIcon( "text"), i18n( "Text Line..." ), mt_FloatText );
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_Component_Diagram:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(componentPixmap, i18n("Component..."), mt_Component);
			m_pInsert->insertItem(artifactPixmap, i18n("Artifact..."), mt_Artifact);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_Deployment_Diagram:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(nodePixmap, i18n("Node..."), mt_Node);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_On_Sequence_Diagram:
		case mt_On_Collaboration_Diagram:
			m_pInsert = new KPopupMenu( this, "New" );
			m_pInsert -> insertItem(objectPixmap, i18n("Object..."), mt_Object);
			m_pInsert -> insertItem( SmallIcon( "text"),i18n( "Text Line..." ), mt_FloatText );
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			setupDiagramMenu(view);
			break;

		case mt_Class:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "source"), i18n("Attribute"), mt_Attribute);
			m_pInsert -> insertItem(SmallIcon( "source"), i18n("Operation"), mt_Operation);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Interface:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert->insertItem(SmallIcon("source"), i18n("Operation"), mt_Operation);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Package:
			m_pInsert = new KPopupMenu(this, "New");
			m_pInsert->insertItem(classPixmap, i18n("Class..."), mt_Class);
			m_pInsert->insertItem(interfacePixmap, i18n("Interface..."), mt_Interface);
			m_pInsert->insertItem(datatypePixmap, i18n("Datatype..."), mt_Datatype);
			m_pInsert->insertItem(enumPixmap, i18n("Enum..."), mt_Enum);
			m_pInsert->insertItem(packagePixmap, i18n("Package..."), mt_Package);
			insertItem(SmallIcon( "filenew"), i18n("New"), m_pInsert);
			insertSeparator();
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			insertSeparator();
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;

		case mt_Datatype:
		case mt_Enum:
		case mt_Component:
		case mt_Node:
		case mt_Artifact:
		case mt_Actor:
		case mt_UseCase:
		case mt_Attribute:
		case mt_Operation:
			insertStdItem(mt_Cut);
			insertStdItem(mt_Copy);
			insertStdItem(mt_Paste);
			insertSeparator();
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_New_Parameter:
			insertItem(SmallIcon("source"),i18n("New Parameter..."), mt_New_Parameter);
			break;

		case mt_New_Operation:
			insertItem(SmallIcon("source"),i18n("New Operation..."), mt_New_Operation);
			break;

		case mt_New_Attribute:
			insertItem(SmallIcon("source"), i18n("New Attribute..."), mt_New_Attribute);
			break;

		case mt_New_Template:
			insertItem(SmallIcon("source"), i18n("New Template..."), mt_New_Template);
			break;

		case mt_New_EnumLiteral:
			insertItem(SmallIcon("source"), i18n("New Literal..."), mt_New_EnumLiteral);
			break;

		case mt_New_Activity:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "source"),i18n("Activity..."), mt_New_Activity);
			insertItem(SmallIcon( "editcut"), i18n("New"), m_pInsert);
			break;

		case mt_Activity_Selected:
			m_pInsert = new KPopupMenu(this,"New");
			m_pInsert -> insertItem(SmallIcon( "source"),i18n("Activity..."), mt_New_Activity);
			insertItem(SmallIcon("filenew"), i18n("New"), m_pInsert);
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			break;

		case mt_Parameter_Selected:
			insertItem(SmallIcon("source"),i18n("New Parameter..."), mt_New_Parameter);
			insertStdItem(mt_Rename);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Operation_Selected:
			insertItem(SmallIcon("source"),i18n("New Operation..."), mt_New_Operation);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Attribute_Selected:
			insertItem(SmallIcon("source"),i18n("New Attribute..."), mt_New_Attribute);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Template_Selected:
			insertItem(SmallIcon("source"),i18n("New Template..."), mt_New_Attribute);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_EnumLiteral_Selected:
			insertItem(SmallIcon("source"),i18n("New Literal..."), mt_New_EnumLiteral);
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Association_Selected:
			insertStdItem(mt_Delete);
			insertStdItem(mt_Properties);
			break;

		case mt_Anchor:
			insertItem(SmallIcon( "editdelete"),i18n("Delete Anchor"), mt_Delete);
			break;

		case mt_MultiB:
			insertItem(SmallIcon( "editdelete"),i18n("Delete Association"), mt_Delete_Association);
			insertItem(SmallIcon( "charset"),i18n("Change Multiplicity..."), mt_Rename_MultiB);
			insertStdItem(mt_Change_Font);
			insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
			insertStdItem(mt_Properties);
			break;

		case mt_MultiA:
			insertItem(SmallIcon( "editdelete"),i18n("Delete Association"), mt_Delete_Association);
			insertItem(SmallIcon( "charset"),i18n("Change Multiplicity..."), mt_Rename_MultiA);
			insertStdItem(mt_Change_Font);
			insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
			insertItem(SmallIcon( "info"),i18n("Properties..."), mt_Properties);
			break;

		case mt_Name:
			insertItem(SmallIcon( "editdelete"),i18n("Delete Association"), mt_Delete_Association);
			insertItem(SmallIcon( "charset"), i18n("Change Name"), mt_Rename_Name);
			insertItem(SmallIcon( "fonts"), i18n( "Change Font" ), mt_Change_Font );
			insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
			insertItem(SmallIcon( "info"),i18n("Properties..."), mt_Properties);
			break;

		case mt_RoleName:
			insertItem(SmallIcon( "editdelete"),i18n("Delete Association"), mt_Delete_Association);
			insertItem(SmallIcon( "charset"), i18n("Change Association Name"), mt_Rename_Name);
			insertItem(SmallIcon( "charset"), i18n("Change Role A Name"), mt_Rename_RoleAName);
			insertItem(SmallIcon( "charset"), i18n("Change Role B Name"), mt_Rename_RoleBName);
			insertItem(SmallIcon( "fonts"), i18n( "Change Font" ), mt_Change_Font );
			insertItem(i18n("Reset Label Positions"), mt_Reset_Label_Positions);
			insertItem(SmallIcon( "info"),i18n("Properties..."), mt_Properties);
			break;

		case mt_Collaboration_Message:
//			insertStdItem(mt_Cut);
//			insertStdItem(mt_Copy);
//			insertStdItem(mt_Paste);
//			insertSeparator();
			insertStdItem(mt_Delete);
			insertStdItem(mt_Change_Font);
			insertItem(SmallIcon( "filenew"), i18n("New Operation..."), mt_Operation);
			insertItem(SmallIcon( "charset"), i18n("Select Operation..."), mt_Select_Operation);
			insertItem(SmallIcon( "charset"), i18n("Set Sequence Number..."), mt_Sequence_Number);
			break;
		default:
			insertStdItem(mt_Expand_All);
			insertStdItem(mt_Collapse_All);
			break;
	}//end switch

	if( view ) {
		bool bCutState = view -> getDocument() -> getCutCopyState();
		setItemEnabled( mt_Undo, UMLApp::app()->getUndoEnabled() );
		setItemEnabled( mt_Redo, UMLApp::app()->getRedoEnabled() );
		setItemEnabled( mt_Cut, bCutState );
		setItemEnabled( mt_Copy, bCutState );
		setItemEnabled( mt_Paste, view -> getDocument() -> getPasteState() );
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
	insertItem(SmallIcon("image"), i18n("Export as Picture..."), mt_Export_Image);
	insertSeparator();
	insertItem(i18n("Snap to Grid"), mt_SnapToGrid);
	setItemChecked(mt_SnapToGrid, view->getSnapToGrid() );
	insertItem(i18n("Show Grid"), mt_ShowSnapGrid );
	setItemChecked(mt_ShowSnapGrid, view->getShowSnapGrid() );
	insertStdItem(mt_Properties);
}
