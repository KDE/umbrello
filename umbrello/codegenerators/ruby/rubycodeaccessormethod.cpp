/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubycodeaccessormethod.h"

// local includes
#include "attribute.h"
#include "codegenerator.h"
#include "classifiercodedocument.h"
#include "codegen_utils.h"
#include "debug_utils.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "rubycodegenerator.h"
#include "rubycodeclassfield.h"
#include "rubycodedocumentation.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

// qt includes
#include <QRegExp>
#include <QXmlStreamWriter>

/**
 * Constructor.
 */
RubyCodeAccessorMethod::RubyCodeAccessorMethod(CodeClassField * field, CodeAccessorMethod::AccessorType type)
  : CodeAccessorMethod(field)
{
    setType(type);

    // lets use full-blown comment
    RubyClassifierCodeDocument *rccd = dynamic_cast<RubyClassifierCodeDocument*>(field->getParentDocument());
    setComment(new RubyCodeDocumentation(rccd));
}

/**
 * Empty Destructor.
 */
RubyCodeAccessorMethod::~RubyCodeAccessorMethod()
{
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void RubyCodeAccessorMethod::setAttributesOnNode(QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeAccessorMethod::setAttributesOnNode(writer);

    // set local attributes now
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void RubyCodeAccessorMethod::setAttributesFromNode(QDomElement& root)
{
    // set attributes from superclass method the XMI
    CodeAccessorMethod::setAttributesFromNode(root);

    // load local stuff
}

void RubyCodeAccessorMethod::updateContent()
{
    CodeClassField * parentField = getParentClassField();
    RubyCodeClassField * rubyfield = dynamic_cast<RubyCodeClassField*>(parentField);

    // Check for dynamic casting failure!
    if (rubyfield == 0)
    {
        logError0("rubyfield: invalid dynamic cast");
        return;
    }

    QString fieldName = rubyfield->getFieldName();
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    QString text;
    switch(getType()) {
    case CodeAccessorMethod::ADD:
        {
            int maxOccurs = rubyfield->maximumListOccurances();
            QString fieldType = rubyfield->getTypeName();
            QString indent = getIndentation();
            if (maxOccurs > 0)
                text += QLatin1String("if ") + fieldName + QLatin1String(".size() < ")+ QString::number(maxOccurs) + QLatin1Char(' ') + endLine + indent;
            text += fieldName + QLatin1String(".push(value)");
            if (maxOccurs > 0)
            {
                text += endLine + QLatin1String("else") + endLine;
                text += indent + QLatin1String("puts(\"ERROR: Cannot add") + fieldType + QLatin1String(" to ") + fieldName
                     + QLatin1String(", minimum number of items reached.\")") + endLine + QLatin1String("end") + endLine;
            }
            break;
        }
    case CodeAccessorMethod::GET:
//        text = QLatin1String("return ") + fieldName;
        break;
    case CodeAccessorMethod::LIST:
        text = QLatin1String("return ") + fieldName;
        break;
    case CodeAccessorMethod::REMOVE:
        {
            int minOccurs = rubyfield->minimumListOccurances();
            QString fieldType = rubyfield->getTypeName();
            QString indent = getIndentation();

            if (minOccurs > 0)
                text += QLatin1String("if ") + fieldName + QLatin1String(".size() >= ") + QString::number(minOccurs) + endLine + indent;
            text += fieldName + QLatin1String(".delete(value)");
            if (minOccurs > 0)
            {
                text += endLine + QLatin1String("else") + endLine;
                text += indent + QLatin1String("puts(\"ERROR: Cant remove") + fieldType + QLatin1String(" from ") + fieldName + QLatin1String(", minimum number of items reached.\")") + endLine + QLatin1String("end") + endLine;
            }
            break;
        }
    case CodeAccessorMethod::SET:
//        text = fieldName + QLatin1String(" = value");
        break;
    default:
        // do nothing
        break;
    }

    setText(text);
}

void RubyCodeAccessorMethod::updateMethodDeclaration()
{
    RubyCodeClassField * rubyfield = dynamic_cast<RubyCodeClassField*>(getParentClassField());

    // Check for dynamic casting failure!
    if (rubyfield == 0)
    {
        logError0("rubyfield: invalid dynamic cast");
        return;
    }

    // gather defs
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    Uml::Visibility::Enum scopePolicy = p->getAttributeAccessorScope();
    QString strVis = Uml::Visibility::toString(rubyfield->getVisibility());
    QString fieldName = RubyCodeGenerator::cppToRubyName(rubyfield->getFieldName());
    QString fieldType = RubyCodeGenerator::cppToRubyType(rubyfield->getTypeName());
    QString objectType = rubyfield->getListObjectType();
    if (objectType.isEmpty())
        objectType = fieldName;
    QString endLine = p->getNewLineEndingChars();

    QString description = getParentObject()->doc();
    description.remove(QRegExp(QLatin1String("m_[npb](?=[A-Z])")));
    description.remove(QLatin1String("m_"));
    description.replace(QRegExp(QLatin1String("[\\n\\r]+[\\t ]*")), endLine);

    // set scope of this accessor appropriately..if its an attribute,
    // we need to be more sophisticated
    if (rubyfield->parentIsAttribute())
        switch (scopePolicy) {
        case Uml::Visibility::Public:
        case Uml::Visibility::Private:
        case Uml::Visibility::Protected:
            strVis = Uml::Visibility::toString(scopePolicy);
            break;
        default:
        case Uml::Visibility::FromParent:
            // do nothing..already have taken parent value
            break;
        }

    // some variables we will need to populate
    QString headerText;
    QString methodReturnType;
    QString methodName;
    QString methodParams;

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = QLatin1String("add") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QString();
        methodParams = objectType + QLatin1String(" value ");
        headerText = QLatin1String("Add an object of type ") + objectType + QLatin1String(" to the Array ") + fieldName + endLine + description + endLine + QLatin1String("@return nil");
        setStartMethodText(QLatin1String("def ")+ methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QLatin1String("end"));
        break;
    case CodeAccessorMethod::GET:
        headerText = QLatin1String("Get the value of ") + fieldName + endLine + description;
        setStartMethodText(QString(QLatin1String("attr_reader :")) + fieldName);
        setEndMethodText(QString());
        break;
    case CodeAccessorMethod::LIST:
        methodName = QLatin1String("get") + Codegen_Utils::capitalizeFirstLetter(fieldType) + QLatin1String("List");
        methodReturnType = QString();
        headerText = QLatin1String("Get the list of ") + fieldName + endLine + description + endLine + QLatin1String("_returns_ List of ") + fieldName;
        setStartMethodText(QLatin1String("def ")+ methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QLatin1String("end"));
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QLatin1String("remove") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QString();
        methodParams = objectType + QLatin1String(" value ");
        headerText = QLatin1String("Remove an object of type ") + objectType + QLatin1String(" from the List ") + fieldName + endLine + description;
        setStartMethodText(QLatin1String("def ") + methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QLatin1String("end"));
        break;
    case CodeAccessorMethod::SET:
        headerText = QLatin1String("Set the value of ") + fieldName + endLine + description;
        setStartMethodText(QString(QLatin1String("attr_writer :")) + fieldName);
        setEndMethodText(QString());
        break;
    default:
        // do nothing..no idea what this is
        logWarn1("Warning: cannot generate RubyCodeAccessorMethod for type: %1", getType());
        break;
    }

    // set header once.
    if (getComment()->getText().isEmpty())
        getComment()->setText(headerText);
}

/**
 * Must be called before this object is usable.
 */
void RubyCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

