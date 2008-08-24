/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPMAKECODEDOCUMENT_H
#define CPPMAKECODEDOCUMENT_H

#include "codedocument.h"

#include <QtCore/QString>

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
    CPPMakefileCodeDocument ( );

    /**
     * Empty Destructor
     */
    virtual ~CPPMakefileCodeDocument ( );

    /**
     * @return  QString
     */
    QString toString ( );

    QString getPath ( );

    void updateContent();

protected:

private:

};

#endif // CPPMAKECODEDOCUMENT_H
