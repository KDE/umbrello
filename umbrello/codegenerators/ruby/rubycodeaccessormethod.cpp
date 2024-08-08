/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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
    if (rubyfield == nullptr)
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
                text += QStringLiteral("if ") + fieldName + QStringLiteral(".size() < ")+ QString::number(maxOccurs) + QLatin1Char(' ') + endLine + indent;
            text += fieldName + QStringLiteral(".push(value)");
            if (maxOccurs > 0)
            {
                text += endLine + QStringLiteral("else") + endLine;
                text += indent + QStringLiteral("puts(\"ERROR: Cannot add") + fieldType + QStringLiteral(" to ") + fieldName
                     + QStringLiteral(", minimum number of items reached.\")") + endLine + QStringLiteral("end") + endLine;
            }
            break;
        }
    case CodeAccessorMethod::GET:
//        text = QStringLiteral("return ") + fieldName;
        break;
    case CodeAccessorMethod::LIST:
        text = QStringLiteral("return ") + fieldName;
        break;
    case CodeAccessorMethod::REMOVE:
        {
            int minOccurs = rubyfield->minimumListOccurances();
            QString fieldType = rubyfield->getTypeName();
            QString indent = getIndentation();

            if (minOccurs > 0)
                text += QStringLiteral("if ") + fieldName + QStringLiteral(".size() >= ") + QString::number(minOccurs) + endLine + indent;
            text += fieldName + QStringLiteral(".delete(value)");
            if (minOccurs > 0)
            {
                text += endLine + QStringLiteral("else") + endLine;
                text += indent + QStringLiteral("puts(\"ERROR: Cant remove") + fieldType + QStringLiteral(" from ") + fieldName + QStringLiteral(", minimum number of items reached.\")") + endLine + QStringLiteral("end") + endLine;
            }
            break;
        }
    case CodeAccessorMethod::SET:
//        text = fieldName + QStringLiteral(" = value");
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
    if (rubyfield == nullptr)
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
    description.remove(QRegExp(QStringLiteral("m_[npb](?=[A-Z])")));
    description.remove(QStringLiteral("m_"));
    description.replace(QRegExp(QStringLiteral("[\\n\\r]+[\\t ]*")), endLine);

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
        methodName = QStringLiteral("add") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QString();
        methodParams = objectType + QStringLiteral(" value ");
        headerText = QStringLiteral("Add an object of type ") + objectType + QStringLiteral(" to the Array ") + fieldName + endLine + description + endLine + QStringLiteral("@return nil");
        setStartMethodText(QStringLiteral("def ")+ methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QStringLiteral("end"));
        break;
    case CodeAccessorMethod::GET:
        headerText = QStringLiteral("Get the value of ") + fieldName + endLine + description;
        setStartMethodText(QString(QStringLiteral("attr_reader :")) + fieldName);
        setEndMethodText(QString());
        break;
    case CodeAccessorMethod::LIST:
        methodName = QStringLiteral("get") + Codegen_Utils::capitalizeFirstLetter(fieldType) + QStringLiteral("List");
        methodReturnType = QString();
        headerText = QStringLiteral("Get the list of ") + fieldName + endLine + description + endLine + QStringLiteral("_returns_ List of ") + fieldName;
        setStartMethodText(QStringLiteral("def ")+ methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QStringLiteral("end"));
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QStringLiteral("remove") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QString();
        methodParams = objectType + QStringLiteral(" value ");
        headerText = QStringLiteral("Remove an object of type ") + objectType + QStringLiteral(" from the List ") + fieldName + endLine + description;
        setStartMethodText(QStringLiteral("def ") + methodName + QLatin1Char('(') + methodParams + QLatin1Char(')'));
        setEndMethodText(QStringLiteral("end"));
        break;
    case CodeAccessorMethod::SET:
        headerText = QStringLiteral("Set the value of ") + fieldName + endLine + description;
        setStartMethodText(QString(QStringLiteral("attr_writer :")) + fieldName);
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

