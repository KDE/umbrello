/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef RUBYCLASSDECLARATIONBLOCK_H
#define RUBYCLASSDECLARATIONBLOCK_H

#include "rubyclassifiercodedocument.h"
#include "ownedhierarchicalcodeblock.h"

#include <QString>

class RubyClassDeclarationBlock : public OwnedHierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    explicit RubyClassDeclarationBlock (RubyClassifierCodeDocument * parentDoc, const QString &start = QString(), const QString &endText = QLatin1String("end"), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~RubyClassDeclarationBlock ();

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ();

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI1 (QDomDocument & doc, QDomElement & root);

    /**
     * load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI1 (QDomElement & root);

private:

    void init (RubyClassifierCodeDocument * parent, const QString &comment);

};

#endif // RUBYCLASSDECLARATIONBLOCK_H
