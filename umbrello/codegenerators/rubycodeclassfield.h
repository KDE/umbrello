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
 ***************************************************************************/

#ifndef RUBYCODECLASSFIELD_H
#define RUBYCODECLASSFIELD_H

#include <qstring.h>

#include "../codeclassfield.h"

class RubyClassifierCodeDocument;

class RubyCodeClassField : public CodeClassField
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Constructors 
     */
    RubyCodeClassField (RubyClassifierCodeDocument * parentDoc, UMLRole * role);
    RubyCodeClassField (RubyClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeClassField ( );

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ( );
protected:

private:

    // void initDeclCodeBlock ();

};

#endif // RUBYCODECLASSFIELD_H
