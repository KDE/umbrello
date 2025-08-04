/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPHEADERCODEOPERATION_H
#define CPPHEADERCODEOPERATION_H

#include "codeoperation.h"

#include <QString>

class CPPHeaderCodeDocument;

class CPPHeaderCodeOperation : public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CPPHeaderCodeOperation (CPPHeaderCodeDocument * doc, UMLOperation * op,
                             const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderCodeOperation ();

    virtual int lastEditableLine();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

protected:

    /**
     * Check to see if we have a valid stereotype to apply in the operation
     */
    virtual void applyStereotypes (QString&, UMLOperation *, bool, bool, QString&, QString&);

};

#endif // CPPHEADERCODEOPERATION_H
