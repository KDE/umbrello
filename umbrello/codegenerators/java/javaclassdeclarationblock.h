/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACLASSDECLARATIONBLOCK_H
#define JAVACLASSDECLARATIONBLOCK_H

#include "javaclassifiercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QString>

class JavaClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit JavaClassDeclarationBlock (JavaClassifierCodeDocument * parentDoc, const QString &start = QString(), const QString &endText = QStringLiteral("}"), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~JavaClassDeclarationBlock ();

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI (QDomElement & root);

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ();

private:

    void init (JavaClassifierCodeDocument * parent, const QString &comment);

};

#endif // JAVACLASSDECLARATIONBLOCK_H
