/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "rubycodeclassfielddeclarationblock.h"

#include "classifier.h"
#include "codegenerator.h"
#include "rubycodeclassfield.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "umlrole.h"
#include "uml.h"

/**
 * Constructor.
 */
RubyCodeClassFieldDeclarationBlock::RubyCodeClassFieldDeclarationBlock(CodeClassField * parent)
        : CodeClassFieldDeclarationBlock(parent)
{
    setOverallIndentationLevel(1);
}

/**
 * Empty Destructor.
 */
RubyCodeClassFieldDeclarationBlock::~RubyCodeClassFieldDeclarationBlock()
{
}

/**
 * This will be called by syncToParent whenever the parent object is "modified".
 */
void RubyCodeClassFieldDeclarationBlock::updateContent()
{
    CodeClassField * cf = getParentClassField();
    RubyCodeClassField * rcf = dynamic_cast<RubyCodeClassField*>(cf);
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    Uml::Visibility::Enum scopePolicy = p->getAssociationFieldScope();

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? QLatin1String("static ") : QString();
    QString scopeStr = Uml::Visibility::toString(getParentObject()->visibility());

    // IF this is from an association, then scope taken as appropriate to policy
    if (!rcf->parentIsAttribute())
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

    QString typeName = rcf->getTypeName();
    QString fieldName = rcf->getFieldName();
    QString initialV = rcf->getInitialValue();

    if (!cf->parentIsAttribute() && !cf->fieldIsSingleValue())
        typeName = QLatin1String("Array");

    QString body = staticValue + scopeStr + QLatin1Char(' ') + typeName + QLatin1Char(' ') + fieldName;
    if (!initialV.isEmpty())
        body.append(QLatin1String(" = ") + initialV);
    else if (!cf->parentIsAttribute())
    {
        const UMLRole * role = cf->getParentObject()->asUMLRole();
        if (role && role->object()->baseType() == UMLObject::ot_Interface)
        {
            // do nothing.. can't instantiate an interface
        } else {

            // FIX?: IF a constructor method exists in the classifiercodedoc
            // of the parent Object, then we can use that instead (if its empty).
            if (cf->fieldIsSingleValue())
            {
                if (!typeName.isEmpty())
                    body.append(QLatin1String(" = ") + typeName + QLatin1String(".new()"));
            } else
                body.append(QLatin1String(" = []"));
        }
    }

    setText(body);
}

