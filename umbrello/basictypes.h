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

#include <QString>
#include <string>

/**
 * This namespace contains all the enums used all over the code base.
 * The enums are embedded into classes to add usefull functionality.
 */
namespace Uml
{

    /**
     * The model type enum is used to identify the folder the diagrams belong to.
     */
    namespace ModelType
    {
        enum Enum {
            Logical,
            UseCase,
            Component,
            Deployment,
            EntityRelationship,
            N_MODELTYPES   // must remain last
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * The visibility enum defines the visibility of attributes and operations.
     */
    namespace Visibility
    {
        enum Enum {
            Public,
            Private,
            Protected,
            Implementation, // objects marked with this are declared in the implementation file.
            FromParent = 3  // alias for Implementation, used by CodeGenerationPolicy
        };
        QString toString(Enum item, bool mnemonic = false);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * Supported diagram types.
     */
    namespace DiagramType
    {
        enum Enum {
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
        QString toString(Enum item);
        QString toStringI18n(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * Association types.
     */
    namespace AssociationType
    {
        enum Enum {
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
        QString toString(Enum item);
        QString toStringI18n(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
        bool hasUMLRepresentation(Enum item);
    }

    /**
     * Signature types.
     */
    namespace SignatureType
    {
        enum Enum {
            NoSig  =  600,
            ShowSig,
            SigNoVis,
            NoSigNoVis
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * TextRole types.
     */
    namespace TextRole
    {
        enum Enum {
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
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

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
        /*explicit*/ Changeability(Value item);
        static QString toString(Value item);
        static Changeability fromString(const QString& item);
        QString toString() const;
        operator Value() const;
    private:
        Value m_value;
    };

    /**
     * SequenceMessage type
     */
    namespace SequenceMessage
    {
        enum Enum {
            //This is saved out to the file so only add new entries at the end
            Synchronous = 1000,
            Asynchronous,
            Creation,
            Lost,
            Found
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

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
    namespace ProgrammingLanguage
    {
        enum Enum {
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
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

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
    namespace ID
    {
        typedef std::string Type;

        const Type None     = "-1";   ///< special value for uninitialized ID
        const Type Reserved = "0";    ///< special value for illegal ID

        #define STR2ID(id)  qPrintable(id)
        #define ID2STR(id)  QString(id.c_str())
        QDebug operator<<(QDebug out, ID::Type &type);
    }

}  // end namespace Uml

#endif
