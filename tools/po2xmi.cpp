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
       cerr << "failed to fetch po file: '" << argv[2] << "'" << endl;
       exit(2);
   }

   return applyTranslationToXMIFile(argv[1], XMILanguagesAttributes(), translationMap) ? 0 : 2;
}
