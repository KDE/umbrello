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

#ifndef CPPHEADERCLASSDECLARATIONBLOCK_H
#define CPPHEADERCLASSDECLARATIONBLOCK_H

#include "cppheadercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QtCore/QString>

class CPPHeaderClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit CPPHeaderClassDeclarationBlock ( CPPHeaderCodeDocument * parentDoc, const QString &start = "", const QString &endText = "}", const QString &comment = "");

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderClassDeclarationBlock ( );

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ( );

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

protected:

    /**
     * Set the class attributes from a passed object.
     */
    virtual void setAttributesFromObject (TextBlock * obj);

private:

    void init (CPPHeaderCodeDocument * parent, const QString &comment);

};

#endif // CPPHEADERCLASSDECLARATIONBLOCK_H
