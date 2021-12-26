/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMBRELLO_VERSION_H
#define UMBRELLO_VERSION_H

#if QT_VERSION < 0x050000
#include <kdeversion.h>
#endif

inline QByteArray umbrelloVersion()
{
#ifdef UMBRELLO_VERSION_STRING
    QString versionStr = QString::fromLatin1(UMBRELLO_VERSION_STRING);
#else
    QString versionStr = QString::fromLatin1("%1.%2.%3")
        .arg(KDE::versionMajor()-2)
        .arg(KDE::versionMinor())
        .arg(KDE::versionRelease());
#endif
#if defined(ENABLE_WIDGET_SHOW_DOC) || defined(ENABLE_XMIRESOLUTION)
    versionStr.append(QLatin1String(" (experimental)"));
#endif
    return versionStr.toLatin1();
}

// Update this version and dtd's in doc/xml when changing the XMI file format
#define XMI1_FILE_VERSION  "1.7.4"
#define XMI2_FILE_VERSION "2.0.0"

#endif
