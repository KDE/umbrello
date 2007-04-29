/***************************************************************************
                          rubycodeclassfield.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "rubycodeclassfield.h"

// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>

// local includes
#include "rubycodecomment.h"
#include "rubycodegenerator.h"

#include "../attribute.h"
#include "../umlobject.h"
#include "../umlrole.h"
#include "../uml.h"

#include "rubyclassifiercodedocument.h"

// Constructors/Destructors
//

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role)
        : CodeClassField(parentDoc, role)
{

}

RubyCodeClassField::RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib)
        : CodeClassField(parentDoc, attrib)
{

}

RubyCodeClassField::~RubyCodeClassField ( ) { }

//
// Methods
//

// Other methods
//

QString RubyCodeClassField::getFieldName() {
    if (parentIsAttribute())
    {
        UMLAttribute * at = (UMLAttribute*) getParentObject();
        return cleanName(at->getName());
    }
    else
    {
        UMLRole * role = (UMLRole*) getParentObject();
        QString roleName = role->getName();
        if(fieldIsSingleValue()) {
            return roleName.replace(0, 1, roleName.left(1).lower());
        } else {
            return roleName.lower() + "Array";
        }
    }
}


QString RubyCodeClassField::getInitialValue() {

    if (parentIsAttribute())
    {
        UMLAttribute * at = dynamic_cast<UMLAttribute*>( getParentObject() );
        if (at) {
            return fixInitialStringDeclValue(at->getInitialValue(), getTypeName());
        } else {
            kError() << "RubyodeClassField::getInitialValue: parent object is not a UMLAttribute"
            << endl;
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
            return RubyCodeGenerator::getListFieldClassName()+".new()";
        }
    }

}

QString RubyCodeClassField::getTypeName ( )
{
    return RubyCodeGenerator::cppToRubyType(CodeClassField::getTypeName());
}

#include "rubycodeclassfield.moc"
