/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2017-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PHPIMPORT_H
#define PHPIMPORT_H

#include "classimport.h"

#include <QString>

class PHPImportPrivate;

/**
 * PHP code import
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PHPImport : public ClassImport
{
public:
    explicit PHPImport(CodeImpThread* thread = nullptr);
    virtual ~PHPImport();

protected:
    void initialize();
    void initPerFile();
    bool parseFile(const QString& fileName);

private:
    void feedTheModel(const QString& fileName);
    PHPImportPrivate *m_d;
};

#endif
