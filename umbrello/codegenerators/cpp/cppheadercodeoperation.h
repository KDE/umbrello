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

#ifndef CPPHEADERCODEOPERATION_H
#define CPPHEADERCODEOPERATION_H

#include "codeoperation.h"

#include <QtCore/QString>

class CPPHeaderCodeDocument;

class CPPHeaderCodeOperation : virtual public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CPPHeaderCodeOperation ( CPPHeaderCodeDocument * doc, UMLOperation * op,
                             const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderCodeOperation ( );

    virtual int lastEditableLine();

    virtual void updateMethodDeclaration();
    virtual void updateContent( );

protected:

    /**
     * Check to see if we have a valid stereotype to apply in the operation
     */
    virtual void applyStereotypes (QString&, UMLOperation *, bool, bool, QString&, QString&);

};

#endif // CPPHEADERCODEOPERATION_H
