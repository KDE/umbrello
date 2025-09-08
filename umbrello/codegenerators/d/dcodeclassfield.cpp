/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodeclassfield.h"

// local includes
#include "umlattribute.h"
#include "dcodecomment.h"
#include "dcodegenerator.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "umlapp.h"

// #include "dcodeaccessormethod.h"
#include "dclassifiercodedocument.h"

// qt includes

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
            return roleName + (roleName.right(1) == QStringLiteral("s") ? QStringLiteral("es") : QStringLiteral("s"));
        }
    }
}

QString DCodeClassField::getInitialValue()
{
    if (parentIsAttribute())
    {
        const UMLAttribute * at = getParentObject()->asUMLAttribute();
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            logError0("parent object is not a UMLAttribute");
            return QString();
        }
    }
    else
    {
        if(fieldIsSingleValue()) {
            // FIX : IF the multiplicity is "1" then we should init a new object here, if its 0 or 1,
            //       then we can just return 'empty' string (minor problem).
            return QString();
        } else {
            return QStringLiteral(" new ") + DCodeGenerator::getListFieldClassName() + QStringLiteral("()");
        }
    }

}

QString DCodeClassField::getTypeName()
{
    return DCodeGenerator::fixTypeName(CodeClassField::getTypeName());
}

