/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef JAVACODECLASSFIELD_H
#define JAVACODECLASSFIELD_H

#include "codeclassfield.h"

#include <QtCore/QString>

class ClassifierCodeDocument;

class JavaCodeClassField : public CodeClassField
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    JavaCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);
    JavaCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeClassField ( );

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ( );
protected:

private:

    // void initDeclCodeBlock ();

};

#endif // JAVACODECLASSFIELD_H
