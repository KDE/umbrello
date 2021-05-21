/*
    SPDX-License-Identifier: GPL-2.0-or-later

    copyright (C) 2020-2020
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "file_utils.h"

// app includes
#include "debug_utils.h"

// qt includes
#include <QFileInfo>
#include <QCoreApplication>

namespace File_Utils {

/**
 * returns path to xml catalog
 * @return string with file path
 */
QString xmlCatalogFilePath()
{
#ifdef Q_OS_WIN
    QString dataRoot = QCoreApplication::applicationDirPath() + QLatin1String("/../");
#else
    QString dataRoot = QLatin1String("/");
#endif
    QFileInfo fi(dataRoot + QLatin1String("etc/xml/catalog"));
    uDebug() << fi.canonicalFilePath();
    return fi.canonicalFilePath();
}

} // namespace File_Utils
