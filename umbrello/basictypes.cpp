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

#include "basictypes.h"

#include "debug_utils.h"

#include <klocale.h>

#include <QRegExp>

namespace Uml
{

namespace ModelType
{

/**
 * Convert ModelType item into QString representation.
 * @param item       item to convert
 * @return QString representation of ModelType
 */
QString toString(Enum item)
{
    switch (item) {
        case UseCase:
            return QString("UseCase");
        case Component:
            return QString("Component");
        case Deployment:
            return QString("Deployment");
        case EntityRelationship:
            return QString("EntityRelationship");
        case Logical:
        default:
            return QString("Logical");
    }
}

/**
 * Convert a string item into Model representation.
 * @param item   item to convert
 * @return Model object
 */
Enum fromString(const QString& item)
{
    if (item == "UseCase")
        return UseCase;
    else if (item == "Component")
        return Component;
    else if (item == "Deployment")
        return Deployment;
    else if (item == "EntityRelationship")
        return EntityRelationship;
    else
        return Logical;
}

/**
 * Convert a integer item into ModelType representation.
 * @param item   integer value to convert
 * @return ModelType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace ModelType

//-----------------------------------------------------------------------------

namespace Visibility {

/**
 * Convert Visibility item into QString representation.
 * @param item       item to convert
 * @param mnemonic   if true then return a single character:
 *              "+" for public, "-" for private,
 *              "#" for protected or "~" for implementation
 * @return QString representation of Visibility
 */
QString toString(Enum item, bool mnemonic)
{
    switch (item) {
        case Protected:
            return (mnemonic ? QString("#") : QString("protected"));
        case Private:
            return (mnemonic ? QString("-") : QString("private"));
        case Implementation:
            return (mnemonic ? QString("~") : QString("implementation"));
        case Public:
        default:
            return (mnemonic ? QString("+") : QString("public"));
    }
}

/**
 * Convert a string item into Visibility representation.
 * @param item   item to convert
 * @return Visibility enum
 */
Enum fromString(const QString& item)
{
    if (item == "public" || item == "+")
        return Public;
    else if (item == "protected" || item == "#")
        return Protected;
    else if (item == "private" || item == "-")
        return Private;
    else if (item == "~")
        return Implementation;
    else if (item == "signals")
        return Protected;
    else if (item == "class")
        return Private;
    else
        return Public;
}

/**
 * Convert a integer item into Visibility representation.
 * @param item   integer value to convert
 * @return Visibility enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace Visibility

//-----------------------------------------------------------------------------

namespace DiagramType {

/**
 * Convert DiagramType item into QString representation.
 * @param item   item to convert
 * @return QString representation of DiagramType
 */
QString toString(Enum item)
{
    switch (item) {
        case Undefined:
            return QString("Undefined");
        case Class:
            return QString("Class");
        case UseCase:
            return QString("UseCase");
        case Sequence:
            return QString("Sequence");
        case Collaboration:
            return QString("Collaboration");
        case State:
            return QString("State");
        case Activity:
            return QString("Activity");
        case Component:
            return QString("Component");
        case Deployment:
            return QString("Deployment");
        case EntityRelationship:
            return QString("EntityRelationship");
        case N_DIAGRAMTYPES:   // must remain last
            return QString("N_DIAGRAMTYPES");
        default:
            return QString("? DiagramType ?");
    }
}

/**
 * Return string corresponding to DiagramType
 */
QString toStringI18n(Enum item)
{
    switch (item) {
       case Class:
           return i18n("Class Diagram");
       case UseCase:
           return i18n("Use Case Diagram");
       case Sequence:
           return i18n("Sequence Diagram");
       case Collaboration:
           return i18n("Collaboration Diagram");
       case State:
           return i18n("State Diagram");
       case Activity:
           return i18n("Activity Diagram");
       case Component:
           return i18n("Component Diagram");
       case Deployment:
           return i18n("Deployment Diagram");
       case EntityRelationship:
           return i18n("Entity Relationship Diagram");
       default:
           return i18n("No Diagram");
    }
}

/**
 * Convert a string item into DiagramType representation.
 * @param item   item to convert
 * @return DiagramType object
 */
Enum fromString(const QString& item)
{
    if (item == "Undefined")
        return Undefined;
    else if (item == "Class")
        return Class;
    else if (item == "UseCase")
        return UseCase;
    else if (item == "Sequence")
        return Sequence;
    else if (item == "Collaboration")
        return Collaboration;
    else if (item == "State")
        return State;
    else if (item == "Activity")
        return Activity;
    else if (item == "Component")
        return Component;
    else if (item == "Deployment")
        return Deployment;
    else if (item == "EntityRelationship")
        return EntityRelationship;
    else
        return Undefined;
}

/**
 * Convert an integer item into DiagramType representation.
 * @param item   integer value to convert
 * @return DiagramType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace DiagramType

//-----------------------------------------------------------------------------

namespace AssociationType {

/**
 * Convert AssociationType item into QString representation.
 * @param item   item to convert
 * @return QString representation of AssociationType
 */
QString toString(Enum item)
{
    switch (item) {
        case Generalization:
            return QString("Generalization");
        case Aggregation:
            return QString("Aggregation");
        case Dependency:
            return QString("Dependency");
        case Association:
            return QString("Association");
        case Association_Self:
            return QString("Association_Self");
        case Coll_Message_Asynchronous:
            return QString("Coll_Message");
        case Seq_Message:
            return QString("Seq_Message");
        case Coll_Message_Self:
            return QString("Coll_Message_Self");
        case Seq_Message_Self:
            return QString("Seq_Message_Self");
        case Containment:
            return QString("Containment");
        case Composition:
          return QString("Composition");
        case Realization:
          return QString("Realization");
        case UniAssociation:
          return QString("UniAssociation");
        case Anchor:
          return QString("Anchor");
        case State:
          return QString("State");
        case Activity:
          return QString("Activity");
        case Exception:
          return QString("Exception");
        case Category2Parent:
          return QString("Category2Parent");
        case Child2Category:
          return QString("Child2Category");
        case Relationship:
          return QString("Relationship");
        case Unknown:
            return QString("Unknown");
        default:
            return QString("? AssociationType ?");
    }
}

/**
 * Converts a AssociationType to its string representation.
 * @return  the string representation of the AssociationType
 */
QString toStringI18n(Enum item)
{
    switch (item) {
          case Generalization:
              return i18n("Generalization");
          case Aggregation:
              return i18n("Aggregation");
          case Dependency:
              return i18n("Dependency");
          case Association:
              return i18n("Association");
          case Association_Self:
              return i18n("Self Association");
          case Coll_Message_Asynchronous:
              return i18n("Collaboration Message");
          case Seq_Message:
              return i18n("Sequence Message");
          case Coll_Message_Self:
              return i18n("Collaboration Self Message");
          case Seq_Message_Self:
              return i18n("Sequence Self Message");
          case Containment:
              return i18n("Containment");
          case Composition:
              return i18n("Composition");
          case Realization:
              return i18n("Realization");
          case UniAssociation:
              return i18n("Uni Association");
          case Anchor:
              return i18n("Anchor");
          case State:
              return i18n("State Transition");
          case Activity:
              return i18n("Activity");
          case Exception:
              return i18n("Exception");
          case Category2Parent:
              return i18n("Category to Parent");
          case Child2Category:
              return i18n("Child to Category");
          case Relationship:
              return i18n("Relationship");
          case Unknown:
              return i18n("Unknown");
          default:
              return i18n("? AssociationType ?");
      };
}

/**
 * Convert a string item into AssociationType representation.
 * @param item   item to convert
 * @return AssociationType object
 */
Enum fromString(const QString& item)
{
    if (item == "Generalization")
        return Generalization;
    else if (item == "Aggregation")
        return Aggregation;
    else if (item == "Dependency")
        return Dependency;
    else if (item == "Association")
        return Association;
    else if (item == "Association_Self")
        return Association_Self;
    else if (item == "Coll_Message")
        return Coll_Message_Asynchronous;
    else if (item == "Seq_Message")
        return Seq_Message;
    else if (item == "Coll_Message_Self")
        return Coll_Message_Self;
    else if (item == "Seq_Message_Self")
        return Seq_Message_Self;
    else if (item == "Containment")
        return Containment;
    else if (item == "Composition")
        return Composition;
    else if (item == "Realization")
        return Realization;
    else if (item == "UniAssociation")
        return UniAssociation;
    else if (item == "Anchor")
        return Anchor;
    else if (item == "State")
        return State;
    else if (item == "Activity")
        return Activity;
    else if (item == "Exception")
        return Exception;
    else if (item == "Category2Parent")
        return Category2Parent;
    else if (item == "Child2Category")
        return Child2Category;
    else if (item == "Relationship")
        return Relationship;
    else
        return Unknown;
}

/**
 * Convert a integer item into ProgrammingLanguage representation.
 * @param item   integer value to convert
 * @return AssociationType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

/**
 * Returns true if the given AssociationType has a representation as a
 * UMLAssociation.
 * @param item   the AssociationType enum value to check
 * @return  boolean value
 */
bool hasUMLRepresentation(Enum item)
{
    return (item == Generalization   ||
            item == Realization      ||
            item == Association      ||
            item == Association_Self ||
            item == UniAssociation   ||
            item == Aggregation      ||
            item == Relationship     ||
            item == Composition      ||
            item == Dependency       ||
            item == Category2Parent  ||
            item == Child2Category);
}

}  // end namespace AssociationType

//-----------------------------------------------------------------------------

namespace SignatureType {

/**
 * Return string corresponding to the given SignatureType.
 */
QString toString(Enum item)
{
    switch (item) {
        case NoSig:
            return QString("NoSig");
        case ShowSig:
            return QString("ShowSig");
        case SigNoVis:
            return QString("SigNoVis");
        case NoSigNoVis:
            return QString("NoSigNoVis");
        default:
            break;
    }
    return QString();
}

/**
 * Return SignatureType corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "NoSig")
        return NoSig;
    if (item == "ShowSig")
        return ShowSig;
    if (item == "SigNoVis")
        return SigNoVis;
    if (item == "NoSigNoVis")
        return NoSigNoVis;
    return NoSig;
}

/**
 * Convert a integer item into SignatureType representation.
 * @param item   integer value to convert
 * @return SignatureType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace SignatureType

//-----------------------------------------------------------------------------

namespace TextRole {

/**
 * Return string corresponding to the given TextRole.
 */
QString toString(Enum item)
{
    switch (item) {
        case Floating:
            return QString("Floating");
        case MultiA:
            return QString("MultiA");
        case MultiB:
            return QString("MultiB");
        case Name:
            return QString("Name");
        case Seq_Message:
            return QString("Seq_Message");
        case Seq_Message_Self:
            return QString("Seq_Message_Self");
        case Coll_Message:
            return QString("Coll_Message");
        case Coll_Message_Self:
            return QString("Coll_Message_Self");
        case State:
            return QString("State");
        case RoleAName:
            return QString("RoleAName");
        case RoleBName:
            return QString("RoleBName");
        case ChangeA:
            return QString("ChangeA");
        case ChangeB:
            return QString("ChangeB");
        default:
            break;
    }
    return QString("? TextRole ?");
}

/**
 * Return TextRole corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "Floating")
        return Floating;
    if (item == "MultiA")
        return MultiA;
    if (item == "MultiB")
        return MultiB;
    if (item == "Name")
        return Name;
    if (item == "Seq_Message")
        return Seq_Message;
    if (item == "Seq_Message_Self")
        return Seq_Message_Self;
    if (item == "Coll_Message")
        return Coll_Message;
    if (item == "Coll_Message_Self")
        return Coll_Message_Self;
    if (item == "State")
        return State;
    if (item == "RoleAName")
        return RoleAName;
    if (item == "RoleBName")
        return RoleBName;
    if (item == "ChangeA")
        return ChangeA;
    if (item == "ChangeB")
        return ChangeB;
    return Floating;
}

/**
 * Convert a integer item into TextRole representation.
 * @param item   integer value to convert
 * @return TextRole enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace TextRole

//-----------------------------------------------------------------------------

namespace Changeability {

/**
 * Convert Changeability::Enum value into QString representation.
 * @param type   The Changeability enum value to convert.
 */
QString toString(Enum item)
{
    switch (item) {
        case Changeability::Frozen:
            return QString("frozen");
        case Changeability::AddOnly:
            return QString("addOnly");
        case Changeability::Changeable:
            return QString("changeable");
        default:
            break;
    }
    return QString("? Changeability ?");
}

/**
 * Return Changeability::Enum corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "frozen")
        return Frozen;
    if (item == "addOnly")
        return AddOnly;
    if (item == "changeable")
        return Changeable;
    return Changeable;
}

/**
 * Convert a integer item into Changeability representation.
 * @param item   integer value to convert
 * @return Changeability enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace Changeability

//-----------------------------------------------------------------------------

namespace SequenceMessage {

/**
 * Return string corresponding to the given SequenceMessage.
 */
QString toString(Enum item)
{
    switch (item) {
        case Synchronous:
            return QString("Synchronous");
        case Asynchronous:
            return QString("Asynchronous");
        case Creation:
            return QString("Creation");
        case Lost:
            return QString("Lost");
        case Found:
            return QString("Found");
        default:
            break;
    }
    return QString("? SequenceMessage ?");
}

/**
 * Return SequenceMessage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "Synchronous")
        return Synchronous;
    if (item == "Asynchronous")
        return Asynchronous;
    if (item == "Creation")
        return Creation;
    if (item == "Lost")
        return Lost;
    if (item == "Found")
        return Found;
    return Synchronous;
}

/**
 * Convert a integer item into SequenceMessage representation.
 * @param item   integer value to convert
 * @return SequenceMessage enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace SequenceMessage

//-----------------------------------------------------------------------------

namespace RoleType {

/**
 * Return string corresponding to the given RoleType.
 */
QString toString(Enum item)
{
    switch (item) {
        case A:
            return QString("A");
        case B:
            return QString("B");
        default:
            break;
    }
    return QString("? RoleType ?");
}

/**
 * Return RoleType corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "A")
        return A;
    if (item == "B")
        return B;
    return A;
}

/**
 * Convert a integer item into RoleType representation.
 * @param item   integer value to convert
 * @return RoleType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace RoleType

//-----------------------------------------------------------------------------

namespace ParameterDirection {

/**
 * Return string corresponding to the given ParameterDirection.
 */
QString toString(Enum item)
{
    switch (item) {
        case In:
            return QString("In");
        case InOut:
            return QString("InOut");
        case Out:
            return QString("Out");
        default:
            break;
    }
    return QString("? ParameterDirection ?");
}

/**
 * Return ParameterDirection corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "In")
        return In;
    if (item == "InOut")
        return InOut;
    if (item == "Out")
        return Out;
    return In;
}

/**
 * Convert a integer item into ParameterDirection representation.
 * @param item   integer value to convert
 * @return ParameterDirection enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace ParameterDirection

//-----------------------------------------------------------------------------

namespace ProgrammingLanguage {

/**
 * Return string corresponding to the given ProgrammingLanguage.
 */
QString toString(Enum item)
{
    switch (item) {
        case ActionScript:
            return QString("ActionScript");
        case Ada:
            return QString("Ada");
        case Cpp:
            return QString("C++");
        case CSharp:
            return QString("C#");
        case D:
            return QString("D");
        case IDL:
            return QString("IDL");
        case Java:
            return QString("Java");
        case JavaScript:
            return QString("JavaScript");
        case MySQL:
            return QString("MySQL");
        case Pascal:
            return QString("Pascal");
        case Perl:
            return QString("Perl");
        case PHP:
            return QString("PHP");
        case PHP5:
            return QString("PHP5");
        case PostgreSQL:
            return QString("PostgreSQL");
        case Python:
            return QString("Python");
        case Ruby:
            return QString("Ruby");
        case SQL:
            return QString("SQL");
        case Tcl:
            return QString("Tcl");
        case Vala:
            return QString("Vala");
        case XMLSchema:
            return QString("XMLSchema");
        default:
            break;
    }
    return QString();
}

/**
 * Return ProgrammingLanguage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "ActionScript")
        return ActionScript;
    if (item == "Ada")
        return Ada;
    if (item == "C++" || item == "Cpp")  // "Cpp" only for bkwd compatibility
        return Cpp;
    if (item == "C#")
        return CSharp;
    if (item == "D")
        return D;
    if (item == "IDL")
        return IDL;
    if (item == "Java")
        return Java;
    if (item == "JavaScript")
        return JavaScript;
    if (item == "MySQL")
        return MySQL;
    if (item == "Pascal")
        return Pascal;
    if (item == "Perl")
        return Perl;
    if (item == "PHP")
        return PHP;
    if (item == "PHP5")
        return PHP5;
    if (item == "PostgreSQL")
        return PostgreSQL;
    if (item == "Python")
        return Python;
    if (item == "Ruby")
        return Ruby;
    if (item == "SQL")
        return SQL;
    if (item == "Tcl")
        return Tcl;
    if (item == "Vala")
        return Vala;
    if (item == "XMLSchema")
        return XMLSchema;
    return Reserved;
}

/**
 * Convert a integer item into ProgrammingLanguage representation.
 * @param item   integer value to convert
 * @return ProgrammingLanguage enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

/**
 * Return extensions associated with the requested language.
 * @param item programming language index
 * @return extensions list
 */
QStringList toExtensions(Enum item)
{
    QStringList result;
    switch (item) {  //:TODO: More languages?
        case Uml::ProgrammingLanguage::Ada:
            result << "*.ads" << "*.adb" << "*.ada";
            break;
        case Uml::ProgrammingLanguage::Cpp:
            result << "*.h" << "*.hpp" << "*.hh" << "*.hxx" << "*.H";
            break;
        case Uml::ProgrammingLanguage::IDL:
            result << "*.idl";
            break;
        case Uml::ProgrammingLanguage::Java:
            result << "*.java";
            break;
        case Uml::ProgrammingLanguage::Pascal:
            result << "*.pas";
            break;
        case Uml::ProgrammingLanguage::Python:
            result << "*.py" << "*.pyw";
            break;
        case Uml::ProgrammingLanguage::CSharp:
            result << "*.cs";
            break;
        default:
            break;
    }
    return result;
}

/**
 * Return clear text file extension description for the requested language.
 * @param item programming language index
 * @return extension
 */
QString toExtensionsDescription(Enum item)
{
    QString result = "Files";
    switch (item) {  //:TODO: More languages?
        case Uml::ProgrammingLanguage::Ada:
            result = "Source files";
            break;
        case Uml::ProgrammingLanguage::Cpp:
            result = "Header files";
            break;
        case Uml::ProgrammingLanguage::IDL:
            result = "Source files";
            break;
        case Uml::ProgrammingLanguage::Java:
            result = "Source files";
            break;
        case Uml::ProgrammingLanguage::Pascal:
            result = "Source files";
            break;
        case Uml::ProgrammingLanguage::Python:
            result = "Source files";
            break;
        case Uml::ProgrammingLanguage::CSharp:
            result = "Source files";
            break;
        default:
            break;
    }
    return toString(item) + QLatin1String(" ") + result;
}

}  // end namespace ProgrammingLanguage

//-----------------------------------------------------------------------------

namespace Region {

/**
 * Return string corresponding to the given Region.
 */
QString toString(Enum item)
{
    switch (item) {
        case Error:
            return QString("Error");
        case West:
            return QString("West");
        case North:
            return QString("North");
        case East:
            return QString("East");
        case South:
            return QString("South");
        case NorthWest:
            return QString("NorthWest");
        case NorthEast:
            return QString("NorthEast");
        case SouthEast:
            return QString("SouthEast");
        case SouthWest:
            return QString("SouthWest");
        case Center:
            return QString("Center");
        default:
            break;
    }
    return QString("? Region ?");
}

/**
 * Return Region corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == "Error")
        return Error;
    if (item == "West")
        return West;
    if (item == "North")
        return North;
    if (item == "East")
        return East;
    if (item == "South")
        return South;
    if (item == "NorthWest")
        return NorthWest;
    if (item == "NorthEast")
        return NorthEast;
    if (item == "SouthEast")
        return SouthEast;
    if (item == "SouthWest")
        return SouthWest;
    if (item == "Center")
        return Center;
    return Error;
}

/**
 * Convert a integer item into Region representation.
 * @param item   integer value to convert
 * @return Region enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace Region

//-----------------------------------------------------------------------------

//namespace Corner {

/**
 * Return string corresponding to the given Corner.
 */
QString Corner::toString(Enum item)
{
    switch (item) {
        case TopLeft:
            return QString("TopLeft");
        case TopRight:
            return QString("TopRight");
        case BottomRight:
            return QString("BottomRight");
        case BottomLeft:
            return QString("BottomLeft");
        default:
            break;
    }
    return QString("? Corner ?");
}

/**
 * Return Corner corresponding to the given string.
 */
Corner::Enum Corner::fromString(const QString& item)
{
    if (item == "TopLeft")
        return TopLeft;
    if (item == "TopRight")
        return TopRight;
    if (item == "BottomRight")
        return BottomRight;
    if (item == "BottomLeft")
        return BottomLeft;
    return TopLeft;
}

/**
 * Convert a integer item into Corner representation.
 * @param item   integer value to convert
 * @return Corner enum
 */
Corner::Enum Corner::fromInt(int item)
{
    return Enum(item);
}

//}  // end namespace Corner

//-----------------------------------------------------------------------------

namespace ID {

QDebug operator<<(QDebug out, ID::Type &type)
{
    out.nospace() << "ID::Type: " << Uml::ID::toString(type);
    return out.space();
}

QString toString(const ID::Type &id)
{
    return QLatin1String(id.c_str());
}

ID::Type fromString(const QString &id)
{
    return qPrintable(id);
}

}  // end namespace ID

}  // end namespace Uml
