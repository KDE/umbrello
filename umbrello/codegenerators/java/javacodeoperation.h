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

#ifndef JAVACODEOPERATION_H
#define JAVACODEOPERATION_H

#include "codeoperation.h"

#include <QtCore/QString>

class JavaClassifierCodeDocument;

class JavaCodeOperation : virtual public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    JavaCodeOperation ( JavaClassifierCodeDocument * doc, UMLOperation * op, const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeOperation ( );

    virtual int lastEditableLine();

protected:

    void updateMethodDeclaration();

};

#endif // JAVACODEOPERATION_H
