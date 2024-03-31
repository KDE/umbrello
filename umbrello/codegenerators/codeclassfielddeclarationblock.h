/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODECLASSFIELDDECLARATIONBLOCK_H
#define CODECLASSFIELDDECLARATIONBLOCK_H

class UMLObject;
class CodeClassField;

#include "codeblockwithcomments.h"
#include "ownedcodeblock.h"

/**
 * Used to declare classifier fields (e.g. either class attributes or classifier
 * associations) in the code document for any given code classfield. This is a
 * special CodeBlockWithComments which is "sync'd" to the parent CodeClassField.
 * Note: keep the inheritance sequence: QObject needs to be first in inheritance list.
 * Basically a class to allow for synchronization of the contents based on the
 * values of the parentClassField's parentObject.
 */
class CodeClassFieldDeclarationBlock : public OwnedCodeBlock, public CodeBlockWithComments
{
    friend class CodeClassField;
    Q_OBJECT
public:

    explicit CodeClassFieldDeclarationBlock(CodeClassField * parent);
    virtual ~CodeClassFieldDeclarationBlock();

    CodeClassField * getParentClassField();

    UMLObject * getParentObject();

    virtual void setAttributesFromObject(TextBlock * obj);

    virtual void updateContent() = 0;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI(QDomElement & root);

    virtual Q_SLOT void syncToParent();

protected:

    virtual void release();

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);

    virtual void setAttributesFromNode(QDomElement & element);

    void forceRelease();

private:

    CodeClassField * m_parentclassfield;
    void init(CodeClassField * parent);
};

#endif // CODECLASSFIELDDECLARATIONBLOCK_H
