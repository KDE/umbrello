/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2013                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

#ifndef CPPIMPORT_H
#define CPPIMPORT_H

#include "classimport.h"

#include <QString>

class CppDriver;

/**
 * C++ code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class CppImport : public ClassImport
{
public:
    explicit CppImport(CodeImpThread* thread = 0);
    virtual ~CppImport();

protected:

    void initialize();

    bool parseFile(const QString& fileName);

private:

    void feedTheModel(const QString& fileName);

    static CppDriver * ms_driver;
    static QStringList ms_seenFiles;  ///< auxiliary buffer for feedTheModel()

};

#endif
