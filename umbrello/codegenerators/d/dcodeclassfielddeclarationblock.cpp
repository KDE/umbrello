/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "dcodeclassfielddeclarationblock.h"

#include "dcodeclassfield.h"
#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "codegenerator.h"
#include "umlclassifier.h"
#include "umlrole.h"
#include "uml.h"
#include "debug_utils.h"

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

    if (!jcf)
    {
       logError0("jcf: invalid dynamic cast");
       return;
    }

    CodeGenerationPolicy * commonpolicy = UMLApp::app()->commonPolicy();

    Uml::Visibility::Enum scopePolicy = commonpolicy->getAssociationFieldScope();

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? QStringLiteral("static ") : QString();
    QString scopeStr = Uml::Visibility::toString(getParentObject()->visibility());

    // IF this is from an association, then scope taken as appropriate to policy
    if (!jcf->parentIsAttribute())
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
        typeName = QStringLiteral("List");

    QString body = staticValue + scopeStr + QLatin1Char(' ') + typeName + QLatin1Char(' ') + fieldName;
    if (!initialV.isEmpty())
        body.append(QStringLiteral(" = ") + initialV);
    else if (!cf->parentIsAttribute())
    {
        const UMLRole * role = cf->getParentObject()->asUMLRole();

        // Check for dynamic casting failure!
        if (role == nullptr)
        {
            logError0("role: invalid dynamic cast");
            return;
        }

        if (role->object()->baseType() == UMLObject::ot_Interface)
        {
            // do nothing.. can't instantiate an interface
        } else {

            // FIX?: IF a constructor method exists in the classifiercodedoc
            // of the parent Object, then we can use that instead (if its empty).
            if(cf->fieldIsSingleValue())
            {
                if(!typeName.isEmpty())
                    body.append(QStringLiteral(" = new ") + typeName + QStringLiteral(" ()"));
            } else
                body.append(QStringLiteral(" = new Vector ()"));
        }
    }

    setText(body + QLatin1Char(';'));

}

