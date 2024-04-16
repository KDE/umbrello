/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "import_argo.h"

// app includes
#define DBG_SRC QStringLiteral("Import_Argo")
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>
#include <KZip>

// qt includes
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>
#include <QXmlStreamReader>

DEBUG_REGISTER(Import_Argo)

static void reportError(const QXmlStreamReader &xml, const KZip &zipFile, const QString &fileName)
{
    logError3("Import_Argo %1 in file %2 : %3", xml.name().toString(), zipFile.fileName(), fileName);
}

bool Import_Argo::loadFromArgoFile(const KZip &zipFile, const QString &fileName)
{
    const KArchiveFile *file = static_cast<const KArchiveFile*>(zipFile.directory()->entry(fileName));
    if (!file)
        return false;

    QXmlStreamReader xml;
    xml.addData(file->data());

    bool result = true;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.name() == QStringLiteral("member")) {
            QXmlStreamAttributes attributes = xml.attributes();
            QString type = attributes.value(QStringLiteral("type")).toString();
            QString name = attributes.value(QStringLiteral("name")).toString();
            if (type == QStringLiteral("xmi") && !loadFromXMIFile(zipFile, name))
                result = false;
            else if (type == QStringLiteral("pgml") && !loadFromPGMLFile(zipFile, name))
                result = false;
            else if (type == QStringLiteral("todo") && loadFromTodoFile(zipFile, name))
                result = false;
            else {
                logError3("loadFromArgoFile unknown file type %1 in file %2 : %3",
                          type, zipFile.fileName(), fileName);
                result = false;
            }
        }
    }
    if (xml.hasError()) {
         reportError(xml, zipFile, fileName);
         result = false;
    }
    return result;
}

bool Import_Argo::loadFromPGMLFile(const KZip &zipFile, const QString &fileName)
{
    const KArchiveFile *file = static_cast<const KArchiveFile*>(zipFile.directory()->entry(fileName));
    if (!file)
        return false;

    QXmlStreamReader xml;
    xml.addData(file->data());

    while (!xml.atEnd()) {
        xml.readNext();
        logDebug3("Import_Argo::loadFromPGMLFile unhandled tag %1 in file %2:%3",
                  xml.name().toString(), zipFile.fileName(), fileName);
    }
    if (xml.hasError()) {
        reportError(xml, zipFile, fileName);
        return false;
    }
    return true;
}

bool Import_Argo::loadFromTodoFile(const KZip &zipFile, const QString &fileName)
{
    const KArchiveFile *file = static_cast<const KArchiveFile*>(zipFile.directory()->entry(fileName));
    if (!file)
        return false;

    QXmlStreamReader xml;
    xml.addData(file->data());

    while (!xml.atEnd()) {
        xml.readNext();
        logDebug3("Import_Argo::loadFromTodoFile unhandled tag %1 in file %2:%3",
                  xml.name().toString(), zipFile.fileName(), fileName);
    }
    if (xml.hasError()) {
        reportError(xml, zipFile, fileName);
        return false;
    }
    return true;
}

bool Import_Argo::loadFromXMIFile(const KZip &zipFile, const QString &fileName)
{
    const KArchiveFile *file = static_cast<const KArchiveFile*>(zipFile.directory()->entry(fileName));
    if (!file)
        return false;

    QTemporaryDir tmpDir;
    tmpDir.setAutoRemove(true);

    file->copyTo(tmpDir.path());
    QFile xmiFile(tmpDir.path() + QLatin1Char('/') + file->name());
    if(!xmiFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    return UMLApp::app()->document()->loadFromXMI(xmiFile, 0);
}

bool Import_Argo::loadFromZArgoFile(QIODevice &file, UMLPackage *parentPkg)
{
    Q_UNUSED(parentPkg);

    KZip zipFile(&file);
    if (!zipFile.open(QIODevice::ReadOnly))
        return false;

    const KArchiveDirectory *directory = zipFile.directory();
    bool result = true;
    Q_FOREACH(const QString &name, directory->entries()) {
        const KArchiveEntry *entry = directory->entry(name);
        if (entry->isFile()) {
            const KArchiveFile *file = static_cast<const KArchiveFile*>(entry);
            if (file == 0) {
                logError1("loadFromZArgoFile: Could not read file from %1", name);
                continue;
            }
            if (name.endsWith(QStringLiteral(".argo")))
                result = loadFromArgoFile(zipFile, name);
        }
    }
    return result;
}
