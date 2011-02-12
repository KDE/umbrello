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

JavaCodeAccessorMethod::JavaCodeAccessorMethod ( CodeClassField * field, CodeAccessorMethod::AccessorType type)
        : CodeAccessorMethod ( field )
{
    setType(type);

    // lets use full-blown comment
    JavaClassifierCodeDocument* jccd = dynamic_cast<JavaClassifierCodeDocument*>(field->getParentDocument());
    setComment(new JavaCodeDocumentation(jccd));
}

JavaCodeAccessorMethod::~JavaCodeAccessorMethod ( )
{
}

void JavaCodeAccessorMethod::setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement)
{
    // set super-class attributes
    CodeAccessorMethod::setAttributesOnNode(doc, blockElement);

    // set local attributes now
}

void JavaCodeAccessorMethod::setAttributesFromNode( QDomElement & root)
{
    // set attributes from superclass method the XMI
    CodeAccessorMethod::setAttributesFromNode(root);

    // load local stuff
}

void JavaCodeAccessorMethod::updateContent( )
{
    CodeClassField * parentField = getParentClassField();
    JavaCodeClassField * javafield = dynamic_cast<JavaCodeClassField*>(parentField);
    QString fieldName = javafield->getFieldName();

    QString text = "";
    switch(getType()) {
    case CodeAccessorMethod::ADD:
        {
            int maxOccurs = javafield->maximumListOccurances();
            QString fieldType = javafield->getTypeName();
            QString indent = getIndentation();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            if(maxOccurs > 0)
                text += "if ("+fieldName+".size() < "+ QString::number(maxOccurs)+") {"+endLine+indent;
            text += fieldName+".add(value);";
            if(maxOccurs > 0)
            {
                text += endLine+"} else {"+endLine;
                text += indent + "System.err.println(\"ERROR: Cant add"+fieldType+" to "+fieldName+", minimum number of items reached.\");"+endLine+'}'+endLine;
            }
            break;
        }
    case CodeAccessorMethod::GET:
        text = "return "+fieldName+';';
        break;
    case CodeAccessorMethod::LIST:
        text = "return (List) "+fieldName+';';
        break;
    case CodeAccessorMethod::REMOVE:
        {
            int minOccurs = javafield->minimumListOccurances();
            QString fieldType = javafield->getTypeName();
            QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
            QString indent = getIndentation();

            if(minOccurs > 0)
                text += "if ("+fieldName+".size() >= "+ QString::number(minOccurs)+") {"+endLine+indent;
            text += fieldName+".remove(value);";
            if(minOccurs > 0)
            {
                text += endLine+"} else {"+endLine;
                text += indent + "System.err.println(\"ERROR: Cant remove"+fieldType+" from "+fieldName+", minimum number of items reached.\");"+endLine+'}'+endLine;
            }
            break;
        }
    case CodeAccessorMethod::SET:
        text = fieldName+" = value;";
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
    Uml::Visibility::Value scopePolicy = commonpolicy->getAttributeAccessorScope();
    QString strVis = javafield->getVisibility().toString();
    QString fieldName = javafield->getFieldName();
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
              strVis = Uml::Visibility::toString((Uml::Visibility::Value) scopePolicy);
            break;
        default:
        case Uml::Visibility::FromParent:
            // do nothing..already have taken parent value
            break;
        }

    // some variables we will need to populate
    QString headerText = "";
    QString methodReturnType = "";
    QString methodName = "";
    QString methodParams = "";

    switch(getType()) {
    case CodeAccessorMethod::ADD:
        methodName = "add" + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = "void";
        methodParams = objectType+" value ";
        headerText = "Add an object of type "+objectType+" to the List "+fieldName+endLine+getParentObject()->doc()+endLine+"@return void";
        break;
    case CodeAccessorMethod::GET:
        methodName = "get" + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = fieldType;
        headerText = "Get the value of "+fieldName+endLine+getParentObject()->doc()+endLine+"@return the value of "+fieldName;
        break;
    case CodeAccessorMethod::LIST:
        methodName = "get" + Codegen_Utils::capitalizeFirstLetter(fieldType)+"List";
        methodReturnType = "List";
        headerText = "Get the list of "+fieldName+endLine+getParentObject()->doc()+endLine+"@return List of "+fieldName;
        break;
    case CodeAccessorMethod::REMOVE:
        methodName = "remove" + Codegen_Utils::capitalizeFirstLetter(fieldType);
        methodReturnType = "void";
        methodParams = objectType+" value ";
        headerText = "Remove an object of type "+objectType+" from the List "+fieldName+endLine+getParentObject()->doc();
        break;
    case CodeAccessorMethod::SET:
        methodName = "set" + Codegen_Utils::capitalizeFirstLetter(fieldName);
        methodReturnType = "void";
        methodParams = fieldType + " value ";
        headerText = "Set the value of "+fieldName+endLine+getParentObject()->doc()+endLine;
        break;
    default:
        // do nothing..no idea what this is
        uWarning()<<"Warning: cant generate JavaCodeAccessorMethod for type: "<<getType();
        break;
    }

    // set header once.
    if(getComment()->getText().isEmpty())
        getComment()->setText(headerText);

    // set start/end method text
    setStartMethodText(strVis+' '+methodReturnType+' '+methodName+" ( "+methodParams+" ) {");
    setEndMethodText("}");
}

void JavaCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

#include "javacodeaccessormethod.moc"
