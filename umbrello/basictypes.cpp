/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "basictypes.h"

#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFontDatabase>
#include <QRegularExpression>

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
            return QString(QStringLiteral("UseCase"));
        case Component:
            return QString(QStringLiteral("Component"));
        case Deployment:
            return QString(QStringLiteral("Deployment"));
        case EntityRelationship:
            return QString(QStringLiteral("EntityRelationship"));
        case Logical:
        default:
            return QString(QStringLiteral("Logical"));
    }
}

/**
 * Convert a string item into Model representation.
 * @param item   item to convert
 * @return Model object
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("UseCase"))
        return UseCase;
    else if (item == QStringLiteral("Component"))
        return Component;
    else if (item == QStringLiteral("Deployment"))
        return Deployment;
    else if (item == QStringLiteral("EntityRelationship"))
        return EntityRelationship;
    else
        return Logical;
}

/**
 * Convert an integer item into ModelType representation.
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
            return (mnemonic ? QStringLiteral("#") : QStringLiteral("protected"));
        case Private:
            return (mnemonic ? QStringLiteral("-") : QStringLiteral("private"));
        case Implementation:
            return (mnemonic ? QStringLiteral("~") : QStringLiteral("implementation"));
        case Public:
        default:
            return (mnemonic ? QStringLiteral("+") : QStringLiteral("public"));
    }
}

/**
 * Convert a string item into Visibility representation.
 * @param item   item to convert
 * @param checkUnknown  If the visibility expression in @p item is not recognized
 *                      and checkUnknown is true then the value Unknown is returned.
 * @return Visibility enum
 */
Enum fromString(const QString& item, bool checkUnknown)
{
    if (item == QStringLiteral("public") || item == QStringLiteral("+"))
        return Public;
    else if (item == QStringLiteral("protected") || item == QStringLiteral("#"))
        return Protected;
    else if (item == QStringLiteral("private") || item == QStringLiteral("-"))
        return Private;
    else if (item == QStringLiteral("~"))
        return Implementation;
    else if (item == QStringLiteral("signals"))
        return Protected;
    else if (item == QStringLiteral("class"))
        return Private;
    else if (checkUnknown)
        return Unknown;
    else
        return Public;
}

/**
 * Convert an integer item into Visibility representation.
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
            return QStringLiteral("Undefined");
        case Class:
            return QStringLiteral("Class");
        case Object:
            return QStringLiteral("Object");
        case UseCase:
            return QStringLiteral("UseCase");
        case Sequence:
            return QStringLiteral("Sequence");
        case Collaboration:
            return QStringLiteral("Collaboration");
        case State:
            return QStringLiteral("State");
        case Activity:
            return QStringLiteral("Activity");
        case Component:
            return QStringLiteral("Component");
        case Deployment:
            return QStringLiteral("Deployment");
        case EntityRelationship:
            return QStringLiteral("EntityRelationship");
        case N_DIAGRAMTYPES:   // must remain last
            return QStringLiteral("N_DIAGRAMTYPES");
        default:
            return QStringLiteral("? DiagramType ?");
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
        case Object:
            return i18n("Object Diagram");
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
    if (item == QStringLiteral("Undefined"))
        return Undefined;
    else if (item == QStringLiteral("Class"))
        return Class;
    else if (item == QStringLiteral("UseCase"))
        return UseCase;
    else if (item == QStringLiteral("Sequence"))
        return Sequence;
    else if (item == QStringLiteral("Collaboration"))
        return Collaboration;
    else if (item == QStringLiteral("State"))
        return State;
    else if (item == QStringLiteral("Activity"))
        return Activity;
    else if (item == QStringLiteral("Component"))
        return Component;
    else if (item == QStringLiteral("Deployment"))
        return Deployment;
    else if (item == QStringLiteral("EntityRelationship"))
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
            return QStringLiteral("Generalization");
        case Aggregation:
            return QStringLiteral("Aggregation");
        case Dependency:
            return QStringLiteral("Dependency");
        case Association:
            return QStringLiteral("Association");
        case Association_Self:
            return QStringLiteral("Association_Self");
        case Coll_Mesg_Async:
            return QStringLiteral("Coll_Mesg_Async");
        case Coll_Mesg_Sync:
            return QStringLiteral("Coll_Mesg_Sync");
        case Seq_Message:
            return QStringLiteral("Seq_Message");
        case Coll_Mesg_Self:
            return QStringLiteral("Coll_Mesg_Self");
        case Seq_Message_Self:
            return QStringLiteral("Seq_Message_Self");
        case Containment:
            return QStringLiteral("Containment");
        case Composition:
          return QStringLiteral("Composition");
        case Realization:
          return QStringLiteral("Realization");
        case UniAssociation:
          return QStringLiteral("UniAssociation");
        case Anchor:
          return QStringLiteral("Anchor");
        case State:
          return QStringLiteral("State");
        case Activity:
          return QStringLiteral("Activity");
        case Exception:
          return QStringLiteral("Exception");
        case Category2Parent:
          return QStringLiteral("Category2Parent");
        case Child2Category:
          return QStringLiteral("Child2Category");
        case Relationship:
          return QStringLiteral("Relationship");
        case Unknown:
            return QStringLiteral("Unknown");
        default:
            return QStringLiteral("? AssociationType ?");
    }
}

/**
 * Converts an AssociationType to its string representation.
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
          case Coll_Mesg_Async:
              return i18n("Collaboration Asynchronous Message");
          case Coll_Mesg_Sync:
              return i18n("Collaboration Synchronous Message");
          case Seq_Message:
              return i18n("Sequence Message");
          case Coll_Mesg_Self:
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
    if (item == QStringLiteral("Generalization"))
        return Generalization;
    else if (item == QStringLiteral("Aggregation"))
        return Aggregation;
    else if (item == QStringLiteral("Dependency"))
        return Dependency;
    else if (item == QStringLiteral("Association"))
        return Association;
    else if (item == QStringLiteral("Association_Self"))
        return Association_Self;
    else if (item == QStringLiteral("Coll_Mesg_Async"))
        return Coll_Mesg_Async;
    else if (item == QStringLiteral("Coll_Mesg_Sync"))
        return Coll_Mesg_Sync;
    else if (item == QStringLiteral("Seq_Message"))
        return Seq_Message;
    else if (item == QStringLiteral("Coll_Mesg_Self"))
        return Coll_Mesg_Self;
    else if (item == QStringLiteral("Seq_Message_Self"))
        return Seq_Message_Self;
    else if (item == QStringLiteral("Containment"))
        return Containment;
    else if (item == QStringLiteral("Composition"))
        return Composition;
    else if (item == QStringLiteral("Realization"))
        return Realization;
    else if (item == QStringLiteral("UniAssociation"))
        return UniAssociation;
    else if (item == QStringLiteral("Anchor"))
        return Anchor;
    else if (item == QStringLiteral("State"))
        return State;
    else if (item == QStringLiteral("Activity"))
        return Activity;
    else if (item == QStringLiteral("Exception"))
        return Exception;
    else if (item == QStringLiteral("Category2Parent"))
        return Category2Parent;
    else if (item == QStringLiteral("Child2Category"))
        return Child2Category;
    else if (item == QStringLiteral("Relationship"))
        return Relationship;
    else
        return Unknown;
}

/**
 * Convert an integer item into ProgrammingLanguage representation.
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

namespace LayoutType
{
    
/**
 * Return string corresponding to the given LayoutType.
 */
QString toString(Enum item)
{
    switch (item) {
        case Direct:
            return QStringLiteral("Direct");
        case Orthogonal:
            return QStringLiteral("Orthogonal");
        case Polyline:
            return QStringLiteral("Polyline");
        case Spline:
            return QStringLiteral("Spline");
        default:
            break;
    }
    return QString();
}

/**
 * Return LayoutType corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("Direct"))
        return Direct;
    if (item == QStringLiteral("Orthogonal"))
        return Orthogonal;
    if (item == QStringLiteral("Polyline"))
        return Polyline;
    if (item == QStringLiteral("Spline"))
        return Spline;
    return Direct;
}

/**
 * Convert an integer item into LayoutType representation.
 * @param item   integer value to convert
 * @return LayoutType enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}
}
    
//-----------------------------------------------------------------------------

namespace SignatureType {

/**
 * Return string corresponding to the given SignatureType.
 */
QString toString(Enum item)
{
    switch (item) {
        case NoSig:
            return QStringLiteral("NoSig");
        case ShowSig:
            return QStringLiteral("ShowSig");
        case SigNoVis:
            return QStringLiteral("SigNoVis");
        case NoSigNoVis:
            return QStringLiteral("NoSigNoVis");
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
    if (item == QStringLiteral("NoSig"))
        return NoSig;
    if (item == QStringLiteral("ShowSig"))
        return ShowSig;
    if (item == QStringLiteral("SigNoVis"))
        return SigNoVis;
    if (item == QStringLiteral("NoSigNoVis"))
        return NoSigNoVis;
    return NoSig;
}

/**
 * Convert an integer item into SignatureType representation.
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
            return QStringLiteral("Floating");
        case MultiA:
            return QStringLiteral("MultiA");
        case MultiB:
            return QStringLiteral("MultiB");
        case Name:
            return QStringLiteral("Name");
        case Seq_Message:
            return QStringLiteral("Seq_Message");
        case Seq_Message_Self:
            return QStringLiteral("Seq_Message_Self");
        case Coll_Message:
            return QStringLiteral("Coll_Message");
        case Coll_Message_Self:
            return QStringLiteral("Coll_Message_Self");
        case State:
            return QStringLiteral("State");
        case RoleAName:
            return QStringLiteral("RoleAName");
        case RoleBName:
            return QStringLiteral("RoleBName");
        case ChangeA:
            return QStringLiteral("ChangeA");
        case ChangeB:
            return QStringLiteral("ChangeB");
        default:
            break;
    }
    return QStringLiteral("? TextRole ?");
}

/**
 * Return TextRole corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("Floating"))
        return Floating;
    if (item == QStringLiteral("MultiA"))
        return MultiA;
    if (item == QStringLiteral("MultiB"))
        return MultiB;
    if (item == QStringLiteral("Name"))
        return Name;
    if (item == QStringLiteral("Seq_Message"))
        return Seq_Message;
    if (item == QStringLiteral("Seq_Message_Self"))
        return Seq_Message_Self;
    if (item == QStringLiteral("Coll_Message"))
        return Coll_Message;
    if (item == QStringLiteral("Coll_Message_Self"))
        return Coll_Message_Self;
    if (item == QStringLiteral("State"))
        return State;
    if (item == QStringLiteral("RoleAName"))
        return RoleAName;
    if (item == QStringLiteral("RoleBName"))
        return RoleBName;
    if (item == QStringLiteral("ChangeA"))
        return ChangeA;
    if (item == QStringLiteral("ChangeB"))
        return ChangeB;
    return Floating;
}

/**
 * Convert an integer item into TextRole representation.
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
 * @param item   The Changeability enum value to convert.
 */
QString toString(Enum item)
{
    switch (item) {
        case Changeability::Frozen:
            return QStringLiteral("frozen");
        case Changeability::AddOnly:
            return QStringLiteral("addOnly");
        case Changeability::Changeable:
            return QStringLiteral("changeable");
        default:
            break;
    }
    return QStringLiteral("? Changeability ?");
}

/**
 * Return Changeability::Enum corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("frozen"))
        return Frozen;
    if (item == QStringLiteral("addOnly"))
        return AddOnly;
    if (item == QStringLiteral("changeable"))
        return Changeable;
    return Changeable;
}

/**
 * Convert an integer item into Changeability representation.
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
            return QStringLiteral("Synchronous");
        case Asynchronous:
            return QStringLiteral("Asynchronous");
        case Creation:
            return QStringLiteral("Creation");
        case Lost:
            return QStringLiteral("Lost");
        case Found:
            return QStringLiteral("Found");
        default:
            break;
    }
    return QStringLiteral("? SequenceMessage ?");
}

/**
 * Return SequenceMessage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("Synchronous"))
        return Synchronous;
    if (item == QStringLiteral("Asynchronous"))
        return Asynchronous;
    if (item == QStringLiteral("Creation"))
        return Creation;
    if (item == QStringLiteral("Lost"))
        return Lost;
    if (item == QStringLiteral("Found"))
        return Found;
    return Synchronous;
}

/**
 * Convert an integer item into SequenceMessage representation.
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
            return QStringLiteral("A");
        case B:
            return QStringLiteral("B");
        default:
            break;
    }
    return QStringLiteral("? RoleType ?");
}

/**
 * Return RoleType corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("A"))
        return A;
    if (item == QStringLiteral("B"))
        return B;
    return A;
}

/**
 * Convert an integer item into RoleType representation.
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
            return QStringLiteral("In");
        case InOut:
            return QStringLiteral("InOut");
        case Out:
            return QStringLiteral("Out");
        default:
            break;
    }
    return QStringLiteral("? ParameterDirection ?");
}

/**
 * Return ParameterDirection corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("In"))
        return In;
    if (item == QStringLiteral("InOut"))
        return InOut;
    if (item == QStringLiteral("Out"))
        return Out;
    return In;
}

/**
 * Convert an integer item into ParameterDirection representation.
 * @param item   integer value to convert
 * @return ParameterDirection enum
 */
Enum fromInt(int item)
{
    return Enum(item);
}

}  // end namespace ParameterDirection

//-----------------------------------------------------------------------------

namespace PrimitiveTypes {

const char *strings[] = {
    "String",
    "Boolean",
    "UnlimitedNatural",
    "Integer",
    "Real",
    "not_a_primitivetype"
};

QString toString(Enum item)
{
    return QLatin1String(strings[item]);
}

QString toString(int item)
{
    if (item < 0 || item >= n_types)
        return QLatin1String(strings[Reserved]);
    return QLatin1String(strings[item]);
}

/**
 * Converts the string of an Enum to the Enum value.
 * @param item    The string to convert to Enum
 * @param strict  Controls the value returned if the given string does not
 *                represent an Enum value: If strict is true then the value
 *                Reserved is returned, otherwise the value String is
 *                returned.
 */
Enum fromString(const QString& item, bool strict /* = false */)
{
    for (int i = 0; i < n_types; i++) {
        if (item == toString(i))
            return Enum(i);
    }
    return (strict ? Reserved : String);
}

Enum fromInt(int item)
{
    if (item < 0 || item >= n_types)
        return Reserved;
    return Enum(item);
}

}  // end namespace PrimitiveTypes

//-----------------------------------------------------------------------------

namespace ProgrammingLanguage {

/**
 * Return string corresponding to the given ProgrammingLanguage.
 */
QString toString(Enum item)
{
    switch (item) {
        case ActionScript:
            return QStringLiteral("ActionScript");
        case Ada:
            return QStringLiteral("Ada");
        case Cpp:
            return QStringLiteral("C++");
        case CSharp:
            return QStringLiteral("C#");
        case D:
            return QStringLiteral("D");
        case IDL:
            return QStringLiteral("IDL");
        case Java:
            return QStringLiteral("Java");
        case JavaScript:
            return QStringLiteral("JavaScript");
        case MySQL:
            return QStringLiteral("MySQL");
        case Pascal:
            return QStringLiteral("Pascal");
        case Perl:
            return QStringLiteral("Perl");
        case PHP:
            return QStringLiteral("PHP");
        case PHP5:
            return QStringLiteral("PHP5");
        case PostgreSQL:
            return QStringLiteral("PostgreSQL");
        case Python:
            return QStringLiteral("Python");
        case Ruby:
            return QStringLiteral("Ruby");
        case SQL:
            return QStringLiteral("SQL");
        case Tcl:
            return QStringLiteral("Tcl");
        case Vala:
            return QStringLiteral("Vala");
        case XMLSchema:
            return QStringLiteral("XMLSchema");
        default:
            break;
    }
    return QStringLiteral("none");
}

/**
 * Return ProgrammingLanguage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("ActionScript"))
        return ActionScript;
    if (item == QStringLiteral("Ada"))
        return Ada;
    if (item == QStringLiteral("C++") || item == QStringLiteral("Cpp"))  // "Cpp" only for bkwd compatibility
        return Cpp;
    if (item == QStringLiteral("C#"))
        return CSharp;
    if (item == QStringLiteral("D"))
        return D;
    if (item == QStringLiteral("IDL"))
        return IDL;
    if (item == QStringLiteral("Java"))
        return Java;
    if (item == QStringLiteral("JavaScript"))
        return JavaScript;
    if (item == QStringLiteral("MySQL"))
        return MySQL;
    if (item == QStringLiteral("Pascal"))
        return Pascal;
    if (item == QStringLiteral("Perl"))
        return Perl;
    if (item == QStringLiteral("PHP"))
        return PHP;
    if (item == QStringLiteral("PHP5"))
        return PHP5;
    if (item == QStringLiteral("PostgreSQL"))
        return PostgreSQL;
    if (item == QStringLiteral("Python"))
        return Python;
    if (item == QStringLiteral("Ruby"))
        return Ruby;
    if (item == QStringLiteral("SQL"))
        return SQL;
    if (item == QStringLiteral("Tcl"))
        return Tcl;
    if (item == QStringLiteral("Vala"))
        return Vala;
    if (item == QStringLiteral("XMLSchema"))
        return XMLSchema;
    return Reserved;
}

/**
 * Convert an integer item into ProgrammingLanguage representation.
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
        case ActionScript:
            result << QStringLiteral("*.as");
            break;
        case Ada:
            result << QStringLiteral("*.ads")
                   << QStringLiteral("*.adb")
                   << QStringLiteral("*.ada");
            break;
        case Cpp:
            result << QStringLiteral("*.h")
                   << QStringLiteral("*.hpp")
                   << QStringLiteral("*.hh")
                   << QStringLiteral("*.hxx")
                   << QStringLiteral("*.H");
            break;
        case CSharp:
            result << QStringLiteral("*.cs");
            break;
        case D:
            result << QStringLiteral("*.d");
            break;
        case IDL:
            result << QStringLiteral("*.idl");
            break;
        case Java:
            result << QStringLiteral("*.java");
            break;
        case JavaScript:
            result << QStringLiteral("*.js");
            break;
        case Pascal:
            result << QStringLiteral("*.pas");
            break;
        case Perl:
            result << QStringLiteral("*.pl");
            break;
        case PHP:
            result << QStringLiteral("*.php") << QStringLiteral("*.inc");
            break;
        case PHP5:
            result << QStringLiteral("*.php") << QStringLiteral("*.php5") << QStringLiteral("*.inc");
            break;
        case Python:
            result << QStringLiteral("*.py") << QStringLiteral("*.pyw");
            break;
        case Ruby:
            result << QStringLiteral("*.rb");
            break;
        case SQL:
        case MySQL:
        case PostgreSQL:
            result << QStringLiteral("*.sql");
            if (item == MySQL)
                result << QStringLiteral("*.frm");
            break;
        case Tcl:
            result << QStringLiteral("*.tcl");
            break;
        case Vala:
            result << QStringLiteral("*.vala") << QStringLiteral("*.vapi");
            break;
        case XMLSchema:
            result << QStringLiteral("*.xsd");
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
    QString result = QString::fromLatin1("Files");
    switch (item) {  //:TODO: More languages?
        case Uml::ProgrammingLanguage::Ada:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::Cpp:
            result = QStringLiteral("Header files");
            break;
        case Uml::ProgrammingLanguage::IDL:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::Java:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::Pascal:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::Python:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::CSharp:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::PHP:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::PHP5:
            result = QStringLiteral("Source files");
            break;
        case Uml::ProgrammingLanguage::Vala:
            result = QStringLiteral("Source files");
            break;
        default:
            break;
    }
    return toString(item) + QStringLiteral(" ") + result;
}

bool isCaseSensitive(Enum item)
{
    return (item != Uml::ProgrammingLanguage::Pascal &&
            item != Uml::ProgrammingLanguage::Ada &&
            item != Uml::ProgrammingLanguage::SQL &&
            item != Uml::ProgrammingLanguage::MySQL &&
            item != Uml::ProgrammingLanguage::PostgreSQL);
}

QString scopeSeparator(Enum item)
{
    if (item == Uml::ProgrammingLanguage::Ada ||
        item == Uml::ProgrammingLanguage::CSharp ||
        item == Uml::ProgrammingLanguage::Vala ||
        item == Uml::ProgrammingLanguage::Pascal ||
        item == Uml::ProgrammingLanguage::Java ||
        item == Uml::ProgrammingLanguage::JavaScript ||
        item == Uml::ProgrammingLanguage::Vala ||
        item == Uml::ProgrammingLanguage::Python)  // CHECK: more?
        return QStringLiteral(".");
    return QStringLiteral("::");
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
            return QStringLiteral("Error");
        case West:
            return QStringLiteral("West");
        case North:
            return QStringLiteral("North");
        case East:
            return QStringLiteral("East");
        case South:
            return QStringLiteral("South");
        case NorthWest:
            return QStringLiteral("NorthWest");
        case NorthEast:
            return QStringLiteral("NorthEast");
        case SouthEast:
            return QStringLiteral("SouthEast");
        case SouthWest:
            return QStringLiteral("SouthWest");
        case Center:
            return QStringLiteral("Center");
        default:
            break;
    }
    return QStringLiteral("? Region ?");
}

/**
 * Return Region corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QStringLiteral("Error"))
        return Error;
    if (item == QStringLiteral("West"))
        return West;
    if (item == QStringLiteral("North"))
        return North;
    if (item == QStringLiteral("East"))
        return East;
    if (item == QStringLiteral("South"))
        return South;
    if (item == QStringLiteral("NorthWest"))
        return NorthWest;
    if (item == QStringLiteral("NorthEast"))
        return NorthEast;
    if (item == QStringLiteral("SouthEast"))
        return SouthEast;
    if (item == QStringLiteral("SouthWest"))
        return SouthWest;
    if (item == QStringLiteral("Center"))
        return Center;
    return Error;
}

/**
 * Convert an integer item into Region representation.
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
            return QStringLiteral("TopLeft");
        case TopRight:
            return QStringLiteral("TopRight");
        case BottomRight:
            return QStringLiteral("BottomRight");
        case BottomLeft:
            return QStringLiteral("BottomLeft");
        default:
            break;
    }
    return QStringLiteral("? Corner ?");
}

/**
 * Return Corner corresponding to the given string.
 */
Corner::Enum Corner::fromString(const QString& item)
{
    if (item == QStringLiteral("TopLeft"))
        return TopLeft;
    if (item == QStringLiteral("TopRight"))
        return TopRight;
    if (item == QStringLiteral("BottomRight"))
        return BottomRight;
    if (item == QStringLiteral("BottomLeft"))
        return BottomLeft;
    return TopLeft;
}

/**
 * Convert an integer item into Corner representation.
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


/**
* Return general system font.
* @return font
*/

QFont systemFont()
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

}  // end namespace Uml

/**
 * Convert floating point number string with '.' or ',' as decimal point to qreal.
 * @param s floating point number string
 * @return floating point number
 * @note See https://bugs.kde.org/show_bug.cgi?id=357373 for more informations.
 */
qreal toDoubleFromAnyLocale(const QString &s)
{
    bool ok;
    qreal value = s.toDouble(&ok);
    if (!ok) {
        static QLocale hungary(QLocale::Hungarian);
        value = hungary.toDouble(s, &ok);
        if (!ok) {
            qCritical() << "could not read floating point number";
            value = 0;
        }
    }
    return value;
}
