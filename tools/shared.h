/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 *                                                                         *
 *   @author Ralf Habacker <ralf.habacker@freenet.de>                      *
 *                                                                         *
 ***************************************************************************/

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
