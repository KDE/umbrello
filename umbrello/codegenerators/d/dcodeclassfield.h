/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODECLASSFIELD_H
#define DCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QString>

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
    virtual ~DCodeClassField ();

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ();
protected:

private:

    // void initDeclCodeBlock ();

};

#endif // DCODECLASSFIELD_H
