/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cppsourcecodeclassfielddeclarationblock.h"

#include "cppcodeclassfield.h"
#include "model_utils.h"

CPPSourceCodeClassFieldDeclarationBlock::CPPSourceCodeClassFieldDeclarationBlock ( CodeClassField * parent )
        : CodeClassFieldDeclarationBlock ( parent )
{
    setOverallIndentationLevel(1);
}

CPPSourceCodeClassFieldDeclarationBlock::~CPPSourceCodeClassFieldDeclarationBlock ( )
{
}

void CPPSourceCodeClassFieldDeclarationBlock::updateContent( )
{
    CodeClassField * cf = getParentClassField();
    CPPCodeClassField * jcf = dynamic_cast<CPPCodeClassField*>(cf);

    // Set the comment
    QString notes = getParentObject()->doc();
    getComment()->setText(notes);

    // Set the body
    QString staticValue = getParentObject()->isStatic() ? "static " : "";
    QString scopeStr = getParentObject()->visibility().toString();

    QString typeName = jcf->getTypeName();
    QString fieldName = jcf->getFieldName();
    QString initialV = jcf->getInitialValue();

    QString body = staticValue+scopeStr + ' ' + typeName + ' ' + fieldName;
    if (!initialV.isEmpty())
            body.append(" = " + initialV);
    setText(body + ';');
}

#include "cppsourcecodeclassfielddeclarationblock.moc"
