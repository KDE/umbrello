/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "basictypes.h"

#include "debug_utils.h"

// kde includes
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <KGlobalSettings>
#endif

// qt includes
#if QT_VERSION >= 0x050000
#include <QFontDatabase>
#endif
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
            return QString(QLatin1String("UseCase"));
        case Component:
            return QString(QLatin1String("Component"));
        case Deployment:
            return QString(QLatin1String("Deployment"));
        case EntityRelationship:
            return QString(QLatin1String("EntityRelationship"));
        case Logical:
        default:
            return QString(QLatin1String("Logical"));
    }
}

/**
 * Convert a string item into Model representation.
 * @param item   item to convert
 * @return Model object
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("UseCase"))
        return UseCase;
    else if (item == QLatin1String("Component"))
        return Component;
    else if (item == QLatin1String("Deployment"))
        return Deployment;
    else if (item == QLatin1String("EntityRelationship"))
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
            return (mnemonic ? QLatin1String("#") : QLatin1String("protected"));
        case Private:
            return (mnemonic ? QLatin1String("-") : QLatin1String("private"));
        case Implementation:
            return (mnemonic ? QLatin1String("~") : QLatin1String("implementation"));
        case Public:
        default:
            return (mnemonic ? QLatin1String("+") : QLatin1String("public"));
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
    if (item == QLatin1String("public") || item == QLatin1String("+"))
        return Public;
    else if (item == QLatin1String("protected") || item == QLatin1String("#"))
        return Protected;
    else if (item == QLatin1String("private") || item == QLatin1String("-"))
        return Private;
    else if (item == QLatin1String("~"))
        return Implementation;
    else if (item == QLatin1String("signals"))
        return Protected;
    else if (item == QLatin1String("class"))
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
            return QLatin1String("Undefined");
        case Class:
            return QLatin1String("Class");
        case Object:
            return QLatin1String("Object");
        case UseCase:
            return QLatin1String("UseCase");
        case Sequence:
            return QLatin1String("Sequence");
        case Collaboration:
            return QLatin1String("Collaboration");
        case State:
            return QLatin1String("State");
        case Activity:
            return QLatin1String("Activity");
        case Component:
            return QLatin1String("Component");
        case Deployment:
            return QLatin1String("Deployment");
        case EntityRelationship:
            return QLatin1String("EntityRelationship");
        case N_DIAGRAMTYPES:   // must remain last
            return QLatin1String("N_DIAGRAMTYPES");
        default:
            return QLatin1String("? DiagramType ?");
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
    if (item == QLatin1String("Undefined"))
        return Undefined;
    else if (item == QLatin1String("Class"))
        return Class;
    else if (item == QLatin1String("UseCase"))
        return UseCase;
    else if (item == QLatin1String("Sequence"))
        return Sequence;
    else if (item == QLatin1String("Collaboration"))
        return Collaboration;
    else if (item == QLatin1String("State"))
        return State;
    else if (item == QLatin1String("Activity"))
        return Activity;
    else if (item == QLatin1String("Component"))
        return Component;
    else if (item == QLatin1String("Deployment"))
        return Deployment;
    else if (item == QLatin1String("EntityRelationship"))
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
            return QLatin1String("Generalization");
        case Aggregation:
            return QLatin1String("Aggregation");
        case Dependency:
            return QLatin1String("Dependency");
        case Association:
            return QLatin1String("Association");
        case Association_Self:
            return QLatin1String("Association_Self");
        case Coll_Mesg_Async:
            return QLatin1String("Coll_Mesg_Async");
        case Coll_Mesg_Sync:
            return QLatin1String("Coll_Mesg_Sync");
        case Seq_Message:
            return QLatin1String("Seq_Message");
        case Coll_Mesg_Self:
            return QLatin1String("Coll_Mesg_Self");
        case Seq_Message_Self:
            return QLatin1String("Seq_Message_Self");
        case Containment:
            return QLatin1String("Containment");
        case Composition:
          return QLatin1String("Composition");
        case Realization:
          return QLatin1String("Realization");
        case UniAssociation:
          return QLatin1String("UniAssociation");
        case Anchor:
          return QLatin1String("Anchor");
        case State:
          return QLatin1String("State");
        case Activity:
          return QLatin1String("Activity");
        case Exception:
          return QLatin1String("Exception");
        case Category2Parent:
          return QLatin1String("Category2Parent");
        case Child2Category:
          return QLatin1String("Child2Category");
        case Relationship:
          return QLatin1String("Relationship");
        case Unknown:
            return QLatin1String("Unknown");
        default:
            return QLatin1String("? AssociationType ?");
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
    if (item == QLatin1String("Generalization"))
        return Generalization;
    else if (item == QLatin1String("Aggregation"))
        return Aggregation;
    else if (item == QLatin1String("Dependency"))
        return Dependency;
    else if (item == QLatin1String("Association"))
        return Association;
    else if (item == QLatin1String("Association_Self"))
        return Association_Self;
    else if (item == QLatin1String("Coll_Mesg_Async"))
        return Coll_Mesg_Async;
    else if (item == QLatin1String("Coll_Mesg_Sync"))
        return Coll_Mesg_Sync;
    else if (item == QLatin1String("Seq_Message"))
        return Seq_Message;
    else if (item == QLatin1String("Coll_Mesg_Self"))
        return Coll_Mesg_Self;
    else if (item == QLatin1String("Seq_Message_Self"))
        return Seq_Message_Self;
    else if (item == QLatin1String("Containment"))
        return Containment;
    else if (item == QLatin1String("Composition"))
        return Composition;
    else if (item == QLatin1String("Realization"))
        return Realization;
    else if (item == QLatin1String("UniAssociation"))
        return UniAssociation;
    else if (item == QLatin1String("Anchor"))
        return Anchor;
    else if (item == QLatin1String("State"))
        return State;
    else if (item == QLatin1String("Activity"))
        return Activity;
    else if (item == QLatin1String("Exception"))
        return Exception;
    else if (item == QLatin1String("Category2Parent"))
        return Category2Parent;
    else if (item == QLatin1String("Child2Category"))
        return Child2Category;
    else if (item == QLatin1String("Relationship"))
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
            return QLatin1String("Direct");
        case Orthogonal:
            return QLatin1String("Orthogonal");
        case Polyline:
            return QLatin1String("Polyline");
        case Spline:
            return QLatin1String("Spline");
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
    if (item == QLatin1String("Direct"))
        return Direct;
    if (item == QLatin1String("Orthogonal"))
        return Orthogonal;
    if (item == QLatin1String("Polyline"))
        return Polyline;
    if (item == QLatin1String("Spline"))
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
            return QLatin1String("NoSig");
        case ShowSig:
            return QLatin1String("ShowSig");
        case SigNoVis:
            return QLatin1String("SigNoVis");
        case NoSigNoVis:
            return QLatin1String("NoSigNoVis");
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
    if (item == QLatin1String("NoSig"))
        return NoSig;
    if (item == QLatin1String("ShowSig"))
        return ShowSig;
    if (item == QLatin1String("SigNoVis"))
        return SigNoVis;
    if (item == QLatin1String("NoSigNoVis"))
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
            return QLatin1String("Floating");
        case MultiA:
            return QLatin1String("MultiA");
        case MultiB:
            return QLatin1String("MultiB");
        case Name:
            return QLatin1String("Name");
        case Seq_Message:
            return QLatin1String("Seq_Message");
        case Seq_Message_Self:
            return QLatin1String("Seq_Message_Self");
        case Coll_Message:
            return QLatin1String("Coll_Message");
        case Coll_Message_Self:
            return QLatin1String("Coll_Message_Self");
        case State:
            return QLatin1String("State");
        case RoleAName:
            return QLatin1String("RoleAName");
        case RoleBName:
            return QLatin1String("RoleBName");
        case ChangeA:
            return QLatin1String("ChangeA");
        case ChangeB:
            return QLatin1String("ChangeB");
        default:
            break;
    }
    return QLatin1String("? TextRole ?");
}

/**
 * Return TextRole corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("Floating"))
        return Floating;
    if (item == QLatin1String("MultiA"))
        return MultiA;
    if (item == QLatin1String("MultiB"))
        return MultiB;
    if (item == QLatin1String("Name"))
        return Name;
    if (item == QLatin1String("Seq_Message"))
        return Seq_Message;
    if (item == QLatin1String("Seq_Message_Self"))
        return Seq_Message_Self;
    if (item == QLatin1String("Coll_Message"))
        return Coll_Message;
    if (item == QLatin1String("Coll_Message_Self"))
        return Coll_Message_Self;
    if (item == QLatin1String("State"))
        return State;
    if (item == QLatin1String("RoleAName"))
        return RoleAName;
    if (item == QLatin1String("RoleBName"))
        return RoleBName;
    if (item == QLatin1String("ChangeA"))
        return ChangeA;
    if (item == QLatin1String("ChangeB"))
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
            return QLatin1String("frozen");
        case Changeability::AddOnly:
            return QLatin1String("addOnly");
        case Changeability::Changeable:
            return QLatin1String("changeable");
        default:
            break;
    }
    return QLatin1String("? Changeability ?");
}

/**
 * Return Changeability::Enum corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("frozen"))
        return Frozen;
    if (item == QLatin1String("addOnly"))
        return AddOnly;
    if (item == QLatin1String("changeable"))
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
            return QLatin1String("Synchronous");
        case Asynchronous:
            return QLatin1String("Asynchronous");
        case Creation:
            return QLatin1String("Creation");
        case Lost:
            return QLatin1String("Lost");
        case Found:
            return QLatin1String("Found");
        default:
            break;
    }
    return QLatin1String("? SequenceMessage ?");
}

/**
 * Return SequenceMessage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("Synchronous"))
        return Synchronous;
    if (item == QLatin1String("Asynchronous"))
        return Asynchronous;
    if (item == QLatin1String("Creation"))
        return Creation;
    if (item == QLatin1String("Lost"))
        return Lost;
    if (item == QLatin1String("Found"))
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
            return QLatin1String("A");
        case B:
            return QLatin1String("B");
        default:
            break;
    }
    return QLatin1String("? RoleType ?");
}

/**
 * Return RoleType corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("A"))
        return A;
    if (item == QLatin1String("B"))
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
            return QLatin1String("In");
        case InOut:
            return QLatin1String("InOut");
        case Out:
            return QLatin1String("Out");
        default:
            break;
    }
    return QLatin1String("? ParameterDirection ?");
}

/**
 * Return ParameterDirection corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("In"))
        return In;
    if (item == QLatin1String("InOut"))
        return InOut;
    if (item == QLatin1String("Out"))
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
            return QLatin1String("ActionScript");
        case Ada:
            return QLatin1String("Ada");
        case Cpp:
            return QLatin1String("C++");
        case CSharp:
            return QLatin1String("C#");
        case D:
            return QLatin1String("D");
        case IDL:
            return QLatin1String("IDL");
        case Java:
            return QLatin1String("Java");
        case JavaScript:
            return QLatin1String("JavaScript");
        case MySQL:
            return QLatin1String("MySQL");
        case Pascal:
            return QLatin1String("Pascal");
        case Perl:
            return QLatin1String("Perl");
        case PHP:
            return QLatin1String("PHP");
        case PHP5:
            return QLatin1String("PHP5");
        case PostgreSQL:
            return QLatin1String("PostgreSQL");
        case Python:
            return QLatin1String("Python");
        case Ruby:
            return QLatin1String("Ruby");
        case SQL:
            return QLatin1String("SQL");
        case Tcl:
            return QLatin1String("Tcl");
        case Vala:
            return QLatin1String("Vala");
        case XMLSchema:
            return QLatin1String("XMLSchema");
        default:
            break;
    }
    return QLatin1String("none");
}

/**
 * Return ProgrammingLanguage corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("ActionScript"))
        return ActionScript;
    if (item == QLatin1String("Ada"))
        return Ada;
    if (item == QLatin1String("C++") || item == QLatin1String("Cpp"))  // "Cpp" only for bkwd compatibility
        return Cpp;
    if (item == QLatin1String("C#"))
        return CSharp;
    if (item == QLatin1String("D"))
        return D;
    if (item == QLatin1String("IDL"))
        return IDL;
    if (item == QLatin1String("Java"))
        return Java;
    if (item == QLatin1String("JavaScript"))
        return JavaScript;
    if (item == QLatin1String("MySQL"))
        return MySQL;
    if (item == QLatin1String("Pascal"))
        return Pascal;
    if (item == QLatin1String("Perl"))
        return Perl;
    if (item == QLatin1String("PHP"))
        return PHP;
    if (item == QLatin1String("PHP5"))
        return PHP5;
    if (item == QLatin1String("PostgreSQL"))
        return PostgreSQL;
    if (item == QLatin1String("Python"))
        return Python;
    if (item == QLatin1String("Ruby"))
        return Ruby;
    if (item == QLatin1String("SQL"))
        return SQL;
    if (item == QLatin1String("Tcl"))
        return Tcl;
    if (item == QLatin1String("Vala"))
        return Vala;
    if (item == QLatin1String("XMLSchema"))
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
            result << QLatin1String("*.as");
            break;
        case Ada:
            result << QLatin1String("*.ads")
                   << QLatin1String("*.adb")
                   << QLatin1String("*.ada");
            break;
        case Cpp:
            result << QLatin1String("*.h")
                   << QLatin1String("*.hpp")
                   << QLatin1String("*.hh")
                   << QLatin1String("*.hxx")
                   << QLatin1String("*.H");
            break;
        case CSharp:
            result << QLatin1String("*.cs");
            break;
        case D:
            result << QLatin1String("*.d");
            break;
        case IDL:
            result << QLatin1String("*.idl");
            break;
        case Java:
            result << QLatin1String("*.java");
            break;
        case JavaScript:
            result << QLatin1String("*.js");
            break;
        case Pascal:
            result << QLatin1String("*.pas");
            break;
        case Perl:
            result << QLatin1String("*.pl");
            break;
        case PHP:
            result << QLatin1String("*.php") << QLatin1String("*.inc");
            break;
        case PHP5:
            result << QLatin1String("*.php") << QLatin1String("*.php5") << QLatin1String("*.inc");
            break;
        case Python:
            result << QLatin1String("*.py") << QLatin1String("*.pyw");
            break;
        case Ruby:
            result << QLatin1String("*.rb");
            break;
        case SQL:
        case MySQL:
        case PostgreSQL:
            result << QLatin1String("*.sql");
            if (item == MySQL)
                result << QLatin1String("*.frm");
            break;
        case Tcl:
            result << QLatin1String("*.tcl");
            break;
        case Vala:
            result << QLatin1String("*.vala");
            break;
        case XMLSchema:
            result << QLatin1String("*.xsd");
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
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::Cpp:
            result = QLatin1String("Header files");
            break;
        case Uml::ProgrammingLanguage::IDL:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::Java:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::Pascal:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::Python:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::CSharp:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::PHP:
            result = QLatin1String("Source files");
            break;
        case Uml::ProgrammingLanguage::PHP5:
            result = QLatin1String("Source files");
            break;
        default:
            break;
    }
    return toString(item) + QLatin1String(" ") + result;
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
        item == Uml::ProgrammingLanguage::Pascal ||
        item == Uml::ProgrammingLanguage::Java ||
        item == Uml::ProgrammingLanguage::JavaScript ||
        item == Uml::ProgrammingLanguage::Vala ||
        item == Uml::ProgrammingLanguage::Python)  // CHECK: more?
        return QLatin1String(".");
    return QLatin1String("::");
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
            return QLatin1String("Error");
        case West:
            return QLatin1String("West");
        case North:
            return QLatin1String("North");
        case East:
            return QLatin1String("East");
        case South:
            return QLatin1String("South");
        case NorthWest:
            return QLatin1String("NorthWest");
        case NorthEast:
            return QLatin1String("NorthEast");
        case SouthEast:
            return QLatin1String("SouthEast");
        case SouthWest:
            return QLatin1String("SouthWest");
        case Center:
            return QLatin1String("Center");
        default:
            break;
    }
    return QLatin1String("? Region ?");
}

/**
 * Return Region corresponding to the given string.
 */
Enum fromString(const QString& item)
{
    if (item == QLatin1String("Error"))
        return Error;
    if (item == QLatin1String("West"))
        return West;
    if (item == QLatin1String("North"))
        return North;
    if (item == QLatin1String("East"))
        return East;
    if (item == QLatin1String("South"))
        return South;
    if (item == QLatin1String("NorthWest"))
        return NorthWest;
    if (item == QLatin1String("NorthEast"))
        return NorthEast;
    if (item == QLatin1String("SouthEast"))
        return SouthEast;
    if (item == QLatin1String("SouthWest"))
        return SouthWest;
    if (item == QLatin1String("Center"))
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
            return QLatin1String("TopLeft");
        case TopRight:
            return QLatin1String("TopRight");
        case BottomRight:
            return QLatin1String("BottomRight");
        case BottomLeft:
            return QLatin1String("BottomLeft");
        default:
            break;
    }
    return QLatin1String("? Corner ?");
}

/**
 * Return Corner corresponding to the given string.
 */
Corner::Enum Corner::fromString(const QString& item)
{
    if (item == QLatin1String("TopLeft"))
        return TopLeft;
    if (item == QLatin1String("TopRight"))
        return TopRight;
    if (item == QLatin1String("BottomRight"))
        return BottomRight;
    if (item == QLatin1String("BottomLeft"))
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
#if QT_VERSION >= 0x050000
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#else
    return KGlobalSettings::generalFont();
#endif
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
