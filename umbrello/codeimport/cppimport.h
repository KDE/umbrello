/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPIMPORT_H
#define CPPIMPORT_H

#include "classimport.h"

#include <QString>

class CppDriver;

/**
 * C++ code import
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class CppImport : public ClassImport
{
public:
    explicit CppImport(CodeImpThread* thread = 0);
    virtual ~CppImport();

protected:

    void initialize();

    void initPerFile();

    bool parseFile(const QString& fileName);

private:

    void feedTheModel(const QString& fileName);

    static CppDriver * ms_driver;
    static QStringList ms_seenFiles;  ///< auxiliary buffer for feedTheModel()

};

#endif
