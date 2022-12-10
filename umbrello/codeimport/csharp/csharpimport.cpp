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
 
/**
 * Reimplement operation from CsValaImportBase.
 * @param word   whitespace delimited item
 */
void CSharpImport::fillSource(const QString& word)
{
    CsValaImportBase::fillSource(word);
    if (m_source.isEmpty())
        return;
    // Map .NET types to their native C# equivalents
    static const char *dotNet2CSharp[] = {
        "System.Boolean", "bool",
        "System.Byte",    "byte",
        "System.SByte",   "sbyte",
        "System.Char",    "char",
        "System.Decimal", "decimal",
        "System.Double",  "double",
        "System.Single",  "float",
        "System.Int32",   "int",
        "System.UInt32",  "uint",
        "System.IntPtr",  "nint",
        "System.UIntPtr", "nuint",
        "System.Int64",   "long",
        "System.UInt64",  "ulong",
        "System.Int16",   "short",
        "System.UInt16",  "ushort",
        "System.Object",  "object",
        "System.String",  "string",
    };
    QString& last = m_source.last();
    for (size_t i = 0; i < sizeof(dotNet2CSharp) / sizeof(char*); i += 2) {
        if (last == QLatin1String(dotNet2CSharp[i]))
            last = QLatin1String(dotNet2CSharp[i + 1]);
    }
}

