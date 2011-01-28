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
            return "UseCase";
        case Component:
            return "Component";
        case Deployment:
            return "Deployment";
        case EntityRelationship:
            return "EntityRelationship";
        case Logical:
        default:
            return "Logical";
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
            return (mnemonic ? "#" : "protected");
        case Private:
            return (mnemonic ? "-" : "private");
        case Implementation:
            return (mnemonic ? "~" : "implementation");
        case Public:
        default:
            return (mnemonic ? "+" : "public");
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
            return "Undefined";
        case Class:
            return "Class";
        case UseCase:
            return "UseCase";
        case Sequence:
            return "Sequence";
        case Collaboration:
            return "Collaboration";
        case State:
            return "State";
        case Activity:
            return "Activity";
        case Component:
            return "Component";
        case Deployment:
            return "Deployment";
        case EntityRelationship:
            return "EntityRelationship";
        case N_DIAGRAMTYPES:   // must remain last
            return QString("N_DIAGRAMTYPES");
        default:
            return QString("? DiagramType ?");
    }
}

/**
 * Return string corresponding to Uml::DiagramType
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
 * Convert a string item into Model representation.
 * @param item   item to convert
 * @return Model object
 */
DiagramType DiagramType::fromString(const QString& item)
{
    if (item == "Undefined")
        return DiagramType(Undefined);
    else if (item == "Class")
        return DiagramType(Class);
    else if (item == "UseCase")
        return DiagramType(UseCase);
    else if (item == "Sequence")
        return DiagramType(Sequence);
    else if (item == "Collaboration")
        return DiagramType(Collaboration);
    else if (item == "State")
        return DiagramType(State);
    else if (item == "Activity")
        return DiagramType(Activity);
    else if (item == "Component")
        return DiagramType(Component);
    else if (item == "Deployment")
        return DiagramType(Deployment);
    else if (item == "EntityRelationship")
        return DiagramType(EntityRelationship);
    else
        return DiagramType(Undefined);
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
 * Return string corresponding to the given ProgrammingLanguage.
 */
QString ProgrammingLanguage::toString(Value item)
{
    switch (item) {
        case ActionScript:
            return "ActionScript";
        case Ada:
            return "Ada";
        case Cpp:
            return "C++";
        case CSharp:
            return "C#";
        case D:
            return "D";
        case IDL:
            return "IDL";
        case Java:
            return "Java";
        case JavaScript:
            return "JavaScript";
        case MySQL:
             return "MySQL";
        case Pascal:
            return "Pascal";
        case Perl:
            return "Perl";
        case PHP:
            return "PHP";
        case PHP5:
            return "PHP5";
        case PostgreSQL:
            return "PostgreSQL";
        case Python:
            return "Python";
        case Ruby:
            return "Ruby";
        case SQL:
            return "SQL";
        case Tcl:
            return "Tcl";
        case Vala:
            return "Vala";
        case XMLSchema:
            return "XMLSchema";
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


}  // end namespace Uml
