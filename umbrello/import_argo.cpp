/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "import_argo.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>
#include <KTempDir>
#include <KZip>

// qt includes
#include <QFile>
#include <QStringList>
#include <QXmlStreamReader>

static void reportError(const QXmlStreamReader &xml, const KZip &zipFile, const QString &fileName)
{
    uError() << xml.name() << "in file" << zipFile.fileName() << ":" << fileName;
}

bool Import_Argo::loadFromArgoFile(const KZip &zipFile, const QString &fileName)
{
    const KArchiveFile *file = static_cast<const KArchiveFile*>(zipFile.directory()->entry(fileName));
    if (!file)
        return false;

    QXmlStreamReader xml;
    xml.addData(file->data());

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.name() == "member") {
            QXmlStreamAttributes attributes = xml.attributes();
            QString type = attributes.value(QLatin1String("type")).toString();
            QString name = attributes.value(QLatin1String("name")).toString();
            if (type == QLatin1String("xmi"))
                loadFromXMIFile(zipFile, name);
            else if (type == QLatin1String("pgml"))
                loadFromPGMLFile(zipFile, name);
            else if (type == QLatin1String("todo"))
                loadFromTodoFile(zipFile, name);
            else
                uError() << "unknown file type" << type << "in file" << zipFile.fileName() << ":" << fileName;
        }
    }
    if (xml.hasError()) {
         reportError(xml, zipFile, fileName);
         return false;
    }
    return true;
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

    KTempDir tmpDir;
    tmpDir.setAutoRemove(true);
    file->copyTo(tmpDir.name());

    QFile xmiFile(tmpDir.name() + file->name());
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
