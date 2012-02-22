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

#include <QtCore/QRegExp>

namespace Uml
{

 /**
 * Convert ModelType item into QString representation.
 * @param item       item to convert
 * @return QString representation of ModelType
 */
QString ModelType::toString(Value item)
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
ModelType ModelType::fromString(const QString& item)
{
    if (item == "UseCase")
        return ModelType(UseCase);
    else if (item == "Component")
        return ModelType(Component);
    else if (item == "Deployment")
        return ModelType(Deployment);
    else if (item == "EntityRelationship")
        return ModelType(EntityRelationship);
    else
        return ModelType(Logical);
}

/**
 * Constructor.
 */
ModelType::ModelType()
  : m_value(Logical)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
ModelType::ModelType(Value item)
  : m_value(item)
{
}

/**
 * Convert Model value into QString representation.
 * @return QString representation of the model
 */
QString ModelType::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
ModelType::operator ModelType::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Convert Visibility item into QString representation.
 * @param item       item to convert
 * @param mnemonic   if true then return a single character:
 *              "+" for public, "-" for private,
 *              "#" for protected or "~" for implementation
 * @return QString representation of Visibility
 */
QString Visibility::toString(Value item, bool mnemonic)
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
 * @return Visibility object
 */
Visibility Visibility::fromString(const QString& item)
{
    if (item == "public" || item == "+")
        return Visibility(Public);
    else if (item == "protected" || item == "#")
        return Visibility(Protected);
    else if (item == "private" || item == "-")
        return Visibility(Private);
    else if (item == "~")
        return Visibility(Implementation);
    else if (item == "signals")
        return Visibility(Protected);
    else if (item == "class")
        return Visibility(Private);
    else
        return Visibility(Public);
}

/**
 * Constructor.
 */
Visibility::Visibility()
  : m_value(Public)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
Visibility::Visibility(Value item)
  : m_value(item)
{
}

/**
 * Convert Visibility value into QString representation.
 * @param mnemonic    If true then return a single character:
 *              "+" for public, "-" for private,
 *              "#" for protected or "~" for implementation
 */
QString Visibility::toString(bool mnemonic) const
{
    return toString(m_value, mnemonic);
}

/**
 * 
 */
Visibility::operator Visibility::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Convert DiagramType item into QString representation.
 * @param item       item to convert
 * @return QString representation of DiagramType
 */
QString DiagramType::toString(Value item)
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
QString DiagramType::toStringI18n() const
{
    switch (m_value) {
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
DiagramType DiagramType::fromString(const QString& item)
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
 * Constructor.
 */
DiagramType::DiagramType()
  : m_value(Undefined)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
DiagramType::DiagramType(Value item)
  : m_value(item)
{
}

/**
 * Convert DiagramType value into QString representation.
 * @return QString representation of the DiagramType
 */
QString DiagramType::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
DiagramType::operator DiagramType::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Convert AssociationType item into QString representation.
 * @param item   item to convert
 * @return QString representation of AssociationType
 */
QString AssociationType::toString(Value item)
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
        case Coll_Message:
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
QString AssociationType::toStringI18n() const
{
    switch (m_value) {
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
      case Coll_Message:
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
AssociationType AssociationType::fromString(const QString& item)
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
        return Coll_Message;
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
 * Constructor.
 */
AssociationType::AssociationType()
  : m_value(Unknown)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
AssociationType::AssociationType(Value item)
  : m_value(item)
{
}

/**
 * Convert AssociationType value into QString representation.
 * @return QString representation of the AssociationType
 */
QString AssociationType::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
AssociationType::operator AssociationType::Value() const
{
    return m_value;
}

/**
 * Returns true if the given AssociationType has a representation as a
 * UMLAssociation.
 * @param item   the AssociationType enum value to check
 * @return  boolean value
 */
bool AssociationType::hasUMLRepresentation(Value item)
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

//-----------------------------------------------------------------------------

/**
 * Return string corresponding to the given ProgrammingLanguage.
 */
QString ProgrammingLanguage::toString(Value item)
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
ProgrammingLanguage ProgrammingLanguage::fromString(const QString& item)
{
    if (item == "ActionScript")
        return ProgrammingLanguage(ActionScript);
    if (item == "Ada")
        return ProgrammingLanguage(Ada);
    if (item == "C++" || item == "Cpp")  // "Cpp" only for bkwd compatibility
        return ProgrammingLanguage(Cpp);
    if (item == "C#")
        return ProgrammingLanguage(CSharp);
    if (item == "D")
        return ProgrammingLanguage(D);
    if (item == "IDL")
        return ProgrammingLanguage(IDL);
    if (item == "Java")
        return ProgrammingLanguage(Java);
    if (item == "JavaScript")
        return ProgrammingLanguage(JavaScript);
    if (item == "MySQL")
        return ProgrammingLanguage(MySQL);
    if (item == "Pascal")
        return ProgrammingLanguage(Pascal);
    if (item == "Perl")
        return ProgrammingLanguage(Perl);
    if (item == "PHP")
        return ProgrammingLanguage(PHP);
    if (item == "PHP5")
        return ProgrammingLanguage(PHP5);
    if (item == "PostgreSQL")
        return ProgrammingLanguage(PostgreSQL);
    if (item == "Python")
        return ProgrammingLanguage(Python);
    if (item == "Ruby")
        return ProgrammingLanguage(Ruby);
    if (item == "SQL")
        return ProgrammingLanguage(SQL);
    if (item == "Tcl")
        return ProgrammingLanguage(Tcl);
    if (item == "Vala")
        return ProgrammingLanguage(Vala);
    if (item == "XMLSchema")
        return ProgrammingLanguage(XMLSchema);
    return ProgrammingLanguage(Reserved);
}

/**
 * Constructor.
 */
ProgrammingLanguage::ProgrammingLanguage()
  : m_value(Reserved)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
ProgrammingLanguage::ProgrammingLanguage(Value item)
  : m_value(item)
{
}

/**
 * Convert ProgrammingLanguage value into QString representation.
 * @return QString representation of the ProgrammingLanguage
 */
QString ProgrammingLanguage::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
ProgrammingLanguage::operator ProgrammingLanguage::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Return string corresponding to the given SignatureType.
 */
QString SignatureType::toString(Value item)
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
SignatureType SignatureType::fromString(const QString& item)
{
    if (item == "NoSig")
        return SignatureType(NoSig);
    if (item == "ShowSig")
        return SignatureType(ShowSig);
    if (item == "SigNoVis")
        return SignatureType(SigNoVis);
    if (item == "NoSigNoVis")
        return SignatureType(NoSigNoVis);
    return SignatureType(NoSig);
}

/**
 * Constructor.
 */
SignatureType::SignatureType()
  : m_value(NoSig)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
SignatureType::SignatureType(Value item)
  : m_value(item)
{
}

/**
 * Convert SignatureType value into QString representation.
 * @return QString representation of the SignatureType
 */
QString SignatureType::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
SignatureType::operator SignatureType::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Return string corresponding to the given TextRole.
 */
QString TextRole::toString(Value item)
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
TextRole TextRole::fromString(const QString& item)
{
    if (item == "Floating")
        return TextRole(Floating);
    if (item == "MultiA")
        return TextRole(MultiA);
    if (item == "MultiB")
        return TextRole(MultiB);
    if (item == "Name")
        return TextRole(Name);
    if (item == "Seq_Message")
        return TextRole(Seq_Message);
    if (item == "Seq_Message_Self")
        return TextRole(Seq_Message_Self);
    if (item == "Coll_Message")
        return TextRole(Coll_Message);
    if (item == "Coll_Message_Self")
        return TextRole(Coll_Message_Self);
    if (item == "State")
        return TextRole(State);
    if (item == "RoleAName")
        return TextRole(RoleAName);
    if (item == "RoleBName")
        return TextRole(RoleBName);
    if (item == "ChangeA")
        return TextRole(ChangeA);
    if (item == "ChangeB")
        return TextRole(ChangeB);
    return TextRole(Floating);
}

/**
 * Constructor.
 */
TextRole::TextRole()
  : m_value(Floating)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
TextRole::TextRole(Value item)
  : m_value(item)
{
}

/**
 * Convert TextRole value into QString representation.
 * @return QString representation of the TextRole
 */
QString TextRole::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
TextRole::operator TextRole::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

/**
 * Convert Changeability_Type value into QString representation.
 * @param type   The Changeability enum value to convert.
 */
QString Changeability::toString(Value item)
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
 * Return Changeability corresponding to the given string.
 */
Changeability Changeability::fromString(const QString& item)
{
    if (item == "frozen")
        return Changeability(Frozen);
    if (item == "addOnly")
        return Changeability(AddOnly);
    if (item == "changeable")
        return Changeability(Changeable);
    return Changeability(Changeable);
}

/**
 * Constructor.
 */
Changeability::Changeability()
  : m_value(Changeable)
{
}

/**
 * Constructor.
 * @param item   value to set
 */
Changeability::Changeability(Value item)
  : m_value(item)
{
}

/**
 * Convert Changeability value into QString representation.
 * @return QString representation of the Changeability
 */
QString Changeability::toString() const
{
    return toString(m_value);
}

/**
 * 
 */
Changeability::operator Changeability::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------

QDebug operator<<(QDebug out, IDType &type)
{
    out.nospace() << "IDType: " << ID2STR(type);
    return out.space();
}


}  // end namespace Uml
