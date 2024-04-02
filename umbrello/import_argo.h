/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef IMPORT_ARGO_H
#define IMPORT_ARGO_H

#include <QIODevice>

class UMLPackage;
class KZip;

/**
 * Argo model import
 *
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class Import_Argo
{
public:
    static bool loadFromZArgoFile(QIODevice & file, UMLPackage *parentPkg = nullptr);

protected:
    static bool loadFromArgoFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromPGMLFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromTodoFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromXMIFile(const KZip &zipFile, const QString &fileName);
};

#endif
