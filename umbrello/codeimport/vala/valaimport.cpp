/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "valaimport.h"

#include <QFile>

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
    return QStringLiteral(".vala");
}

/**
 * Spawn off an import of the specified file.
 * @param file   the specified file
 */
void ValaImport::spawnImport(const QString& file)
{
    // if the file is being parsed, don't bother
    if (s_filesAlreadyParsed.contains(file)) {
        return;
    }
    if (QFile::exists(file)) {
        ValaImport importer;
        QStringList fileList;
        fileList.append(file);
        s_filesAlreadyParsed.append(file);
        importer.importFiles(fileList);
    }
}
