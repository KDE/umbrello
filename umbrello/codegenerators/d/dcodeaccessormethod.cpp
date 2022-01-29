/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodeaccessormethod.h"

// local includes
#include "attribute.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "codegen_utils.h"
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "dcodeclassfield.h"
#include "dcodedocumentation.h"

// qt/kde includes
#include <QXmlStreamWriter>

DCodeAccessorMethod::DCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type)
        : CodeAccessorMethod (field)
{
    setType(type);

    // lets use full-blown comment
    DClassifierCodeDocument* jccd = dynamic_cast<DClassifierCodeDocument*>(field->getParentDocument());
    setComment(new DCodeDocumentation(jccd));
}

DCodeAccessorMethod::~DCodeAccessorMethod ()
{
}

void DCodeAccessorMethod::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeAccessorMethod::setAttributesOnNode(writer);

    // set local attributes now
}

void DCodeAccessorMethod::setAttributesFromNode(QDomElement & root)
{
    // set attributes from superclass method the XMI
    CodeAccessorMethod::setAttributesFromNode(root);

    // load local stuff
}

void DCodeAccessorMethod::updateContent()
{
    CodeClassField * parentField = getParentClassField();
    DCodeClassField * dfield = dynamic_cast<DCodeClassField*>(parentField);
    QString fieldName = dfield->getFieldName();

    QString text;
    switch(getType()) {
    case CodeAccessorMethod::ADD:
        {
            int maxOccurs = dfield->maximumListOccurances();
            QString fieldType = dfield->getTypeName();
            QString indent = getIndentation();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            if(maxOccurs > 0)
                text += QLatin1String("if (") + fieldName + QLatin1String(".size() < ")+ QString::number(maxOccurs) + QLatin1String(") {") + endLine + indent;
            text += fieldName + QLatin1String(".add(value);");
            if(maxOccurs > 0)
            {
                text += endLine + QLatin1String("} else {") + endLine;
                text += indent + QLatin1String("System.err.println(\"ERROR: Cant add") + fieldType + QLatin1String(" to ") + fieldName + QLatin1String(", minimum number of items reached.\");") + endLine + QLatin1Char('}') + endLine;
            }
            break;
        }
    case CodeAccessorMethod::GET:
        text = QLatin1String("return ") + fieldName + QLatin1Char(';');
        break;
    case CodeAccessorMethod::LIST:
        text = QLatin1String("return (List) ") + fieldName + QLatin1Char(';');
        break;
    case CodeAccessorMethod::REMOVE:
        {
            int minOccurs = dfield->minimumListOccurances();
            QString fieldType = dfield->getTypeName();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            QString indent = getIndentation();

            if(minOccurs > 0)
                text += QLatin1String("if (") + fieldName + QLatin1String(".size() >= ")+ QString::number(minOccurs) + QLatin1String(") {") + endLine + indent;
            text += fieldName + QLatin1String(".remove(value);");
            if(minOccurs > 0)
            {
                text += endLine + QLatin1String("} else {") + endLine;
                text += indent + QLatin1String("System.err.println(\"ERROR: Cant remove") + fieldType + QLatin1String(" from ") + fieldName + QLatin1String(", minimum number of items reached.\");") + endLine + QLatin1Char('}') + endLine;
            }
            break;
        }
    case CodeAccessorMethod::SET:
        text = fieldName + QLatin1String(" = value;");
        break;
    default:
        // do nothing
        break;
    }

    setText(text);
}

void DCodeAccessorMethod::updateMethodDeclaration()
{
    DCodeClassField * dfield = dynamic_cast<DCodeClassField*>(getParentClassField());

    // Check for dynamic casting failure!
    if (dfield == 0)
    {
        logError0("dfield: invalid dynamic cast");
        return;
    }

    CodeGenerationPolicy *commonpolicy = UMLApp::app()->commonPolicy();

    // gather defs
    Uml::Visibility::Enum scopePolicy = commonpolicy->getAttributeAccessorScope();
    QString strVis = Uml::Visibility::toString(dfield->getVisibility());
    QString fieldName = dfield->getFieldName();
    QString fieldType = dfield->getTypeName();
    QString objectType = dfield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    // set scope of this accessor appropriately..if its an attribute,
    // we need to be more sophisticated
    if (dfield->parentIsAttribute()) {
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
    }

    // some variables we will need to populate
    QString headerText;
    QString methodReturnType;
    QString methodName;
    QString methodParams;

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = QLatin1String("add") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QLatin1String("void");
        methodParams = objectType + QLatin1String(" value ");
        headerText = QLatin1String("Add an object of type ") + objectType + QLatin1String(" to the List ") + fieldName + endLine + getParentObject()->doc() + endLine + QLatin1String("@return void");
        break;
    case CodeAccessorMethod::GET:
        methodName = QLatin1String("get") + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = fieldType;
        headerText = QLatin1String("Get the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + QLatin1String("@return the value of ") + fieldName;
        break;
    case CodeAccessorMethod::LIST:
        methodName = QLatin1String("get") + Codegen_Utils::capitalizeFirstLetter(fieldType) + QLatin1String("List");
        methodReturnType = QLatin1String("List");
        headerText = QLatin1String("Get the list of ") + fieldName + endLine + getParentObject()->doc() + endLine + QLatin1String("@return List of ") + fieldName;
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QLatin1String("remove") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QLatin1String("void");
        methodParams = objectType + QLatin1String(" value ");
        headerText = QLatin1String("Remove an object of type ") + objectType + QLatin1String(" from the List ") + fieldName + endLine + getParentObject()->doc();
        break;
    case CodeAccessorMethod::SET:
        methodName = QLatin1String("set") + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = QLatin1String("void");
        methodParams = fieldType + QLatin1String(" value ");
        headerText = QLatin1String("Set the value of ") + fieldName + endLine + getParentObject()->doc() + endLine;
        break;
    default:
        // do nothing..no idea what this is
        logWarn1("Cannot generate DCodeAccessorMethod for type: %1", getType());
        break;
    }

    // set header once.
    if(getComment()->getText().isEmpty())
        getComment()->setText(headerText);

    // set start/end method text
    setStartMethodText(strVis + QLatin1Char(' ') + methodReturnType + QLatin1Char(' ') + methodName + QLatin1String(" (") + methodParams + QLatin1String(") {"));
    setEndMethodText(QLatin1String("}"));
}

void DCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

