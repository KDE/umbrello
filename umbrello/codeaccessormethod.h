/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEACCESSORMETHOD_H
#define CODEACCESSORMETHOD_H

#include <QtCore/QString>
#include "codemethodblock.h"

class CodeClassField;

class CodeAccessorMethod : public CodeMethodBlock
{
    friend class CodeClassField;
    Q_OBJECT
public:

    // some types of accessor methods that are possible:
    // "GET" is to retrieve single-valued (primative or Object) fields
    // "SET" is to set single-valued (primative or Object) fields
    // "ADD" is to add a value to a multiple-valued field of either primative or Object items
    // "REMOVE" is to remove a value to a multiple-valued field of either primative or Object items
    // "LIST" is to retrive the entire list of items in a  multiple-valued field
    enum AccessorType {GET=0, SET, ADD, REMOVE, LIST};

    CodeAccessorMethod(CodeClassField * field);
    virtual ~CodeAccessorMethod();

    CodeClassField * getParentClassField();

    AccessorType getType();

    void setType(AccessorType type);

    // virtual UMLObject * getParentObject();

    bool parentIsAttribute();

    virtual void updateContent() = 0;

    virtual void saveToXMI(QDomDocument & doc, QDomElement & root);
    virtual void loadFromXMI(QDomElement & root);

    virtual void setAttributesFromObject(TextBlock * obj);

protected:

    virtual void release();

    virtual void setAttributesOnNode(QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode(QDomElement & element);

    virtual void updateMethodDeclaration() = 0;

    void forceRelease();

private:

    CodeClassField * m_parentclassfield;
    AccessorType m_accessorType;

    void initFields(CodeClassField * parentCF);

};

#endif // CODEACCESSORMETHOD_H
