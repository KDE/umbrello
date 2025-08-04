/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEOPERATION_H
#define DCODEOPERATION_H

#include "codeoperation.h"

#include <QString>

class DClassifierCodeDocument;

class DCodeOperation : public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    DCodeOperation (DClassifierCodeDocument * doc, UMLOperation * op, const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~DCodeOperation ();

    virtual int lastEditableLine();

protected:

    virtual void updateMethodDeclaration();

};

#endif // DCODEOPERATION_H
