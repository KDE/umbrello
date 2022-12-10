/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPHEADERCLASSDECLARATIONBLOCK_H
#define CPPHEADERCLASSDECLARATIONBLOCK_H

#include "cppheadercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QString>

class CPPHeaderClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit CPPHeaderClassDeclarationBlock (CPPHeaderCodeDocument * parentDoc, const QString &start = QString(), const QString &endText = QStringLiteral("}"), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderClassDeclarationBlock ();

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ();

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI (QDomElement & root);

protected:

    /**
     * Set the class attributes from a passed object.
     */
    virtual void setAttributesFromObject (TextBlock * obj);

private:

    void init (CPPHeaderCodeDocument * parent, const QString &comment);

};

#endif // CPPHEADERCLASSDECLARATIONBLOCK_H
