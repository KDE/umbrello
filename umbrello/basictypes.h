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

// qt includes
#include <QFont>
#include <QString>

#include <string>

/**
 * This namespace contains all the enums used all over the code base.
 * The enums are embedded into namespaces and useful functionality is added.
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

    namespace TypeQualifiers {

        enum Enum{
            None,
            Const,
            Volatile,
            Mutable,
            ConstVolatile
        };

        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    namespace TypeModifiers{
        enum Enum{
            None,
            Pointer,
            Reference
        };

        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInte(int item);
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
        // only append entries to this type
        // it is used as xmi file attribute
        enum Enum {
            Generalization  =  500,
            Aggregation,
            Dependency,
            Association,
            Association_Self,
            Coll_Message_Asynchronous,
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
            Coll_Message_Synchronous,
            // enter new entries before this line
            Reserved,
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
            ChangeB,            //Changeability B text on associations
            Reserved            //Enter new entries before this line
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * Changeability types.
     */
    namespace Changeability
    {
        enum Enum {
            Changeable = 900,
            Frozen,
            AddOnly
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

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
    namespace RoleType
    {
        enum Enum {
            A,
            B
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * Direction of operation parameters:
     *   in = operation uses the parameter as an input value
     *   out = operation fills the parameter as a return value
     *   inout = operation both reads and writes the parameter
     * The numeric values of this enum are not currently saved to file.
     */
    namespace ParameterDirection
    {
        enum Enum {
            In,
            InOut,
            Out
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

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
        QStringList toExtensions(Enum item);
        QString toExtensionsDescription(Enum item);
    }

    /**
     * Enumeration used for stating where a line is on a widget.
     * @note Do not change this ordering, as we use these values in for loop.
     * @note See also associationwidget.h.
     */
    namespace Region
    {
        enum Enum {
            Error = 0,
            West,
            North,
            East,
            South,
            NorthWest,
            NorthEast,
            SouthEast,
            SouthWest,
            Center
        };
        QString toString(Enum item);
        Enum fromString(const QString& item);
        Enum fromInt(int item);
    }

    /**
     * Corner class with special operators.
     */
    class Corner
    {
    public:
        enum Enum {
            TopLeft     = 0x1,
            TopRight    = 0x2,
            BottomRight = 0x4,
            BottomLeft  = 0x8
        };
        static QString toString(Enum item);
        static Enum fromString(const QString& item);
        static Enum fromInt(int item);
    };
    Q_DECLARE_FLAGS(Corners, Corner::Enum)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Corners)

    /**
     * The data type used for unique IDs.
     */
    namespace ID
    {
        typedef std::string Type;

        const Type None     = "-1";   ///< special value for uninitialized ID
        const Type Reserved = "0";    ///< special value for illegal ID

        QString toString(const ID::Type &id);
        ID::Type fromString(const QString &id);
        QDebug operator<<(QDebug out, ID::Type &type);
    }

QFont systemFont();

}  // end namespace Uml

static inline QString toString(Uml::ProgrammingLanguage::Enum lang)
{
    return Uml::ProgrammingLanguage::toString(lang);
}

static inline QString toString(Uml::Visibility::Enum visibility)
{
    return Uml::Visibility::toString(visibility);
}

qreal toDoubleFromAnyLocale(const QString &s);

#endif
