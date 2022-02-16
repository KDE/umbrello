/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "valaimport.h"

/**
 * Constructor
 */
ValaImport::ValaImport(CodeImpThread *thread)
    : CsValaImportBase(thread)
{
    m_language = Uml::ProgrammingLanguage::Vala;
}

/**
 * Destructor
 */
ValaImport::~ValaImport()
{

}

/**
 * Reimplementation of method from CsValaImportBase
 */
QString ValaImport::fileExtension()
{
    return QLatin1String(".vala");
}
