/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "rubycodeclassfielddeclarationblock.h"

#include "rubycodeclassfield.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "../classifier.h"
#include "../umlrole.h"
#include "../codegenerator.h"
#include "../uml.h"

// Constructors/Destructors
//

RubyCodeClassFieldDeclarationBlock::RubyCodeClassFieldDeclarationBlock ( CodeClassField * parent )
        : CodeClassFieldDeclarationBlock ( parent )
{
    setOverallIndentationLevel(1);
    updateContent();
}

RubyCodeClassFieldDeclarationBlock::~RubyCodeClassFieldDeclarationBlock ( ) { }

//
// Methods
//



// Other methods
//

/**
 */
void RubyCodeClassFieldDeclarationBlock::updateContent( )
{

    CodeClassField * cf = getParentClassField();
    ClassifierCodeDocument * doc = cf->getParentDocument();
    RubyCodeClassField * rcf = dynamic_cast<RubyCodeClassField*>(cf);
    RubyClassifierCodeDocument* rdoc = dynamic_cast<RubyClassifierCodeDocument*>(doc);
    CodeGenerationPolicy * p = UMLApp::app()->getCommonPolicy();
    CodeGenerationPolicy::ScopePolicy scopePolicy = p->getAssociationFieldScope();

    // Set the comment
    QString notes = getParentObject()->getDoc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->getStatic() ? "static " : "";
    QString scopeStr = rdoc->scopeToRubyDecl(getParentObject()->getVisibility());

    // IF this is from an association, then scope taken as appropriate to policy
    if(!rcf->parentIsAttribute())
    {
        switch (scopePolicy) {
        case CodeGenerationPolicy::Public:
        case CodeGenerationPolicy::Private:
        case CodeGenerationPolicy::Protected:
            scopeStr = rdoc->scopeToRubyDecl((Uml::Visibility::Value) scopePolicy);
            break;
        default:
        case CodeGenerationPolicy::FromParent:
            // do nothing here... will leave as from parent object
            break;
        }
    }

    QString typeName = rcf->getTypeName();
    QString fieldName = rcf->getFieldName();
    QString initialV = rcf->getInitialValue();

    if (!cf->parentIsAttribute() && !cf->fieldIsSingleValue())
        typeName = "Array";

    QString body = staticValue+scopeStr+' '+typeName+' '+fieldName;
    if (!initialV.isEmpty())
        body.append(" = " + initialV);
    else if (!cf->parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(cf->getParentObject());
        if (role->getObject()->getBaseType() == Uml::ot_Interface)
        {
            // do nothing.. can't instanciate an interface
        } else {

            // FIX?: IF a constructor method exists in the classifiercodedoc
            // of the parent Object, then we can use that instead (if its empty).
            if(cf->fieldIsSingleValue())
            {
                if(!typeName.isEmpty())
                    body.append(" = " + typeName + ".new()");
            } else
                body.append(" = []");
        }
    }

    setText(body);

}

#include "rubycodeclassfielddeclarationblock.moc"
