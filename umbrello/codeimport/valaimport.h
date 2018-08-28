/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  copyright (C) 2018                                                     *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

#ifndef VALAIMPORT_H
#define VALAIMPORT_H

#include "csharpimport.h"

class ValaImport : public CSharpImport
{
public:
    explicit ValaImport(CodeImpThread* thread = 0);
    virtual ~ValaImport();
    QString fileExtension();

};

#endif // VALAIMPORT_H
