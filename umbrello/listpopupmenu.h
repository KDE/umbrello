/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LISTPOPUPMENU_H
#define LISTPOPUPMENU_H

#include <kpopupmenu.h>
#include "umlnamespace.h"


class UMLView;
class UMLWidget;
/**
 *	A popup menu that depending on what type it is set to will
 *	display a different menu.
 *
 *	@short	Displays a popup menu.
 *	@author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ListPopupMenu : public KPopupMenu {
public:

	enum Menu_Type
	{
	    //new type on left which replaces listed on right
	    mt_Logical_View = 1900,           //LVIEW
	    mt_UseCase_View,                  //UCVIEW
	    mt_Component_View,
	    mt_Deployment_View,
	    mt_UseCase_Diagram,               //UCDIAGRAM
	    mt_Sequence_Diagram,              //SEQDIAGRAM
	    mt_Class_Diagram,                 //CONDIAGRAM
	    mt_Collaboration_Diagram,         //COLLDIAGRAM
	    mt_State_Diagram,
	    mt_Activity_Diagram,
	    mt_Component_Diagram,
	    mt_Deployment_Diagram,

	    mt_On_UseCase_Diagram,            //UCDIAGRAM
	    mt_On_Sequence_Diagram,           //SEQDIAGRAM
	    mt_On_Class_Diagram,              //CONDIAGRAM
	    mt_On_Collaboration_Diagram,      //COLLDIAGRAM
	    mt_On_State_Diagram,
	    mt_On_Activity_Diagram,
	    mt_On_Component_Diagram,
	    mt_On_Deployment_Diagram,

	    mt_Logical_Folder,                //LFOLDER
	    mt_UseCase_Folder,                //UCFOLDER
	    mt_Component_Folder,
	    mt_Deployment_Folder,

	    mt_Class,                       //CLASS
	    mt_Package,
	    mt_Component,
	    mt_Node,
	    mt_Artifact,
	    mt_Interface,
	    mt_Actor,                         //ACTOR
	    mt_UseCase,                       //USECASE
	    mt_Attribute,                     //ATTRIBUTE
	    mt_Object,
	    mt_Initial_State,
	    mt_End_State,
	    mt_State,
	    mt_Activity,
	    mt_Initial_Activity,
	    mt_End_Activity,
	    mt_Operation,                     //OPERATION
	    mt_New_Parameter,                 //PARM  and PARMNEW
	    mt_New_Operation,                 //NEWOP
	    mt_New_Attribute,                 //NEWATT
	    mt_New_Template,
	    mt_Parameter_Selected,            //PARMSEL
	    mt_Operation_Selected,            //OPSEL
	    mt_Attribute_Selected,            //ATTSEL
	    mt_Template_Selected,
	    mt_Association_Selected,          //ASSOCSEL
	    mt_Show_Attributes,               //SHOWATTS
	    mt_Show_Operations,               //SHOWOPS
	    mt_Scope,                         //SCOPE
	    mt_DrawAsCircle,
	    mt_Rename_Object,                 //ROBJECT
	    mt_Select_Operation,              //SOPERATION
	    mt_Anchor,                        //ANCHOR
	    mt_Properties,                    //PROPERTIES
	    mt_Rename,                        //RENAME
	    mt_Delete,                        //DELETE
	    mt_Export_Image,                  //EXPORTIMAGE
	    mt_Import_Classes,                //IMPORTCLASSES
	    mt_Sequence_Number,               //SEQNUMBER
	    mt_Cut,                           //CUT
	    mt_Copy,                          //COPY
	    mt_Paste,                         //PASTE
	    mt_Clear,                         //CLEAR
	    mt_Redo,
	    mt_Undo,
	    mt_Link_Docs,                     //LINKDOCS
	    mt_Delete_Message,                //DELETEMESSAGE
	    mt_Show_Operation_Signature,      //SHOWOPSIG

	    mt_Show_Attribute_Signature,      //SHOWATTSIG
	    mt_Message_Text,                  //MESSAGET
	    mt_Collaboration_Message,         //COLLMESSAGE
	    mt_FloatText,                     //FLOATTEXT
	    mt_MultiA,                        //MULTIA
	    mt_MultiB,                        //MULTIB
	    mt_Name,                          //Association name
	    mt_RoleName,                      //ROLENAME
	    mt_Delete_Association,            //DELETEASSOC
	    mt_Delete_Selection,              //DELETESELECTION
	    mt_Reset_Label_Positions,

	    mt_Multi_Selection,               //MULTISELECTION
	    mt_Line_Color,                    //LINECOLOR
	    mt_Fill_Color,                    //FILLCOLOR
	    mt_Use_Fill_Color,                //USEFCOLOR
	    mt_Default_Properties,            //DEFAULTPROPERTIES
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

			mt_Expand_All,                     //Expand all items in the list
			mt_Collapse_All,                   //Collapse all items in the list
			
		mt_Refactoring,

	    mt_Undefined  =  - 1
	};

	/**
	 *	Constructs the popup menu.
	 *
	 *	@param	parent		The parent to ListPopupMenu.
	 *	@param	type		The type of menu to display.
	 */
	ListPopupMenu(QWidget *parent, Menu_Type type = mt_Undefined, UMLView * view = 0);

	/**
	 *	Constructs the popup menu for a list view item.
	 *
	 *	@param	parent		The parent to ListPopupMenu.
	 *	@param	type		The type of menu to display.
	 */
	ListPopupMenu(QWidget *parent, Uml::ListView_Type type);

	/**
	 *	Constructs the popup menu for a canvas widget.
	 *
	 *	@param	parent		The parent to ListPopupMenu.
	 *	@param	object		The UMLWidget to represent a menu for.
	 */
	ListPopupMenu(QWidget * parent, UMLWidget * object, bool multi = false);

	/**
	 *	Standard	deconstructor.
	 */
	~ListPopupMenu();
	static Uml::UMLObject_Type convert_MT_OT(Menu_Type mt);
	static Uml::Diagram_Type convert_MT_DT(Menu_Type mt);
private:
	void setupMenu(Menu_Type type, UMLView * view  = 0);
	KPopupMenu * m_pInsert, * m_pShow, * m_pColor;
	void setupColor(bool fc);
	void setupDiagramMenu(UMLView* view);
};

#endif
