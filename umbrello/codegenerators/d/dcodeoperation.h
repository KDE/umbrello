/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODEOPERATION_H
#define DCODEOPERATION_H

#include "codeoperation.h"

#include <QtCore/QString>

class DClassifierCodeDocument;

class DCodeOperation : virtual public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    DCodeOperation ( DClassifierCodeDocument * doc, UMLOperation * op, const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~DCodeOperation ( );

    virtual int lastEditableLine();

protected:

    virtual void updateMethodDeclaration();

};

#endif // DCODEOPERATION_H
