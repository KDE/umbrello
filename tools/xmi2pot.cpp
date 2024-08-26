/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-FileCopyrightText: 2014-2020 Ralf Habacker <ralf.habacker@freenet.de>
*/

#include <stdlib.h>
#include <iostream>
#include <qfileinfo.h>
#include <qdatetime.h>

#include <QStringList>
#include <QXmlStreamReader>

#include "shared.h"

using namespace std;

void outputMsg(const char *prefix, const QString &message);

int main( int argc, char **argv )
{
    if (argc != 2) {
        cerr << "usage: " << argv[0] << " english-XMI" << endl;
        exit(1);
    }

    POMap map;
    if (!extractAttributesFromXMI(argv[1], XMILanguagesAttributes(), map)) {
        cerr << "failed to extract attributes from: '" << argv[1] << "'" << endl;
        exit(2);
    }

    const QDateTime now = QDateTime::currentDateTime().toUTC();

    cout << "# SOME DESCRIPTIVE TITLE.\n";
    cout << "# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n";
    cout << "#\n";
    cout << "#, fuzzy\n";
    cout << "msgid \"\"\n";
    cout << "msgstr \"\"\n";
    cout << "\"Project-Id-Version: PACKAGE VERSION\\n\"\n";
    cout << "\"Report-Msgid-Bugs-To: https://bugs.kde.org\\n\"\n";
    cout << "\"POT-Creation-Date: " << now.toString(QStringLiteral("yyyy-MM-dd hh:mm")).toUtf8().data() << "+0000\\n\"\n";
    cout << "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n";
    cout << "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n";
    cout << "\"Language-Team: LANGUAGE <kde-i18n-doc@kde.org>\\n\"\n";
    cout << "\"MIME-Version: 1.0\\n\"\n";
    cout << "\"Content-Type: text/plain; charset=UTF-8\\n\"\n";
    cout << "\"Content-Transfer-Encoding: 8bit\\n\"\n";
    cout << "\n";

    const QString fname = QFileInfo(QLatin1String(argv[1])).fileName();

    for (POMap::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        cout << "#. Tag: " << (*it).tagNames.join(QStringLiteral(" ")).toUtf8().data() << '\n';
        cout << "#: ";
        for (QList<int>::ConstIterator it2 =
                 (*it).lineNumbers.constBegin(); it2 != (*it).lineNumbers.constEnd(); ++it2) {
            if (it2 != (*it).lineNumbers.constBegin())
                cout << " ";
            cout << fname.toUtf8().data() << ":" << (*it2);

        }
        cout << "\n";
        cout << "#, no-c-format\n";
        cout << "msgid" << toGetTextString((*it).value).toUtf8().data() << '\n';
        cout << "msgstr \"\"\n";
        cout << "\n";
    }

    return 0;
}

