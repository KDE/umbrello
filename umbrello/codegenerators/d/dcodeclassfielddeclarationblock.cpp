/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "dcodeclassfielddeclarationblock.h"

#include "dcodeclassfield.h"
#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "codegenerator.h"
#include "classifier.h"
#include "umlrole.h"
#include "uml.h"

/**
 * Constructor.
 */
DCodeClassFieldDeclarationBlock::DCodeClassFieldDeclarationBlock(CodeClassField * parent)
        : CodeClassFieldDeclarationBlock(parent)
{
    setOverallIndentationLevel(1);
}

/**
 * Empty Destructor.
 */
DCodeClassFieldDeclarationBlock::~DCodeClassFieldDeclarationBlock()
{
}

/**
 * This will be called by syncToParent whenever the parent object is "modified".
 */
void DCodeClassFieldDeclarationBlock::updateContent()
{
    CodeClassField * cf = getParentClassField();
    DCodeClassField * jcf = dynamic_cast<DCodeClassField*>(cf);
    CodeGenerationPolicy * commonpolicy = UMLApp::app()->commonPolicy();

    Uml::Visibility::Enum scopePolicy = commonpolicy->getAssociationFieldScope();

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? QLatin1String("static ") : QString();
    QString scopeStr = Uml::Visibility::toString(getParentObject()->visibility());

    // IF this is from an association, then scope taken as appropriate to policy
    if(!jcf->parentIsAttribute())
    {
        switch (scopePolicy) {
        case Uml::Visibility::Public:
        case Uml::Visibility::Private:
        case Uml::Visibility::Protected:
              scopeStr = Uml::Visibility::toString(scopePolicy);
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
        typeName = QLatin1String("List");

    QString body = staticValue + scopeStr + QLatin1Char(' ') + typeName + QLatin1Char(' ') + fieldName;
    if (!initialV.isEmpty())
        body.append(QLatin1String(" = ") + initialV);
    else if (!cf->parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(cf->getParentObject());
        if (role->object()->baseType() == UMLObject::ot_Interface)
        {
            // do nothing.. can't instantiate an interface
        } else {

            // FIX?: IF a constructor method exists in the classifiercodedoc
            // of the parent Object, then we can use that instead (if its empty).
            if(cf->fieldIsSingleValue())
            {
                if(!typeName.isEmpty())
                    body.append(QLatin1String(" = new ") + typeName + QLatin1String(" ()"));
            } else
                body.append(QLatin1String(" = new Vector ()"));
        }
    }

    setText(body + QLatin1Char(';'));

}
