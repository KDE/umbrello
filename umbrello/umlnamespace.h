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

#include <qstring.h>

//uncomment for use with Luis' diagram code
//namespace Umbrello
//{
//enum Visibility {Public = 200, Private, Protected };
//}


/**
 *@author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Uml {
	enum UMLObject_Type
	{
	    ot_UMLObject  = 100,
	    ot_Actor,
	    ot_UseCase,
	    ot_Package,
	    ot_Interface,
	    ot_Datatype,
	    ot_Enum,
	    ot_Class,
	    ot_Association,
	    ot_Attribute,
	    ot_Operation,
	    ot_EnumLiteral,
	    ot_Template,
	    ot_Component,
	    ot_Artifact,
	    ot_Node,
	    ot_Stereotype
	};

	enum Scope
	{
	    Public  =  200,
	    Private,
	    Protected
	};

	enum UMLWidget_Type
	{
	    wt_UMLWidget  =  300,	// does not have UMLObject representation
	    wt_Actor,			// has UMLObject representation
	    wt_UseCase,			// has UMLObject representation
	    wt_Class,			// has UMLObject representation
	    wt_Interface,		// has UMLObject representation
	    wt_Datatype,		// has UMLObject representation
	    wt_Enum,			// has UMLObject representation
	    wt_Package,			// has UMLObject representation
	    wt_Object,			// has UMLObject representation
	    wt_Note,			// does not have UMLObject representation
	    wt_Box,			// does not have UMLObject representation
	    wt_Message,			// does not have UMLObject representation
	    wt_Text,			// does not have UMLObject representation
	    wt_State,			// does not have UMLObject representation
	    wt_Activity,		// does not have UMLObject representation
	    wt_Component,		// has UMLObject representation
	    wt_Artifact,		// has UMLObject representation
	    wt_Node			// has UMLObject representation
	};

	enum Diagram_Type
	{
		//the values in this enum are saved out to the file
		//for file compatibility, only add new values to the end
	    dt_UseCase  =  400,
	    dt_Collaboration,
	    dt_Class,
	    dt_Sequence,
	    dt_State,
	    dt_Activity,
	    dt_Component,
	    dt_Deployment,
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
	    at_Containment,
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
	    tr_Floating  =  700,   //text widget on diagrams
	    tr_MultiA,             //Text for Multiple A
	    tr_MultiB,             //Text for Multiple B
	    tr_Name,               //middle text on most associations
	    tr_Seq_Message,        //message on seq diagram between two objects
	    tr_Seq_Message_Self,   //message to self on seq diagram - feature not implemented yet
	    tr_Coll_Message,       //message between two objects on a collab diagram
	    tr_Coll_Message_Self,  //message to object self on collab diagram
	    tr_State,
	    tr_RoleAName,          //RoleA text on associations
	    tr_RoleBName,          //RoleB text on associations
	    tr_ChangeA,            //Changeability A text on associations
	    tr_ChangeB             //Changeability B text on associations
	};

	enum ListView_Type
	{
		//the values in this enum are saved out to the file
		//for file compatibility, only add new values to the end
	    lvt_View  =  800,
	    lvt_Logical_View,
	    lvt_UseCase_View,
	    lvt_Logical_Folder,
	    lvt_UseCase_Folder,
	    lvt_UseCase_Diagram,
	    lvt_Collaboration_Diagram,
	    lvt_Class_Diagram,
	    lvt_State_Diagram,
	    lvt_Activity_Diagram,
	    lvt_Sequence_Diagram,
	    lvt_Actor,
	    lvt_UseCase,
	    lvt_Class,
	    lvt_Attribute,
	    lvt_Operation,
	    lvt_Template,
	    lvt_Interface,
	    lvt_Package,
	    lvt_Component_Diagram,
	    lvt_Component_Folder,
	    lvt_Component_View,
	    lvt_Component,
	    lvt_Diagrams,
	    lvt_Artifact,
	    lvt_Deployment_Diagram,
	    lvt_Deployment_Folder,
	    lvt_Deployment_View,
	    lvt_Node,
	    lvt_Datatype,
	    lvt_Datatype_Folder,
	    lvt_Enum,
	    lvt_Unknown = -1
	};

	enum Changeability_Type
	{
		chg_Changeable = 900,
		chg_Frozen,
		chg_AddOnly
	};

	enum Sequence_Message_Type
	{
		//This is saved out to the file so only add new entries at the end
		sequence_message_synchronous = 1000,
		sequence_message_asynchronous
	};

	/**
	 * Constants used for indexing the roles of associations. 
	 */
	enum Role_Type { A, B };

	/**
	 * Direction of operation parameters:
	 *   in = operation uses the parameter as an input value
	 *   out = operation fills the parameter as a return value
	 *   inout = operation both reads and writes the parameter
	 * The numeric values of this enum are not currently saved to file.
	 */
	enum Parameter_Kind { pk_In, pk_InOut, pk_Out };

	/**
	 * Function for comparing tags in XMI files.
	 */
	bool tagEq (QString tag, QString pattern);

}

#endif
