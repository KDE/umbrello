/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPSOURCECODEOPERATION_H
#define CPPSOURCECODEOPERATION_H

#include "codeoperation.h"

#include <QString>

class CPPSourceCodeDocument;

class CPPSourceCodeOperation : public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Constructor.
     */
    CPPSourceCodeOperation(CPPSourceCodeDocument * doc, UMLOperation * op, const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CPPSourceCodeOperation();

    /**
     * Update the doc and start text of this method.
     */
    virtual void updateMethodDeclaration();

    /**
     * Just want to know whether or not to print out
     * the body of the operation.
     * In C++ if the operations are inline, then we DON'T print out
     * the body text.
     */
    virtual void updateContent();

};

#endif // CPPSOURCECODEOPERATION_H
