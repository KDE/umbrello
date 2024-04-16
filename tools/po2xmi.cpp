/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
    SPDX-FileCopyrightText: 2014-2020 Ralf Habacker <ralf.habacker@freenet.de>
*/

#include "shared.h"

#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include <fstream>

#include <QList>
#include <QTextStream>
#include <QXmlStreamReader>

using namespace std;

int main( int argc, char **argv )
{
   if (argc != 3) {
       qWarning("usage: %s english-XML translated-PO", argv[0]);
       ::exit(1);
   }

   TranslationMap translationMap;
   if (!fetchPoFile(argv[2], translationMap)) {
       cerr << "failed to fetch po file: '" << argv[2] << "'" ;
       exit(2);
   }

   return applyTranslationToXMIFile(argv[1], XMILanguagesAttributes(), translationMap) ? 0 : 2;
}
