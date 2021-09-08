/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPMAKECODEDOCUMENT_H
#define CPPMAKECODEDOCUMENT_H

#include "codedocument.h"

#include <QString>

/**
 * class CPPMakefileCodeDocument
 * Represents
 */
class CPPMakefileCodeDocument : public CodeDocument
{
    Q_OBJECT
public:

    static const char * DOCUMENT_ID_VALUE;

    /**
     * Constructor
     */
    CPPMakefileCodeDocument ();

    /**
     * Empty Destructor
     */
    virtual ~CPPMakefileCodeDocument ();

    /**
     * @return  QString
     */
    QString toString () const;

    QString getPath () const;

    void updateContent();

protected:

private:

};

#endif // CPPMAKECODEDOCUMENT_H
