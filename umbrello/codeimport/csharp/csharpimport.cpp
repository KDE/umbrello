/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "csharpimport.h"

/**
 * Constructor.
 */
CSharpImport::CSharpImport(CodeImpThread *thread)
    : CsValaImportBase(thread)
{
    m_language = Uml::ProgrammingLanguage::CSharp;
}

/**
 * Destructor.
 */
CSharpImport::~CSharpImport()
{

}

/**
 * Reimplementation of method from CsValaImportBase
 */
QString CSharpImport::fileExtension()
{
    return QLatin1String(".cs");
}
