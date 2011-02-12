/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLNAMESPACE_H
#define UMLNAMESPACE_H

#include <string>
#include <QtCore/QString>

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Uml
{

    enum Model_Type
    {
        mt_Logical,
        mt_UseCase,
        mt_Component,
        mt_Deployment,
        mt_EntityRelationship,
        N_MODELTYPES   // must remain last
    };

    enum Object_Type
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
        ot_Stereotype,
        ot_Role,
        ot_Entity,
        ot_EntityAttribute,
        ot_Folder,
        ot_EntityConstraint,
        ot_UniqueConstraint,
        ot_ForeignKeyConstraint,
        ot_CheckConstraint,
        ot_Category
    };

    class Visibility {
    public:
        enum Value {
            Public,
            Private,
            Protected,
            Implementation, // objects marked with this are declared in the implementation file.
            FromParent = 3  // alias for Implementation, used by CodeGenerationPolicy
        };
        Visibility();
        Visibility(Value v);
        static QString toString(Value value, bool mnemonic = false);
        static Visibility fromString(const QString& vis);

        QString toString(bool mnemonic = false) const;
        operator Value () const;
    private:
        Value _v;
    };

    enum Widget_Type
    {
        wt_UMLWidget = 300,         // does not have UMLObject representation
        wt_Actor,                   // has UMLObject representation
        wt_UseCase,                 // has UMLObject representation
        wt_Class,                   // has UMLObject representation
        wt_Interface,               // has UMLObject representation
        wt_Datatype,                // has UMLObject representation
        wt_Enum,                    // has UMLObject representation
        wt_Entity,                  // has UMLObject representation
        wt_Package,                 // has UMLObject representation
        wt_Object,                  // has UMLObject representation
        wt_Note,                    // does not have UMLObject representation
        wt_Box,                     // does not have UMLObject representation
        wt_Message,                 // does not have UMLObject representation
        wt_Text,                    // does not have UMLObject representation
        wt_State,                   // does not have UMLObject representation
        wt_Activity,                // does not have UMLObject representation
        wt_Component,               // has UMLObject representation
        wt_Artifact,                // has UMLObject representation
        wt_Node,                    // has UMLObject representation
        wt_Association,             // has UMLObject representation
        wt_ForkJoin,                // does not have UMLObject representation
        wt_Precondition,            // does not have UMLObject representation
        wt_CombinedFragment,        // does not have UMLObject representation
        wt_FloatingDashLine,        // does not have UMLObject representation
        wt_Signal,                  // does not have UMLObject representation
        wt_Pin,
        wt_ObjectNode,
        wt_Region,
        wt_Category                 // has UMLObject representation
    };

    enum Diagram_Type
    {
        //the values in this enum are saved out to the file
        //for file compatibility, only add new values to the end
        dt_Undefined = 0,
        dt_Class,
        dt_UseCase,
        dt_Sequence,
        dt_Collaboration,
        dt_State,
        dt_Activity,
        dt_Component,
        dt_Deployment,
        dt_EntityRelationship,
        N_DIAGRAMTYPES   // must remain last
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
        at_Exception,
        at_Category2Parent,
        at_Child2Category,
        at_Relationship,
        at_Unknown  =  - 1
    };

    enum Signature_Type
    {
        st_NoSig  =  600,
        st_ShowSig,
        st_SigNoVis,
        st_NoSigNoVis
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
        lvt_Diagrams,  // currently unused
        lvt_Artifact,
        lvt_Deployment_Diagram,
        lvt_Deployment_Folder,
        lvt_Deployment_View,
        lvt_Node,
        lvt_Datatype,
        lvt_Datatype_Folder,
        lvt_Enum,
        lvt_Entity,
        lvt_EntityAttribute,
        lvt_EntityRelationship_Diagram,
        lvt_EntityRelationship_Folder,
        lvt_EntityRelationship_Model,
        lvt_Subsystem,
        lvt_Model,
        lvt_EnumLiteral,
        lvt_UniqueConstraint,
        lvt_PrimaryKeyConstraint,
        lvt_ForeignKeyConstraint,
        lvt_CheckConstraint,
        lvt_Category,
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
        sequence_message_asynchronous,
        sequence_message_creation,
        sequence_message_lost,
        sequence_message_found
    };

    enum DBIndex_Type
    {
        None  =  1100,
        Primary,
        Index,
        Unique
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
    enum Parameter_Direction { pd_In, pd_InOut, pd_Out };

    /**
     * Supported programming languages
     */
    enum Programming_Language {
        pl_ActionScript,
        pl_Ada,
        pl_Cpp,
        pl_CSharp,
        pl_D,
        pl_IDL,
        pl_Java,
        pl_JavaScript,
        pl_MySQL,
        pl_Pascal,
        pl_Perl,
        pl_PHP,
        pl_PHP5,
        pl_PostgreSQL,
        pl_Python,
        pl_Ruby,
        pl_SQL,
        pl_Tcl,
        pl_Vala,
        pl_XMLSchema,
        pl_Reserved
    };

    /**
     * Enumeration used for stating where a line is on a widget.
     * @note Do not change this ordering, as we use these values in for loop.
     */
    enum Region {
        reg_Error = 0,
        reg_West,
        reg_North,
        reg_East,
        reg_South,
        reg_NorthWest,
        reg_NorthEast,
        reg_SouthEast,
        reg_SouthWest
    };

    enum Corner {
        corner_TopLeft = 0x1,
        corner_TopRight = 0x2,
        corner_BottomRight = 0x4,
        corner_BottomLeft = 0x8
    };
    Q_DECLARE_FLAGS(Corners, Corner)

    /**
     * The data type used for unique IDs.
     */
    typedef std::string IDType;

    /**
     * Reserved value for uninitialized/illegal ID.
     */
    const IDType id_None = "-1";
    const IDType id_Reserved = "0";

    bool tagEq (const QString& tag, const QString& pattern);

}  // end namespace Uml

# define STR2ID(id)  qPrintable(id)
# define ID2STR(id)  QString(id.c_str())

#include "debug_utils.h"

#endif
