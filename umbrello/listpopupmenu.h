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
class ClassifierWidget;

/**
 * A popup menu that depending on what type it is set to will
 * display a different menu.
 *
 * @short Displays a popup menu.
 * @author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ListPopupMenu : public KPopupMenu {
public:

	/// This type hosts all possible menu types.
	enum Menu_Type
	{
	    //new type on left which replaces listed on right
	    mt_Logical_View = 1900,           //LVIEW
	    mt_UseCase_View,                  //UCVIEW
	    mt_Component_View,			// 1902
	    mt_Deployment_View,			// 1903
	    mt_UseCase_Diagram,               //UCDIAGRAM
	    mt_Sequence_Diagram,              //SEQDIAGRAM
	    mt_Class_Diagram,                 //CONDIAGRAM
	    mt_Collaboration_Diagram,         //COLLDIAGRAM
	    mt_State_Diagram,			// 1908
	    mt_Activity_Diagram,		// 1909
	    mt_Component_Diagram,		// 1910
	    mt_Deployment_Diagram,		// 1911

	    mt_On_UseCase_Diagram,            //UCDIAGRAM
	    mt_On_Sequence_Diagram,           //SEQDIAGRAM
	    mt_On_Class_Diagram,              //CONDIAGRAM
	    mt_On_Collaboration_Diagram,      //COLLDIAGRAM
	    mt_On_State_Diagram,		// 1916
	    mt_On_Activity_Diagram,		// 1917
	    mt_On_Component_Diagram,		// 1918
	    mt_On_Deployment_Diagram,		// 1919

	    mt_Logical_Folder,                //LFOLDER
	    mt_UseCase_Folder,                //UCFOLDER
	    mt_Component_Folder,		// 1922
	    mt_Deployment_Folder,		// 1923

	    mt_Class,                       //CLASS
	    mt_Package,				// 1925
	    mt_Component,			// 1926
	    mt_Node,				// 1927
	    mt_Artifact,			// 1928
	    mt_Interface,			// 1929
	    mt_Enum,				// 1930
	    mt_Datatype,			// 1931
	    mt_Actor,                         //ACTOR
	    mt_UseCase,                       //USECASE
	    mt_Attribute,                     //ATTRIBUTE
	    mt_EnumLiteral,			// 1935
	    mt_Object,				// 1936
	    mt_Initial_State,			// 1937
	    mt_End_State,			// 1938
	    mt_State,				// 1939
	    mt_Activity,			// 1940
	    mt_Initial_Activity,		// 1941
	    mt_End_Activity,			// 1942
	    mt_Operation,                     //OPERATION
	    mt_New_Parameter,                 //PARM  and PARMNEW
	    mt_New_Operation,                 //NEWOP
	    mt_New_Attribute,                 //NEWATT
	    mt_New_Template,			// 1947
	    mt_New_EnumLiteral,			// 1948
	    mt_Parameter_Selected,            //PARMSEL
	    mt_Operation_Selected,            //OPSEL
	    mt_Attribute_Selected,            //ATTSEL
	    mt_Template_Selected,		// 1952
	    mt_EnumLiteral_Selected,		// 1953
	    mt_Association_Selected,          // Association without role names
	    mt_Show_Attributes,               //SHOWATTS
	    mt_Show_Attributes_Selection,     //SHOWATTS, multiple items
	    mt_Show_Operations,               //SHOWOPS
	    mt_Show_Operations_Selection,     //SHOWOPS, multiple items
	    mt_Show_Packages,                 //SHOWPACKAGE
	    mt_Show_Packages_Selection,       //SHOWPACKAGE, multiple items
	    mt_Show_Stereotypes,              //SHOWSTEREOTYPE
	    mt_Show_Stereotypes_Selection,    //SHOWSTEREOTYPE, multiple items
	    mt_Scope,                         //SCOPE
	    mt_Scope_Selection,               //SCOPE, multiple items
	    mt_DrawAsCircle,                  //DRAWASCIRCLE
	    mt_DrawAsCircle_Selection,        //DRAWASCIRCLE, multiple items
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
	    mt_Redo,				// 1980
	    mt_Undo,				// 1981
	    mt_Link_Docs,                     //LINKDOCS
	    mt_Show_Operation_Signature,      //SHOWOPSIG
	    mt_Show_Operation_Signature_Selection, //SHOWOPSIG, multiple items
	    mt_Show_Attribute_Signature,      //SHOWATTSIG
	    mt_Show_Attribute_Signature_Selection, //SHOWATTSIG, multiple items
	    mt_Message_Text,                  //MESSAGET
	    mt_Collaboration_Message,         //COLLMESSAGE
	    mt_FloatText,                     //FLOATTEXT
	    mt_MultiA,                        //MULTIA
	    mt_MultiB,                        //MULTIB
	    mt_Name,                          //Association name
	    mt_FullAssociation,                 // Association with role names
	    mt_RoleNameA,                       // 1994
	    mt_RoleNameB,                       // 1995
	    mt_Delete_Selection,              //DELETESELECTION
	    mt_Reset_Label_Positions,		// 1997
	    mt_Line_Color,                    //LINECOLOR
	    mt_Line_Color_Selection,          //LINECOLOR, multiple items
	    mt_Fill_Color,                    //FILLCOLOR
	    mt_Fill_Color_Selection,          //FILLCOLOR, multiple items
	    mt_Use_Fill_Color,                //USEFCOLOR
	    mt_Default_Properties,            //DEFAULTPROPERTIES
	    mt_Rename_MultiA,			// 2004
	    mt_Rename_MultiB,
	    mt_Rename_Name,
	    mt_Rename_RoleAName,
	    mt_Rename_RoleBName,
	    mt_Change_Font,			// 2009
	    mt_Change_Font_Selection,
	    mt_SnapToGrid,
	    mt_ShowSnapGrid,
	    mt_Activity_Selected,
	    mt_New_Activity,
	    mt_Up,
	    mt_Down,
	    mt_Branch,				// 2017

		mt_Expand_All,                     //Expand all items in the list
		mt_Collapse_All,                   //Collapse all items in the list

		mt_Refactoring,			// 2020
		mt_ViewCode, // view code document contents
		mt_Clone, // Create a deep copy of the object.
		mt_Show_Public_Only, // (not currently used)
		mt_Show_Public_Only_Selection,  // Show public operations/attributes only.

	    mt_Undefined  =  - 1
	};

	/**
	 * Constructs the popup menu for a diagram
	 *
	 * @param parent	The parent to ListPopupMenu.
	 * @param type		The type of menu to display.
	 */
	ListPopupMenu(QWidget* parent, Menu_Type type = mt_Undefined, UMLView* view = 0);

	/**
	 * Constructs the popup menu for a list view item.
	 *
	 * @param parent	The parent to ListPopupMenu.
	 * @param type		The type of menu to display.
	 */
	ListPopupMenu(QWidget* parent, Uml::ListView_Type type);

	/**
	 * Constructs the popup menu for a canvas widget.
	 *
	 * @param parent	The parent to ListPopupMenu.
	 * @param object	The UMLWidget to represent a menu for.
	 * @param multi		True if multiple items are selected.
	 * @param unique	True if multiple selected items all have
	 *			the same type (e.g. Class, Interface)
	 */
	ListPopupMenu(QWidget* parent, UMLWidget* object, bool multi = false, bool unique = false);

	/**
	 * Standard deconstructor.
	 */
	~ListPopupMenu();

	/**
	 * Utility: Convert a Menu_Type value to an Object_Type value.
	 */
	static Uml::Object_Type convert_MT_OT(Menu_Type mt);

	/**
	 * Utility: Convert a Menu_Type value to a Diagram_Type value.
	 */
	static Uml::Diagram_Type convert_MT_DT(Menu_Type mt);

private:
	/**
	 * Basic initialization - common to all constructors.
	 */
	void init();

	/**
	 * Shortcut for inserting a "File->New" choice.
	 */
	void insertFileNew();

	/**
	 * Shortcut for the most frequently used insertItem() calls.
	 *
	 * @param m	The Menu_Type for which to insert a menu item.
	 */
	void insertStdItem(Menu_Type m);

	/**
	 * Shortcut for the most frequently used insertStdItem() calls.
	 *
	 * @param insertLeadingSeparator  Set this true if the group shall
	 *				  start with a separator.
	 * @param type	The Widget_Type for which to insert the menu items.
	 *		If no argument is supplied then a Rename item will be
	 *		included.
	 */
	void insertStdItems(bool insertLeadingSeparator = true,
			    Uml::Widget_Type type = Uml::wt_UMLWidget);

	/**
	 * Shortcut for inserting standard model items (Class, Interface,
	 * Datatype, Enum, Package) as well as diagram choices.
	 *
	 * @param folderAndDiagrams	Set this true if folders and diagram
	 *				types shall be included as choices.
	 */
	void insertContainerItems(bool folderAndDiagrams);

	/**
	 * Inserts a menu item for an association related text
	 * (such as name, role, multiplicity etc.)
	 *
	 * @param label		The menu text.
	 * @param mt		The menu type.
	 */
	void insertAssocItem(QString label, Menu_Type mt);

	/**
	 * Creates a popup menu for a multiple selection of class and
	 * interface widgets.
	 */
	void makeMultiClassifierPopup(ClassifierWidget *c);

	/**
	 * Creates a popup menu for a single class or interface widgets.
	 */
	void makeClassifierPopup(ClassifierWidget *c);

	/**
	 * Shortcut for commonly used menu initializations.
	 *
	 * @param type	The Menu_Type for which to set up the menu.
	 * @param view	The UMLView parent of the menu.
	 */
	void setupMenu(Menu_Type type, UMLView * view = 0);

	enum PixMap_Type {
		pm_Class,
		pm_Package,
		pm_Interface,
		pm_Datatype,
		pm_Enum,
		pm_Actor,
		pm_Usecase,
		pm_InitialState,
		pm_EndState,
		pm_Branch,
		pm_Object,
		pm_Component,
		pm_Node,
		pm_Artifact,
		pm_Text,
		pm_NUMBER_OF_PIXMAPS
	};
	QPixmap m_pixmap[pm_NUMBER_OF_PIXMAPS];
	KPopupMenu * m_pInsert, * m_pShow, * m_pColor;
	void setupColor(bool fc);
	void setupColorSelection(bool fc);
	void setupDiagramMenu(UMLView* view);
};

#endif
