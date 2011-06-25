/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodeclassfield.h"

// local includes
#include "attribute.h"
#include "debug_utils.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodecomment.h"
#include "rubycodegenerator.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

// qt includes
#include <QtCore/QRegExp>

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

RubyCodeClassField::~RubyCodeClassField ( )
{
}

QString RubyCodeClassField::getFieldName()
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
            return roleName.toLower() + "Array";
        }
    }
}


QString RubyCodeClassField::getInitialValue()
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
            // FIX : IF the multiplicity is "1" then we should init a new object here, if it is 0 or 1,
            //       then we can just return 'empty' string (minor problem).
            return QString("");
        } else {
            return RubyCodeGenerator::getListFieldClassName()+".new()";
        }
    }

}

QString RubyCodeClassField::getTypeName ( )
{
    return RubyCodeGenerator::cppToRubyType(CodeClassField::getTypeName());
}

#include "rubycodeclassfield.moc"
