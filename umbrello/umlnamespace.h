/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLNAMESPACE_H
#define UMLNAMESPACE_H


/**
 *@author Paul Hensgen
 */
namespace Uml {
	enum UMLObject_Type
	{
	    ot_UMLObject  = 100,
	    ot_Actor,
	    ot_UseCase,
	    ot_Package,
	    ot_Interface,
	    ot_Concept,
	    ot_Association,
	    ot_Attribute,
	    ot_Operation,
	    ot_Template
	};

	enum Scope
	{
	    Public  =  200,
	    Private,
	    Protected
	};

	enum UMLWidget_Type
	{
	    wt_UMLWidget  =  300,
	    wt_Actor,
	    wt_UseCase,
	    wt_Class,
	    wt_Interface,
	    wt_Package,
	    wt_Object,
	    wt_Note,
	    wt_Message,
	    wt_Text,
	    wt_State,
	    wt_Activity
	};

	enum Diagram_Type
	{
	    dt_UseCase  =  400,
	    dt_Collaboration,
	    dt_Class,
	    dt_Sequence,
	    dt_State,
	    dt_Activity,
	    dt_Undefined = -1
	};

	enum Association_Type
	{
	    at_Generalization  =  500,
	    at_Aggregation,
	    at_Dependency,
	    at_Association,
	    at_Association_Self,
	    at_Coll_Message,
	    at_Seq_Message,
	    at_Coll_Message_Self,
	    at_Seq_Message_Self,
	    at_Implementation,
	    at_Composition,
	    at_Realization,
	    at_UniAssociation,
	    at_Anchor,
	    at_State,
	    at_Activity,
	    at_Unknown  =  - 1
	};

	enum Signature_Type
	{
	    st_NoSig  =  600,
	    st_ShowSig,
	    st_SigNoScope,
	    st_NoSigNoScope
	};

	enum Text_Role
	{
  	    tr_Floating  =  700,  //text widget on diagrams
  	    tr_MultiA,               //Text for Multiple A
  	    tr_MultiB,                //Text for Multiple B
 	    tr_Name,               //mddile text on most associations
  	    tr_Seq_Message,     //message on seq diagram between two objects
  	    tr_Seq_Message_Self,  //message to self on seq diagram - feature not implemented yet
  	    tr_Coll_Message,      //message between two objects on a collab diagram
  	    tr_Coll_Message_Self,  //message to object self on collab diagram
 	    tr_State,
 	    tr_RoleAName,          //RoleA text on associations
 	    tr_RoleBName,          //RoleB text on associations
 	    tr_ChangeA,            //Changeability A text on associations
 	    tr_ChangeB             //Changeability B text on associations
  	};
  
  	enum ListView_Type
	{
	    lvt_View  =  800,
	    lvt_Logical_View,     //main view
	    lvt_UseCase_View,  //main view
	    lvt_Logical_Folder,
	    lvt_UseCase_Folder,
	    lvt_UseCase_Diagram,
	    lvt_Collaboration_Diagram,
	    lvt_Class_Diagram,
	    lvt_State_Diagram,
	    lvt_Activity_Diagram,
	    lvt_Sequence_Diagram,//leave as last diagram
	    lvt_Actor,
	    lvt_UseCase,
	    lvt_Class,
	    lvt_Interface,
	    lvt_Package,
	    lvt_Attribute,
  	    lvt_Operation,
  	    lvt_Template,
  	    lvt_Unknown = -1
  	};
 
 	enum Changeability_Type
 	{
 		chg_Changeable = 900,
 		chg_Frozen,
 		chg_AddOnly
 	};
  };
  
#endif
