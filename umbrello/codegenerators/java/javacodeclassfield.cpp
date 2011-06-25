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
#include "javacodeclassfield.h"

// local includes
#include "attribute.h"
#include "debug_utils.h"
#include "javacodecomment.h"
#include "javacodegenerator.h"
// #include "javacodeaccessormethod.h"
#include "javaclassifiercodedocument.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

JavaCodeClassField::JavaCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

JavaCodeClassField::JavaCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

JavaCodeClassField::~JavaCodeClassField ( )
{
}

QString JavaCodeClassField::getFieldName()
{
    if (parentIsAttribute())
    {
        UMLAttribute * at = (UMLAttribute*) getParentObject();
        return cleanName(at->name());
    }
    else
    {
        UMLRole * role = (UMLRole*) getParentObject();
        QString roleName = role->name();
        if(fieldIsSingleValue()) {
            return roleName.replace(0, 1, roleName.left(1).toLower());
        } else {
            return roleName.toLower() + "Vector";
        }
    }
}

QString JavaCodeClassField::getInitialValue()
{
    if (parentIsAttribute())
    {
        UMLAttribute * at = dynamic_cast<UMLAttribute*>( getParentObject() );
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            uError() << "parent object is not a UMLAttribute";
            return "";
        }
        return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
    }
    else
    {
        if(fieldIsSingleValue()) {
            // FIX : IF the multiplicity is "1" then we should init a new object here, if its 0 or 1,
            //       then we can just return 'empty' string (minor problem).
            return QString("");
        } else {
            return " new "+JavaCodeGenerator::getListFieldClassName()+"( )";
        }
    }
}

QString JavaCodeClassField::getTypeName ( )
{
    return JavaCodeGenerator::fixTypeName(CodeClassField::getTypeName());
}

#include "javacodeclassfield.moc"
