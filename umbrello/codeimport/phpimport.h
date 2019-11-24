/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2017                                                     *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

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
    explicit PHPImport(CodeImpThread* thread = 0);
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
