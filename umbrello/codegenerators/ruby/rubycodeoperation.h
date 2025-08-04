/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODEOPERATION_H
#define RUBYCODEOPERATION_H

#include "codeoperation.h"

#include <QString>

class RubyClassifierCodeDocument;

class RubyCodeOperation : public CodeOperation
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    RubyCodeOperation (RubyClassifierCodeDocument * doc, UMLOperation * op, const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeOperation ();

    virtual int lastEditableLine();

protected:

    void updateMethodDeclaration();

};

#endif // RUBYCODEOPERATION_H
