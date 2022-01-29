/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javacodeaccessormethod.h"

// local includes
#include "attribute.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"
#include "codegen_utils.h"
#include "javaclassifiercodedocument.h"
#include "javacodegenerationpolicy.h"
#include "javacodeclassfield.h"
#include "javacodedocumentation.h"

// qt includes
#include <QXmlStreamWriter>

JavaCodeAccessorMethod::JavaCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type)
        : CodeAccessorMethod (field)
{
    setType(type);

    // lets use full-blown comment
    JavaClassifierCodeDocument* jccd = dynamic_cast<JavaClassifierCodeDocument*>(field->getParentDocument());
    setComment(new JavaCodeDocumentation(jccd));
}

JavaCodeAccessorMethod::~JavaCodeAccessorMethod ()
{
}

void JavaCodeAccessorMethod::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeAccessorMethod::setAttributesOnNode(writer);

    // set local attributes now
}

void JavaCodeAccessorMethod::setAttributesFromNode(QDomElement & root)
{
    // set attributes from superclass method the XMI
    CodeAccessorMethod::setAttributesFromNode(root);

    // load local stuff
}

void JavaCodeAccessorMethod::updateContent()
{
    CodeClassField * parentField = getParentClassField();
    JavaCodeClassField * javafield = dynamic_cast<JavaCodeClassField*>(parentField);
    QString fieldName = javafield->getFieldName();

    QString text;
    switch(getType()) {
    case CodeAccessorMethod::ADD:
        {
            int maxOccurs = javafield->maximumListOccurances();
            QString fieldType = javafield->getTypeName();
            QString indent = getIndentation();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            if(maxOccurs > 0)
                text += QLatin1String("if (") + fieldName + QLatin1String(".size() < ") +  QString::number(maxOccurs) + QLatin1String(") {") + endLine + indent;
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
            int minOccurs = javafield->minimumListOccurances();
            QString fieldType = javafield->getTypeName();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            QString indent = getIndentation();

            if(minOccurs > 0)
                text += QLatin1String("if (") + fieldName + QLatin1String(".size() >= ") + QString::number(minOccurs) + QLatin1String(") {") + endLine + indent;
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

void JavaCodeAccessorMethod::updateMethodDeclaration()
{
    JavaCodeClassField * javafield = dynamic_cast<JavaCodeClassField*>(getParentClassField());
    CodeGenerationPolicy *commonpolicy = UMLApp::app()->commonPolicy();

    // gather defs
    Uml::Visibility::Enum scopePolicy = commonpolicy->getAttributeAccessorScope();
    QString strVis = Uml::Visibility::toString(javafield->getVisibility());
    QString fieldName = javafield->getFieldName();
    if (fieldName.isEmpty()) {
        logError0("empty FieldName in ParentClassField");
        return;
    }
    QString fieldType = javafield->getTypeName();
    QString objectType = javafield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    // set scope of this accessor appropriately..if its an attribute,
    // we need to be more sophisticated
    if(javafield->parentIsAttribute())
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
        logWarn1("Warning: cant generate JavaCodeAccessorMethod for type: %1", getType());
        break;
    }

    // set header once.
    if(getComment()->getText().isEmpty())
        getComment()->setText(headerText);

    // set start/end method text
    setStartMethodText(strVis + QLatin1Char(' ') + methodReturnType + QLatin1Char(' ') + methodName + QLatin1String(" (") + methodParams + QLatin1String(") {"));
    setEndMethodText(QLatin1String("}"));
}

void JavaCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

