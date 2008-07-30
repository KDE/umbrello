/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODECLASSFIELD_H
#define RUBYCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QtCore/QString>

class ClassifierCodeDocument;

class RubyCodeClassField : public CodeClassField
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);
    RubyCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

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
