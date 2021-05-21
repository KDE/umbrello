/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODECLASSFIELD_H
#define JAVACODECLASSFIELD_H

#include "codeclassfield.h"

#include <QString>

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
    virtual ~JavaCodeClassField ();

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ();
protected:

private:

    // void initDeclCodeBlock ();

};

#endif // JAVACODECLASSFIELD_H
