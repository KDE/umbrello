/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "import_argo.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>
#if QT_VERSION < 0x050000
#include <KTempDir>
#endif
#include <KZip>

// qt includes
#include <QFile>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QTemporaryDir>
#endif
#include <QXmlStreamReader>

static void reportError(const QXmlStreamReader &xml, const KZip &zipFile, const QString &fileName)
{
    logError3("Import_Argo %1 in file %2 : %3", xml.name(), zipFile.fileName(), fileName);
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
        if (xml.name() == QLatin1String("member")) {
            QXmlStreamAttributes attributes = xml.attributes();
            QString type = attributes.value(QLatin1String("type")).toString();
            QString name = attributes.value(QLatin1String("name")).toString();
            if (type == QLatin1String("xmi") && !loadFromXMIFile(zipFile, name))
                result = false;
            else if (type == QLatin1String("pgml") && !loadFromPGMLFile(zipFile, name))
                result = false;
            else if (type == QLatin1String("todo") && loadFromTodoFile(zipFile, name))
                result = false;
            else {
                uError() << "unknown file type" << type << "in file" << zipFile.fileName() << ":" << fileName;
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
        uDebug() << "unhandled tag" << xml.name() << "in file" <<  zipFile.fileName() << ":" << fileName;
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
        uDebug() << "unhandled tag" << xml.name() << "in file" << zipFile.fileName() << ":" << fileName;
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

#if QT_VERSION >= 0x050000
    QTemporaryDir tmpDir;
#else
    KTempDir tmpDir;
#endif
    tmpDir.setAutoRemove(true);

#if QT_VERSION >= 0x050000
    file->copyTo(tmpDir.path());
    QFile xmiFile(tmpDir.path() + QLatin1Char('/') + file->name());
#else
    file->copyTo(tmpDir.name());
    QFile xmiFile(tmpDir.name() + file->name());
#endif
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
    foreach(const QString &name, directory->entries()) {
        const KArchiveEntry *entry = directory->entry(name);
        if (entry->isFile()) {
            const KArchiveFile *file = static_cast<const KArchiveFile*>(entry);
            if (file == 0) {
                uError() << "Could not read file from" << file;
                continue;
            }
            if (name.endsWith(QLatin1String(".argo")))
                result = loadFromArgoFile(zipFile, name);
        }
    }
    return result;
}
