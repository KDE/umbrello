/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODEOPERATION_H
#define JAVACODEOPERATION_H

#include "codeoperation.h"

#include <QString>

class JavaClassifierCodeDocument;

class JavaCodeOperation : public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    JavaCodeOperation (JavaClassifierCodeDocument * doc, UMLOperation * op, const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeOperation ();

    virtual int lastEditableLine();

protected:

    void updateMethodDeclaration();

};

#endif // JAVACODEOPERATION_H
