/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppcodeclassfield.h"

// local includes
#include "umlattribute.h"
#include "classifiercodedocument.h"
#include "codegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "umlapp.h"

CPPCodeClassField::CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

CPPCodeClassField::CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

CPPCodeClassField::~CPPCodeClassField ()
{
}

QString CPPCodeClassField::getFieldName()
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
            return roleName.toLower() + QStringLiteral("Vector");
        }
    }
}

QString CPPCodeClassField::getListFieldClassName ()
{
    CodeGenPolicyExt * p = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy *policy = dynamic_cast<CPPCodeGenerationPolicy*>(p);
    return policy->getVectorClassName();
}

QString CPPCodeClassField::getInitialValue()
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
            return QStringLiteral(" new ") + getListFieldClassName() + QStringLiteral("()");
        }
    }
}

