/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCLASSDECLARATIONBLOCK_H
#define DCLASSDECLARATIONBLOCK_H

#include "dclassifiercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QString>

class DClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit DClassDeclarationBlock (DClassifierCodeDocument * parentDoc, const QString &start = QString(), const QString &endText = QLatin1String("}"), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~DClassDeclarationBlock ();

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * Load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI (QDomElement & root);

    /**
     * Update the start and end text for this hierarchical codeblock.
     */
    void updateContent ();

protected:

    /**
     * Set the class attributes from a passed object.
     */
    virtual void setAttributesFromObject (TextBlock * obj);

private:

    void init (DClassifierCodeDocument * parent, const QString &comment);

};

#endif // DHEADERCLASSDECLARATIONBLOCK_H
