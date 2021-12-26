/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEACCESSORMETHOD_H
#define CODEACCESSORMETHOD_H

#include "codemethodblock.h"
#include <QString>

class CodeClassField;
class QXmlStreamWriter;

class CodeAccessorMethod : public CodeMethodBlock
{
    friend class CodeClassField;
    Q_OBJECT
public:

    // some types of accessor methods that are possible:
    // "GET" is to retrieve single-valued (primitive or Object) fields
    // "SET" is to set single-valued (primitive or Object) fields
    // "ADD" is to add a value to a multiple-valued field of either primitive or Object items
    // "REMOVE" is to remove a value to a multiple-valued field of either primitive or Object items
    // "LIST" is to retrieve the entire list of items in a  multiple-valued field
    enum AccessorType {GET=0, SET, ADD, REMOVE, LIST};

    explicit CodeAccessorMethod(CodeClassField * field);
    virtual ~CodeAccessorMethod();

    CodeClassField * getParentClassField();

    AccessorType getType();

    void setType(AccessorType type);

    // virtual UMLObject * getParentObject();

    bool parentIsAttribute();

    virtual void updateContent() = 0;

    virtual void saveToXMI(QXmlStreamWriter& writer);
    virtual void loadFromXMI(QDomElement & root);

    virtual void setAttributesFromObject(TextBlock * obj);

protected:

    virtual void release();

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);

    virtual void setAttributesFromNode(QDomElement & element);

    virtual void updateMethodDeclaration() = 0;

    void forceRelease();

private:

    CodeClassField * m_parentclassfield;
    AccessorType m_accessorType;

    void initFields(CodeClassField * parentCF);

};

#endif // CODEACCESSORMETHOD_H
