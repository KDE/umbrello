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

#ifndef CODEOPERATION_H
#define CODEOPERATION_H

#include "codemethodblock.h"
#include "operation.h"

#include <QtCore/QString>

/**
 *
 */
class CodeOperation : public CodeMethodBlock
{
    Q_OBJECT
public:

    /**
     * Constructors
     */
    CodeOperation ( ClassifierCodeDocument * doc , UMLOperation * parent,
                    const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~CodeOperation ( );

    UMLOperation * getParentOperation( );

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

    static QString findTag (UMLOperation * op) ;

    virtual void setAttributesFromObject (TextBlock * obj);

    virtual void updateMethodDeclaration() = 0;
    virtual void updateContent();

protected:

    // list of parameters used by this code operation.
    // QList<CodeParameter*> m_parameterVector;

    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode ( QDomElement & element);

private:

    UMLOperation * m_parentOperation;
    void init (UMLOperation * parentOp);

};

#endif // CODEOPERATION_H
