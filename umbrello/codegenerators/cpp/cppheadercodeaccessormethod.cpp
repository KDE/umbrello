/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppheadercodeaccessormethod.h"

// local includes
#include "umlattribute.h"
#include "classifiercodedocument.h"
#include "cppcodegenerator.h"
#include "cppsourcecodedocument.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodeclassfield.h"
#include "cppcodedocumentation.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "umlapp.h"

CPPHeaderCodeAccessorMethod::CPPHeaderCodeAccessorMethod(CodeClassField * field, CodeAccessorMethod::AccessorType type)
  : CodeAccessorMethod(field)
{
    setType(type);
}

void CPPHeaderCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

CPPHeaderCodeAccessorMethod::~CPPHeaderCodeAccessorMethod()
{
}

// we basically want to update the body of this method
void CPPHeaderCodeAccessorMethod::updateContent()
{
    CodeClassField * parentField = getParentClassField();
    CPPCodeClassField * cppfield = dynamic_cast<CPPCodeClassField*>(parentField);

    // Check for dynamic casting failure!
    if (cppfield == nullptr)
    {
        logError0("cppfield: invalid dynamic cast");
        return;
    }

    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);

    // Check for dynamic casting failure!
    if (policy == nullptr)
    {
        logError0("policy: invalid dynamic cast");
        return;
    }

    bool isInlineMethod = policy->getAccessorsAreInline();
    // Uml::Visibility scope = parentField->getVisibility();
    QString variableName = cppfield->getFieldName();
    QString itemClassName = cppfield->getTypeName();
    QString text;

    if(isInlineMethod) {
        switch(getType()) {
        case CodeAccessorMethod::ADD:
            text = policy->getVectorMethodAppend(variableName, itemClassName);
            break;
        case CodeAccessorMethod::REMOVE:
            text = policy->getVectorMethodRemove(variableName, itemClassName);
            break;
        case CodeAccessorMethod::SET:
            text = variableName + QStringLiteral(" = value;");
            break;
        case CodeAccessorMethod::LIST:
        case CodeAccessorMethod::GET:
        default:
            text = QStringLiteral("return ") + variableName + QLatin1Char(';');
            break;
        }
    }

    setText(text);
}

// we basically want to update the start text of this method
void CPPHeaderCodeAccessorMethod::updateMethodDeclaration()
{
    CodeClassField * parentField = getParentClassField();
    ClassifierCodeDocument * doc = parentField->getParentDocument();
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    CPPCodeClassField * cppfield = dynamic_cast<CPPCodeClassField*>(parentField);

    bool isInlineMethod = policy->getAccessorsAreInline();
    QString tag = policy->getDocToolTag();
    QString classMemberPrefix = policy->getClassMemberPrefix();

    QString vectorClassName = policy->getVectorClassName();
    QString fieldName = classMemberPrefix + cppfield->getFieldName();
    QString fieldType = cppfield->getTypeName();
    QString objectType = cppfield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;

    QString methodReturnType = QStringLiteral("void");
    QString methodName; // QStringLiteral("get") + cppdoc->capitalizeFirstLetter(fieldName);
    QString methodParams = QChar(QLatin1Char(' ')); // QStringLiteral("get") + cppdoc->capitalizeFirstLetter(fieldName);
    QString headerText;
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = QStringLiteral("add_") + fieldType;
        methodReturnType = QStringLiteral("void");
        methodParams = objectType + QStringLiteral(" value ");
        headerText = QStringLiteral("Add a ") + fieldName + QStringLiteral(" object to the ") + fieldName + QStringLiteral("List") + endLine + getParentObject()->doc() + endLine + tag + QStringLiteral("return void");
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QStringLiteral("remove_") + fieldType;
        methodParams = objectType + QStringLiteral(" value ");
        methodReturnType = QStringLiteral("void");
        headerText = QStringLiteral("Remove a ") + fieldName + QStringLiteral(" object from the ") + fieldName + QStringLiteral("List") + endLine + getParentObject()->doc() + endLine + tag + QStringLiteral("return void");
        break;
    case CodeAccessorMethod::LIST:
        methodName = QStringLiteral("get_") + fieldType + QStringLiteral("_list");
        methodReturnType = vectorClassName;
        headerText = QStringLiteral("Get the ") + fieldName + QStringLiteral("List") + endLine + getParentObject()->doc() + endLine + tag + QStringLiteral("return ") + vectorClassName + QStringLiteral("with list of objects");
        break;
    case CodeAccessorMethod::SET:
        methodName = QStringLiteral("set_") + fieldName;
        methodParams = fieldType + QStringLiteral(" value ");
        methodReturnType = QStringLiteral("void");
        headerText = QStringLiteral("Set the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + tag + QStringLiteral("param value the value of ") + fieldName;
        break;
    case CodeAccessorMethod::GET:
    default:
        methodName = QStringLiteral("get_") + fieldName;
        methodReturnType = fieldType;
        headerText = QStringLiteral("Get the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + tag + QStringLiteral("return the value of ") + fieldName;
        break;
    }

    // set header
    CPPCodeDocumentation * header = new CPPCodeDocumentation(doc);
    if(!getParentObject()->doc().isEmpty())
        header->setText(headerText);
    setComment(header);

    // set start/end method text
    QString startText = methodReturnType + QLatin1Char(' ') + methodName + QStringLiteral(" (") + methodParams  + QLatin1Char(')');
    if (isInlineMethod)
        startText += QStringLiteral(" {");
    else
        startText += QLatin1Char(';');
    QString endText = (isInlineMethod ? QStringLiteral("}") : QString());

    setStartMethodText(startText);
    setEndMethodText(endText);

    setOverallIndentationLevel(1);
}

