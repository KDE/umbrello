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

#ifndef CPPSOURCECODEOPERATION_H
#define CPPSOURCECODEOPERATION_H

#include "codeoperation.h"

#include <QtCore/QString>

class CPPSourceCodeDocument;

class CPPSourceCodeOperation : virtual public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Constructor.
     */
    CPPSourceCodeOperation(CPPSourceCodeDocument * doc, UMLOperation * op, const QString & body = "", const QString & comment = "");

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
