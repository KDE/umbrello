/*
    SPDX-FileCopyrightText: 025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QIODevice>
#include <QString>
#include <QFile>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KCompressionDevice>
#else
#include <KFilterDev>
#endif

class CompressionHelper {
public:
    /**
     * Opens a compressed file transparently (supports gzip, bzip2).
     * Returns a QIODevice pointer that must be deleted by the caller.
     */
    static QIODevice* deviceForFile(const QString &fileName) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        KCompressionDevice *dev = new KCompressionDevice(fileName);
        if (!dev->open(QIODevice::ReadOnly)) {
            delete dev;
            return nullptr;
        }
        return dev;
#else
        QIODevice *dev = new KFilterDev(fileName);
        if (!dev || !dev->open(QIODevice::ReadOnly)) {
            delete dev;
            return nullptr;
        }
        return dev;
#endif
    }
};
