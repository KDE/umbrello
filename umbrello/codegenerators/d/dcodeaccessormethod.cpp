/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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

DCodeAccessorMethod::DCodeAccessorMethod ( CodeClassField * field, CodeAccessorMethod::AccessorType type)
        : CodeAccessorMethod ( field )
{
    setType(type);

    // lets use full-blown comment
    DClassifierCodeDocument* jccd = dynamic_cast<DClassifierCodeDocument*>(field->getParentDocument());
    setComment(new DCodeDocumentation(jccd));
}

DCodeAccessorMethod::~DCodeAccessorMethod ( )
{
}

void DCodeAccessorMethod::setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement)
{
    // set super-class attributes
    CodeAccessorMethod::setAttributesOnNode(doc, blockElement);

    // set local attributes now
}

void DCodeAccessorMethod::setAttributesFromNode( QDomElement & root)
{
    // set attributes from superclass method the XMI
    CodeAccessorMethod::setAttributesFromNode(root);

    // load local stuff
}

void DCodeAccessorMethod::updateContent( )
{
    CodeClassField * parentField = getParentClassField();
    DCodeClassField * dfield = dynamic_cast<DCodeClassField*>(parentField);
    QString fieldName = dfield->getFieldName();

    QString text = "";
    switch(getType()) {
    case CodeAccessorMethod::ADD:
        {
            int maxOccurs = dfield->maximumListOccurances();
            QString fieldType = dfield->getTypeName();
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
            int minOccurs = dfield->minimumListOccurances();
            QString fieldType = dfield->getTypeName();
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

void DCodeAccessorMethod::updateMethodDeclaration()
{
    DCodeClassField * dfield = dynamic_cast<DCodeClassField*>(getParentClassField());
    CodeGenerationPolicy *commonpolicy = UMLApp::app()->commonPolicy();

    // gather defs
    Uml::Visibility::Value scopePolicy = commonpolicy->getAttributeAccessorScope();
    QString strVis = dfield->getVisibility().toString();
    QString fieldName = dfield->getFieldName();
    QString fieldType = dfield->getTypeName();
    QString objectType = dfield->getListObjectType();
    if(objectType.isEmpty())
        objectType = fieldName;
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    // set scope of this accessor appropriately..if its an attribute,
    // we need to be more sophisticated
    if(dfield->parentIsAttribute())
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
        uWarning()<<"Warning: cant generate DCodeAccessorMethod for type: "<<getType();
        break;
    }

    // set header once.
    if(getComment()->getText().isEmpty())
        getComment()->setText(headerText);

    // set start/end method text
    setStartMethodText(strVis+' '+methodReturnType+' '+methodName+" ( "+methodParams+" ) {");
    setEndMethodText("}");
}

void DCodeAccessorMethod::update()
{
    updateMethodDeclaration();
    updateContent();
}

#include "dcodeaccessormethod.moc"
