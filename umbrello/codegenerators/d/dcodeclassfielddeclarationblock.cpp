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

#include "dcodeclassfielddeclarationblock.h"

#include "dcodeclassfield.h"
#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "codegenerator.h"
#include "classifier.h"
#include "umlrole.h"
#include "uml.h"

DCodeClassFieldDeclarationBlock::DCodeClassFieldDeclarationBlock ( CodeClassField * parent )
        : CodeClassFieldDeclarationBlock ( parent )
{
    setOverallIndentationLevel(1);
}

DCodeClassFieldDeclarationBlock::~DCodeClassFieldDeclarationBlock ( )
{
}

/**
 */
void DCodeClassFieldDeclarationBlock::updateContent( )
{
    CodeClassField * cf = getParentClassField();
    DCodeClassField * jcf = dynamic_cast<DCodeClassField*>(cf);
    CodeGenerationPolicy * commonpolicy = UMLApp::app()->commonPolicy();

    Uml::Visibility::Value scopePolicy = commonpolicy->getAssociationFieldScope();

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? "static " : "";
    QString scopeStr = getParentObject()->visibility().toString();

    // IF this is from an association, then scope taken as appropriate to policy
    if(!jcf->parentIsAttribute())
    {
        switch (scopePolicy) {
        case Uml::Visibility::Public:
        case Uml::Visibility::Private:
        case Uml::Visibility::Protected:
              scopeStr = Uml::Visibility::toString((Uml::Visibility::Value) scopePolicy);
            break;
        default:
        case Uml::Visibility::FromParent:
            // do nothing here... will leave as from parent object
            break;
        }
    }

    QString typeName = jcf->getTypeName();
    QString fieldName = jcf->getFieldName();
    QString initialV = jcf->getInitialValue();

    if (!cf->parentIsAttribute() && !cf->fieldIsSingleValue())
        typeName = "List";

    QString body = staticValue+scopeStr+' '+typeName+' '+fieldName;
    if (!initialV.isEmpty())
        body.append(" = " + initialV);
    else if (!cf->parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(cf->getParentObject());
        if (role->object()->baseType() == UMLObject::ot_Interface)
        {
            // do nothing.. can't instanciate an interface
        } else {

            // FIX?: IF a constructor method exists in the classifiercodedoc
            // of the parent Object, then we can use that instead (if its empty).
            if(cf->fieldIsSingleValue())
            {
                if(!typeName.isEmpty())
                    body.append(" = new " + typeName + " ( )");
            } else
                body.append(" = new Vector ( )");
        }
    }

    setText(body+';');

}

#include "dcodeclassfielddeclarationblock.moc"
