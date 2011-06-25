/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cppheadercodeaccessormethod.h"

// local includes
#include "attribute.h"
#include "classifiercodedocument.h"
#include "cppcodegenerator.h"
#include "cppsourcecodedocument.h"
#include "cppcodegenerationpolicy.h"
#include "cppcodeclassfield.h"
#include "cppcodedocumentation.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

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
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    bool isInlineMethod = policy->getAccessorsAreInline( );
    Uml::Visibility scope = parentField->getVisibility();
    QString variableName = cppfield->getFieldName();
    QString itemClassName = cppfield->getTypeName();
    QString text = "";

    if(isInlineMethod) {
        switch(getType()) {
        case CodeAccessorMethod::ADD:
            text = policy->getVectorMethodAppend(variableName, itemClassName);
            break;
        case CodeAccessorMethod::REMOVE:
            text = policy->getVectorMethodRemove(variableName, itemClassName);
            break;
        case CodeAccessorMethod::SET:
            text = variableName+" = value;";
            break;
        case CodeAccessorMethod::LIST:
        case CodeAccessorMethod::GET:
        default:
            text = "return " + variableName + ';';
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

    bool isInlineMethod = policy->getAccessorsAreInline( );
    QString tag = policy->getDocToolTag( );

    QString vectorClassName = policy->getVectorClassName();
    QString fieldName = cppfield->getFieldName();
    QString fieldType = cppfield->getTypeName();
    QString objectType = cppfield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;

    QString methodReturnType = "void";
    QString methodName; // "get"+cppdoc->capitalizeFirstLetter(fieldName);
    QString methodParams = QChar(' '); // "get"+cppdoc->capitalizeFirstLetter(fieldName);
    QString headerText;
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = "add_"+fieldType;
        methodReturnType = "void";
        methodParams = objectType+" value ";
        headerText = "Add a "+fieldName+" object to the "+fieldName+"List"+endLine+getParentObject()->doc()+endLine+tag+"return void";
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = "remove_"+fieldType;
        methodParams = objectType+" value ";
        methodReturnType = "void";
        headerText = "Remove a "+fieldName+" object from the "+fieldName+"List"+endLine+getParentObject()->doc()+endLine+tag+"return void";
        break;
    case CodeAccessorMethod::LIST:
        methodName = "get_"+fieldType+"_list";
        methodReturnType = vectorClassName;
        headerText = "Get the "+fieldName+"List"+endLine+getParentObject()->doc()+endLine+tag+"return "+vectorClassName+"with list of objects";
        break;
    case CodeAccessorMethod::SET:
        methodName = "set_"+fieldName;
        methodParams = fieldType+" value ";
        methodReturnType = "void";
        headerText = "Set the value of "+fieldName+endLine+getParentObject()->doc()+endLine+tag+"param value the value of "+fieldName;
        break;
    case CodeAccessorMethod::GET:
    default:
        methodName = "get_"+fieldName;
        methodReturnType = fieldType;
        headerText = "Get the value of "+fieldName+endLine+getParentObject()->doc()+endLine+tag+"return the value of "+fieldName;
        break;
    }

    // set header
    CPPCodeDocumentation * header = new CPPCodeDocumentation(doc);
    if(!getParentObject()->doc().isEmpty())
        header->setText(headerText);
    setComment(header);

    // set start/end method text
    QString startText = methodReturnType + ' ' + methodName + " (" + methodParams +')';
    if (isInlineMethod)
        startText += " {";
    else
        startText += ';';
    QString endText = (isInlineMethod ? "}" : "");

    setStartMethodText(startText);
    setEndMethodText(endText);

    setOverallIndentationLevel(1);
}

#include "cppheadercodeaccessormethod.moc"
