/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  copyright (C) 2018                                                     *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

#include "valaimport.h"

ValaImport::ValaImport(CodeImpThread *thread)
    : CSharpImport(thread)
{
    m_language = Uml::ProgrammingLanguage::Vala;
}

ValaImport::~ValaImport()
{

}

QString ValaImport::fileExtension()
{
    return QLatin1String(".vala");
}
