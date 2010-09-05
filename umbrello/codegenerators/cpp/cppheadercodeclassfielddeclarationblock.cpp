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

#include "cppheadercodeclassfielddeclarationblock.h"

#include "cppcodeclassfield.h"
#include "cppheadercodedocument.h"


CPPHeaderCodeClassFieldDeclarationBlock::CPPHeaderCodeClassFieldDeclarationBlock ( CodeClassField * parent )
        : CodeClassFieldDeclarationBlock ( parent )
{
    setOverallIndentationLevel(1);
}

CPPHeaderCodeClassFieldDeclarationBlock::~CPPHeaderCodeClassFieldDeclarationBlock ( )
{
}

void CPPHeaderCodeClassFieldDeclarationBlock::updateContent( )
{
    UMLObject *umlparent = CodeClassFieldDeclarationBlock::getParentObject();
    if (umlparent == NULL) {
        return;
    }

    CodeClassField * cf = getParentClassField();
    CPPCodeClassField * hcppcf = dynamic_cast<CPPCodeClassField*>(cf);

    // Set the comment
    QString notes = umlparent->doc();
    getComment()->setText(notes);
    if (notes.isEmpty())
        getComment()->setWriteOutText(false);
    else
        getComment()->setWriteOutText(true);

    // Set the body
    QString staticValue = umlparent->isStatic() ? "static " : "";
    QString typeName = hcppcf->getTypeName();
    QString fieldName = hcppcf->getFieldName();

    // Ugh. Sloppy exception.
    if (!cf->parentIsAttribute() && !cf->fieldIsSingleValue())
        typeName = hcppcf->getListFieldClassName();

    QString body = staticValue + ' ' + typeName + ' ' + fieldName + ';';
    setText(body);
}

#include "cppheadercodeclassfielddeclarationblock.moc"
