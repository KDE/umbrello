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
#include "dcodeclassfield.h"

// local includes
#include "attribute.h"
#include "dcodecomment.h"
#include "dcodegenerator.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

// #include "dcodeaccessormethod.h"
#include "dclassifiercodedocument.h"

// qt includes
#include <QtCore/QRegExp>

DCodeClassField::DCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

DCodeClassField::DCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

DCodeClassField::~DCodeClassField()
{
}

QString DCodeClassField::getFieldName()
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
        roleName = roleName.replace(0, 1, roleName[0].toLower());

        if(fieldIsSingleValue()) {
            return roleName;
        } else {
            return roleName + (roleName.right(1) == "s" ? "es" : "s");
        }
    }
}

QString DCodeClassField::getInitialValue()
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
            return " new "+DCodeGenerator::getListFieldClassName()+"( )";
        }
    }

}

QString DCodeClassField::getTypeName()
{
    return DCodeGenerator::fixTypeName(CodeClassField::getTypeName());
}

#include "dcodeclassfield.moc"
