/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

JavaCodeClassField::~JavaCodeClassField ()
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
            return roleName.toLower() + QLatin1String("Vector");
        }
    }
}

QString JavaCodeClassField::getInitialValue()
{
    if (parentIsAttribute())
    {
        const UMLAttribute * at = getParentObject()->asUMLAttribute();
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            uError() << "parent object is not a UMLAttribute";
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
            return QLatin1String(" new ") + JavaCodeGenerator::getListFieldClassName() + QLatin1String("()");
        }
    }
}

QString JavaCodeClassField::getTypeName ()
{
    return JavaCodeGenerator::fixTypeName(CodeClassField::getTypeName());
}

