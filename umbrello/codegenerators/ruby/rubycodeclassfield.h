/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODECLASSFIELD_H
#define RUBYCODECLASSFIELD_H

#include "codeclassfield.h"

#include <QString>

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
    virtual ~RubyCodeClassField ();

    QString getFieldType();
    QString getFieldName();
    QString getInitialValue();

    QString getTypeName ();

protected:

private:

    // void initDeclCodeBlock ();

};

#endif // RUBYCODECLASSFIELD_H
