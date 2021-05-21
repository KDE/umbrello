/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-FileCopyrightText: 2014-2020 Ralf Habacker <ralf.habacker@freenet.de>
*/

#ifndef SHARED_H
#define SHARED_H

#include <QString>
#include <QStringList>
#include <QMap>

QStringList XMILanguagesAttributes();

class POEntry {
public:
    QStringList tagNames;
    QString value;
    QList<int> lineNumbers;
};

typedef QMap<QString, POEntry> POMap;


bool extractAttributesFromXMI(const char *fileName, const QStringList &attributes, POMap &result);

QString toGetTextString(const QString &message);

typedef QMap<QString,QString> TranslationMap;

bool fetchPoFile(const QString &fileName, TranslationMap &map);

bool applyTranslationToXMIFile(const char *fileName, const QStringList &attributes, TranslationMap &translations);

#endif
