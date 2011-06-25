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
#include "cppcodeclassfield.h"

// local includes
#include "attribute.h"
#include "classifiercodedocument.h"
#include "codegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

CPPCodeClassField::CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{
}

CPPCodeClassField::CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{
}

CPPCodeClassField::~CPPCodeClassField ( )
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
            return roleName.toLower() + "Vector";
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
        UMLAttribute * at = dynamic_cast<UMLAttribute*>( getParentObject() );
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            uError() << "parent object is not a UMLAttribute";
            return "";
        }
    }
    else
    {
        if(fieldIsSingleValue()) {
            // FIX : IF the multiplicity is "1" then we should init a new object here, if its 0 or 1,
            //       then we can just return 'empty' string (minor problem).
            return "";
        } else {
            return " new "+getListFieldClassName()+"( )";
        }
    }
}

#include "cppcodeclassfield.moc"
