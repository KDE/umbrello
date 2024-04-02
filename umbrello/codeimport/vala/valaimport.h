/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef VALAIMPORT_H
#define VALAIMPORT_H

#include "csvalaimportbase.h"

/**
 * Derivation of CsValaImportBase for Vala
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ValaImport : public CsValaImportBase
{
public:
    explicit ValaImport(CodeImpThread* thread = nullptr);
    virtual ~ValaImport();
    QString fileExtension();

};

#endif // VALAIMPORT_H
