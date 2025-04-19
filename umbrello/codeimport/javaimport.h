/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVAIMPORT_H
#define JAVAIMPORT_H

#include "javacsvalaimportbase.h"

/**
 * Java code import
 * @author Oliver Kellogg
 * @author JP Fournier
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class JavaImport : public JavaCsValaImportBase
{
public:
    explicit JavaImport(CodeImpThread *thread = nullptr);
    virtual ~JavaImport();
    QString fileExtension();

protected:
    bool parseStmt();

    void spawnImport(const QString& file);
};

#endif

