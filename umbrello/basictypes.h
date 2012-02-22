/***************************************************************************
 * Copyright (C) 2011 by Andi Fischer <andi.fischer@hispeed.ch>            *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#ifndef BASICTYPES_H
#define BASICTYPES_H

#include <QtCore/QString>
#include <string>

/**
 * @author Andi Fischer
 * ...
 */
namespace Uml
{

  /**
   * 
   */
  class ModelType
    {
    public:
        enum Value {
            Logical,
            UseCase,
            Component,
            Deployment,
            EntityRelationship,
            N_MODELTYPES   // must remain last
        };
        ModelType();
        ModelType(Value item);
        static QString toString(Value item);
        static ModelType fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * 
     */
    class Visibility
    {
    public:
        enum Value {
            Public,
            Private,
            Protected,
            Implementation, // objects marked with this are declared in the implementation file.
            FromParent = 3  // alias for Implementation, used by CodeGenerationPolicy
        };
        Visibility();
        Visibility(Value item);
        static QString toString(Value item, bool mnemonic = false);
        static Visibility fromString(const QString& item);
        QString toString(bool mnemonic = false) const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * Supported diagram types.
     */
    class DiagramType
    {
    public:
        enum Value {
            //the values in this enum are saved out to the file
            //for file compatibility, only add new values to the end
            Undefined = 0,
            Class,
            UseCase,
            Sequence,
            Collaboration,
            State,
            Activity,
            Component,
            Deployment,
            EntityRelationship,
            N_DIAGRAMTYPES   // must remain last
        };
        DiagramType();
        DiagramType(Value item);
        static QString toString(Value item);
        static DiagramType fromString(const QString& item);
        QString toString() const;
        QString toStringI18n() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * Association types.
     */
    class AssociationType
    {
    public:
        enum Value {
            Generalization  =  500,
            Aggregation,
            Dependency,
            Association,
            Association_Self,
            Coll_Message,
            Seq_Message,
            Coll_Message_Self,
            Seq_Message_Self,
            Containment,
            Composition,
            Realization,
            UniAssociation,
            Anchor,
            State,
            Activity,
            Exception,
            Category2Parent,
            Child2Category,
            Relationship,
            Unknown  =  - 1
        };
        AssociationType();
        AssociationType(Value item);
        static QString toString(Value item);
        static AssociationType fromString(const QString& item);
        QString toString() const;
        QString toStringI18n() const;
        operator Value() const;
        static bool hasUMLRepresentation(Value item);
    private:
        Value m_value;
    };

    /**
     * Signature types.
     */
    class SignatureType
    {
    public:
        enum Value {
            NoSig  =  600,
            ShowSig,
            SigNoVis,
            NoSigNoVis
        };
        SignatureType();
        SignatureType(Value item);
        static QString toString(Value item);
        static SignatureType fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * TextRole types.
     */
    class TextRole
    {
    public:
        enum Value {
            Floating  =  700,   //text widget on diagrams
            MultiA,             //Text for Multiple A
            MultiB,             //Text for Multiple B
            Name,               //middle text on most associations
            Seq_Message,        //message on seq diagram between two objects
            Seq_Message_Self,   //message to self on seq diagram - feature not implemented yet
            Coll_Message,       //message between two objects on a collab diagram
            Coll_Message_Self,  //message to object self on collab diagram
            State,
            RoleAName,          //RoleA text on associations
            RoleBName,          //RoleB text on associations
            ChangeA,            //Changeability A text on associations
            ChangeB             //Changeability B text on associations
        };
        TextRole();
        TextRole(Value item);
        static QString toString(Value item);
        static TextRole fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * Changeability types.
     */
    class Changeability
    {
    public:
        enum Value {
            Changeable = 900,
            Frozen,
            AddOnly
        };
        Changeability();
        Changeability(Value item);
        static QString toString(Value item);
        static Changeability fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * 
     */
    enum Sequence_Message_Type
    {
        //This is saved out to the file so only add new entries at the end
        sequence_message_synchronous = 1000,
        sequence_message_asynchronous,
        sequence_message_creation,
        sequence_message_lost,
        sequence_message_found
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
     * Supported programming languages.
     */
    class ProgrammingLanguage
    {
    public:
        enum Value {
            ActionScript,
            Ada,
            Cpp,
            CSharp,
            D,
            IDL,
            Java,
            JavaScript,
            MySQL,
            Pascal,
            Perl,
            PHP,
            PHP5,
            PostgreSQL,
            Python,
            Ruby,
            SQL,
            Tcl,
            Vala,
            XMLSchema,
            Reserved
        };
        ProgrammingLanguage();
        ProgrammingLanguage(Value item);
        static QString toString(Value item);
        static ProgrammingLanguage fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * Enumeration used for stating where a line is on a widget.
     * @note Do not change this ordering, as we use these values in for loop.
     */
//     enum Region {
//         Error = 0,
//         West,
//         North,
//         East,
//         South,
//         NorthWest,
//         NorthEast,
//         SouthEast,
//         SouthWest
//     };

    /**
     * 
     */
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
//    class IDType
//    {
    typedef std::string IDType;

    const IDType id_None     = "-1";   ///< special value for uninitialized ID
    const IDType id_Reserved = "0";    ///< special value for illegal ID

# define STR2ID(id)  qPrintable(id)
# define ID2STR(id)  QString(id.c_str())
    QDebug operator<<(QDebug out, IDType &type);
//    }

}  // end namespace Uml

#endif
