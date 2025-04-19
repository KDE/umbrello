/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CSHARPIMPORT_H
#define CSHARPIMPORT_H

#include "csvalaimportbase.h"

/**
 * Derivation of CsValaImportBase for C#
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class CSharpImport : public CsValaImportBase
{
public:
    explicit CSharpImport(CodeImpThread *thread = nullptr);
    virtual ~CSharpImport();
    QString fileExtension();
    void fillSource(const QString& word);

protected:
    void spawnImport(const QString& file);
};

#endif
