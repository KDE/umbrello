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
                text += QStringLiteral("if (") + fieldName + QStringLiteral(".size() < ") +  QString::number(maxOccurs) + QStringLiteral(") {") + endLine + indent;
            text += fieldName + QStringLiteral(".add(value);");
            if(maxOccurs > 0)
            {
                text += endLine + QStringLiteral("} else {") + endLine;
                text += indent + QStringLiteral("System.err.println(\"ERROR: Cant add") + fieldType + QStringLiteral(" to ") + fieldName + QStringLiteral(", minimum number of items reached.\");") + endLine + QLatin1Char('}') + endLine;
            }
            break;
        }
    case CodeAccessorMethod::GET:
        text = QStringLiteral("return ") + fieldName + QLatin1Char(';');
        break;
    case CodeAccessorMethod::LIST:
        text = QStringLiteral("return (List) ") + fieldName + QLatin1Char(';');
        break;
    case CodeAccessorMethod::REMOVE:
        {
            int minOccurs = javafield->minimumListOccurances();
            QString fieldType = javafield->getTypeName();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            QString indent = getIndentation();

            if(minOccurs > 0)
                text += QStringLiteral("if (") + fieldName + QStringLiteral(".size() >= ") + QString::number(minOccurs) + QStringLiteral(") {") + endLine + indent;
            text += fieldName + QStringLiteral(".remove(value);");
            if(minOccurs > 0)
            {
                text += endLine + QStringLiteral("} else {") + endLine;
                text += indent + QStringLiteral("System.err.println(\"ERROR: Cant remove") + fieldType + QStringLiteral(" from ") + fieldName + QStringLiteral(", minimum number of items reached.\");") + endLine + QLatin1Char('}') + endLine;
            }
            break;
        }
    case CodeAccessorMethod::SET:
        text = fieldName + QStringLiteral(" = value;");
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
        methodName = QStringLiteral("add") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QStringLiteral("void");
        methodParams = objectType + QStringLiteral(" value ");
        headerText = QStringLiteral("Add an object of type ") + objectType + QStringLiteral(" to the List ") + fieldName + endLine + getParentObject()->doc() + endLine + QStringLiteral("@return void");
        break;
    case CodeAccessorMethod::GET:
        methodName = QStringLiteral("get") + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = fieldType;
        headerText = QStringLiteral("Get the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + QStringLiteral("@return the value of ") + fieldName;
        break;
    case CodeAccessorMethod::LIST:
        methodName = QStringLiteral("get") + Codegen_Utils::capitalizeFirstLetter(fieldType) + QStringLiteral("List");
        methodReturnType = QStringLiteral("List");
        headerText = QStringLiteral("Get the list of ") + fieldName + endLine + getParentObject()->doc() + endLine + QStringLiteral("@return List of ") + fieldName;
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QStringLiteral("remove") + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = QStringLiteral("void");
        methodParams = objectType + QStringLiteral(" value ");
        headerText = QStringLiteral("Remove an object of type ") + objectType + QStringLiteral(" from the List ") + fieldName + endLine + getParentObject()->doc();
        break;
    case CodeAccessorMethod::SET:
        methodName = QStringLiteral("set") + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = QStringLiteral("void");
        methodParams = fieldType + QStringLiteral(" value ");
        headerText = QStringLiteral("Set the value of ") + fieldName + endLine + getParentObject()->doc() + endLine;
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
    setStartMethodText(strVis + QLatin1Char(' ') + methodReturnType + QLatin1Char(' ') + methodName + QStringLiteral(" (") + methodParams + QStringLiteral(") {"));
    setEndMethodText(QStringLiteral("}"));
}

void JavaCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

