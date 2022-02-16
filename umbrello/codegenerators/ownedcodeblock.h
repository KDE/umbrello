/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OWNEDCODEBLOCK_H
#define OWNEDCODEBLOCK_H

#include <QDomDocument>
#include <QDomElement>
#include <QObject>

class TextBlock;
class UMLObject;
class QXmlStreamWriter;

/**
 * Describes any codeblock which is 'owned' by a UMLobject of some sort and should
 * be in sync with that parent.
 */
class OwnedCodeBlock : public QObject
{
    Q_OBJECT
public:

    explicit OwnedCodeBlock (UMLObject * parent);
    virtual ~OwnedCodeBlock ();

    UMLObject * getParentObject ();

    virtual void updateContent () = 0;

protected:

    virtual void release ();

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);
    virtual void setAttributesFromNode (QDomElement & element);
    virtual void setAttributesFromObject (TextBlock * obj);

private:

    void initFields (UMLObject * parent);

    UMLObject * m_parentObject;

public slots:

    virtual void syncToParent ();

};

#endif // OWNEDCODEBLOCK_H
