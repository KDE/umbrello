/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODECLASSFIELD_H
#define DCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QtCore/QString>

class ClassifierCodeDocument;

class DCodeClassField : public CodeClassField
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    DCodeClassField (ClassifierCodeDocument * parentDoc, UMLRole * role);
    DCodeClassField (ClassifierCodeDocument * parentDoc, UMLAttribute * attrib);

    /**
     * Empty Destructor
     */
    virtual ~DCodeClassField ( );

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ( );
protected:

private:

    // void initDeclCodeBlock ();

};

#endif // DCODECLASSFIELD_H
