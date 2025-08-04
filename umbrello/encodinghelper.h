/*
    SPDX-FileCopyrightText: 2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

class EncodingHelper {
public:
    static bool setEncoding(QTextStream &stream, const QString &encodingName) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto enc = QStringConverter::encodingForName(encodingName);
        if (!enc.has_value()) {
            return false;
        }
        stream.setEncoding(enc.value());
        return true;
#else
        QTextCodec *codec = QTextCodec::codecForName(encodingName.toUtf8());
        if (!codec) {
            return false;
        }
        stream.setCodec(codec);
        return true;
#endif
    }

    static bool setEncodingByCharsetNum(QTextStream &stream, int charSetNum) {
        switch (charSetNum) {
            case 0:         // ASCII
            case 1:         // Default
                return setEncoding(stream, QStringLiteral("System"));
            case 2:         // Symbol
                return false; // No known codec
            case 77:
                return setEncoding(stream, QStringLiteral("macintosh"));
            case 128:
                return setEncoding(stream, QStringLiteral("Shift_JIS"));
            case 129:
            case 130:
                return setEncoding(stream, QStringLiteral("EUC-KR"));
            case 134:
                return setEncoding(stream, QStringLiteral("GB18030"));
            case 136:
                return setEncoding(stream, QStringLiteral("Big5"));
            case 161:
                return setEncoding(stream, QStringLiteral("windows-1253"));
            case 162:
                return setEncoding(stream, QStringLiteral("windows-1254"));
            case 163:
                return setEncoding(stream, QStringLiteral("windows-1258"));
            case 177:
                return setEncoding(stream, QStringLiteral("windows-1255"));
            case 178:
                return setEncoding(stream, QStringLiteral("windows-1256"));
            case 186:
                return setEncoding(stream, QStringLiteral("windows-1257"));
            case 204:
                return setEncoding(stream, QStringLiteral("windows-1251"));
            case 222:
                return setEncoding(stream, QStringLiteral("TIS-620"));
            case 238:
                return setEncoding(stream, QStringLiteral("windows-1250"));
            case 255:
                return setEncoding(stream, QStringLiteral("windows-1252"));
            default:
                return false; // unknown charset
        }
    }
};
