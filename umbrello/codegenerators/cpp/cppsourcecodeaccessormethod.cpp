/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppsourcecodeaccessormethod.h"

// local includes
#include "attribute.h"
#include "classifiercodedocument.h"
#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodeclassfield.h"
#include "cppcodedocumentation.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

CPPSourceCodeAccessorMethod::CPPSourceCodeAccessorMethod(CodeClassField * field, CodeAccessorMethod::AccessorType type)
  : CodeAccessorMethod(field)
{
    setType(type);
    setEndMethodText(QLatin1String("}"));
}

void CPPSourceCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

CPPSourceCodeAccessorMethod::~CPPSourceCodeAccessorMethod()
{
}

// we basically want to update the body of this method
void CPPSourceCodeAccessorMethod::updateContent()
{
    CodeClassField * parentField = getParentClassField();
    CPPCodeClassField * cppfield = dynamic_cast<CPPCodeClassField*>(parentField);

    // Check for dynamic casting failure!
    if(cppfield == 0)
    {
        logError0("cppfield: invalid dynamic cast");
        return;
    }

    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);

    // Check for dynamic casting failure!
    if(policy == 0)
    {
        logError0("policy: invalid dynamic cast");
        return;
    }

    bool isInlineMethod = policy->getAccessorsAreInline();

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
            text = variableName + QLatin1String(" = value;");
            break;
        case CodeAccessorMethod::LIST:
        case CodeAccessorMethod::GET:
        default:
            text = QLatin1String("return ") + variableName + QLatin1Char(';');
            break;
        }
    }

    setText(text);
}

// we basically want to update the start text of this method
void CPPSourceCodeAccessorMethod::updateMethodDeclaration()
{
    CodeClassField * parentField = getParentClassField();
    ClassifierCodeDocument * doc = parentField->getParentDocument();
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);

    // Check for dynamic casting failure!
    if (policy == 0)
    {
        logError0("policy: invalid dynamic cast");
        return;
    }

    CPPCodeClassField * cppfield = dynamic_cast<CPPCodeClassField*>(parentField);

    // Check for dynamic casting failure!
    if (cppfield == 0)
    {
        logError0("cppfield: invalid dynamic cast");
        return;
    }

    UMLClassifier * c = doc->getParentClassifier();

    bool isInlineMethod = policy->getAccessorsAreInline();
    QString tag = policy->getDocToolTag();

    QString vectorClassName = policy->getVectorClassName();
    QString fieldName = cppfield->getFieldName();
    QString fieldType = cppfield->getTypeName();
    QString objectType = cppfield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;

    QString methodReturnType(QLatin1String("void"));
    QString methodName; // QLatin1String("get") + cppdoc->capitalizeFirstLetter(fieldName);
    QString methodParams = QChar(QLatin1Char(' ')); // QLatin1String("get") + cppdoc->capitalizeFirstLetter(fieldName);
    QString headerText;
    QString className = CodeGenerator::cleanName(c->name());
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = QLatin1String("add_") + fieldType;
        methodReturnType = QLatin1String("void");
        methodParams = objectType + QLatin1String(" value ");
        headerText = QLatin1String("Add a ") + fieldName + QLatin1String(" object to the ") + fieldName + QLatin1String("List") + endLine + getParentObject()->doc() + endLine + tag + QLatin1String("return void");
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = QLatin1String("remove_") + fieldType;
        methodParams = objectType + QLatin1String(" value ");
        methodReturnType = QLatin1String("void");
        headerText = QLatin1String("Remove a ") + fieldName + QLatin1String(" object from the ") + fieldName + QLatin1String("List") + endLine + getParentObject()->doc() + endLine + tag + QLatin1String("return void");
        break;
    case CodeAccessorMethod::LIST:
        methodName = QLatin1String("get_") + fieldType + QLatin1String("_list");
        methodReturnType = vectorClassName;
        headerText = QLatin1String("Get the ") + fieldName + QLatin1String("List") + endLine + getParentObject()->doc() + endLine + tag + QLatin1String("return ") + vectorClassName + QLatin1String("with list of objects");
        break;
    case CodeAccessorMethod::SET:
        methodName = QLatin1String("set_") + fieldName;
        methodParams = fieldType + QLatin1String(" value ");
        methodReturnType = QLatin1String("void");
        headerText = QLatin1String("Set the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + tag + QLatin1String("param value the value of ") + fieldName;
        break;
    case CodeAccessorMethod::GET:
    default:
        methodName = QLatin1String("get_") + fieldName;
        methodReturnType = fieldType;
        headerText = QLatin1String("Get the value of ") + fieldName + endLine + getParentObject()->doc() + endLine + tag + QLatin1String("return the value of ") + fieldName;
        break;
    }

    // set header
    CPPCodeDocumentation * header = new CPPCodeDocumentation(doc);
    if(!getParentObject()->doc().isEmpty())
        header->setText(headerText);
    setComment(header);

    // set start method text (EndText never changes)
    setStartMethodText(methodReturnType + QLatin1Char(' ') + className + QLatin1String("::") + methodName + QLatin1String(" (") + methodParams + QLatin1Char(')') + QLatin1String(" {"));

    setOverallIndentationLevel(0);

    // these ONLY appear if they arent inline
    if(isInlineMethod)
        setWriteOutText(false);

}

