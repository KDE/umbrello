/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPCODECLASSFIELD_H
#define CPPCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QString>

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
    virtual ~CPPCodeClassField ();

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
