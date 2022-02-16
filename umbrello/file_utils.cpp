/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "file_utils.h"

// app includes
#define DBG_SRC QLatin1String("File_Utils")
#include "debug_utils.h"
#include "uml.h"  // Only needed for logDebug

// qt includes
#include <QFileInfo>
#include <QCoreApplication>

DEBUG_REGISTER(File_Utils)

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
    logDebug1("File_Utils::xmlCatalogFilePath: %1", fi.canonicalFilePath());
    return fi.canonicalFilePath();
}

} // namespace File_Utils
