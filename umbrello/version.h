/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMBRELLO_VERSION_H
#define UMBRELLO_VERSION_H

inline QByteArray umbrelloVersion()
{
#ifdef UMBRELLO_VERSION_STRING
    QString versionStr = QString::fromLatin1(UMBRELLO_VERSION_STRING);
#else
    QString versionStr = QString::fromLatin1("%1.%2.%3")
        .arg(KDE::versionMajor())
        .arg(KDE::versionMinor())
        .arg(KDE::versionRelease());
#endif
#if defined(ENABLE_WIDGET_SHOW_DOC) || defined(ENABLE_XMIRESOLUTION)
    versionStr.append(QStringLiteral(" (experimental)"));
#endif
    return versionStr;
}

// Update this version and dtd's in doc/xml when changing the XMI file format
#define XMI1_FILE_VERSION "1.7.6"
#define XMI2_FILE_VERSION "2.0.4"

#endif
