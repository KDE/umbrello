/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <QRegExp>

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

RubyCodeClassField::~RubyCodeClassField ()
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
            return roleName.toLower() + QLatin1String("Array");
        }
    }
}


QString RubyCodeClassField::getInitialValue()
{
    if (parentIsAttribute())
    {
        const UMLAttribute * at = getParentObject()->asUMLAttribute();
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            logError0("RubyCodeClassField::getInitialValue: parent object is not a UMLAttribute");
            return QString();
        }
    }
    else
    {
        if(fieldIsSingleValue()) {
            // FIX : IF the multiplicity is "1" then we should init a new object here, if it is 0 or 1,
            //       then we can just return 'empty' string (minor problem).
            return QString();
        } else {
            return RubyCodeGenerator::getListFieldClassName() + QLatin1String(".new()");
        }
    }

}

QString RubyCodeClassField::getTypeName ()
{
    return RubyCodeGenerator::cppToRubyType(CodeClassField::getTypeName());
}

