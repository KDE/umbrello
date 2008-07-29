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

#ifndef CPPCODECLASSFIELD_H
#define CPPCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QtCore/QString>

class ClassifierCodeDocument;

class CPPCodeClassField : public CodeClassField
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);
    CPPCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

    /**
     * Empty Destructor
     */
    virtual ~CPPCodeClassField ( );

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    /**
     * Get the name of the class which holds lists, e.g. "QPtrlist" or
     * "Vector" or "List" and so on.
     */
    QString getListFieldClassName();


protected:

private:

};

#endif // CPPCODECLASSFIELD_H
