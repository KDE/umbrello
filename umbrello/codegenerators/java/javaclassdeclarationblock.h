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

#ifndef JAVACLASSDECLARATIONBLOCK_H
#define JAVACLASSDECLARATIONBLOCK_H

#include "javaclassifiercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QtCore/QString>

class JavaClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit JavaClassDeclarationBlock ( JavaClassifierCodeDocument * parentDoc, const QString &start = "", const QString &endText = "}", const QString &comment = "");

    /**
     * Empty Destructor
     */
    virtual ~JavaClassDeclarationBlock ( );

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI ( QDomElement & root );

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ( );

private:

    void init (JavaClassifierCodeDocument * parent, const QString &comment);

};

#endif // JAVACLASSDECLARATIONBLOCK_H
