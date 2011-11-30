/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2009                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef CPPIMPORT_H
#define CPPIMPORT_H

#include <QtCore/QString>
#include "classimport.h"

class CppDriver;

/**
 * C++ code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CppImport : public ClassImport
{
public:
    CppImport(CodeImpThread* thread = 0);
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
