/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cppsourcecodeclassfielddeclarationblock.h"

#include "cppcodeclassfield.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"  // only needed for log{Warn,Error}

CPPSourceCodeClassFieldDeclarationBlock::CPPSourceCodeClassFieldDeclarationBlock (CodeClassField * parent)
        : CodeClassFieldDeclarationBlock (parent)
{
    setOverallIndentationLevel(1);
}

CPPSourceCodeClassFieldDeclarationBlock::~CPPSourceCodeClassFieldDeclarationBlock ()
{
}

void CPPSourceCodeClassFieldDeclarationBlock::updateContent()
{
    CodeClassField * cf = getParentClassField();
    CPPCodeClassField * jcf = dynamic_cast<CPPCodeClassField*>(cf);

    // Check for dynamic casting failure!
    if (jcf == 0)
    {
        logError0("jcf: invalid dynamic cast");
        return;
    }

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? QLatin1String("static ") : QString();
    QString scopeStr = Uml::Visibility::toString(getParentObject()->visibility());

    QString typeName = jcf->getTypeName();
    QString fieldName = jcf->getFieldName();
    QString initialV = jcf->getInitialValue();

    QString body = staticValue+scopeStr + QLatin1Char(' ') + typeName + QLatin1Char(' ') + fieldName;
    if (!initialV.isEmpty())
            body.append(QLatin1String(" = ") + initialV);
    setText(body + QLatin1Char(';'));
}

