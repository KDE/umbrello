/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2007 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLPACKAGELIST_H
#define UMLPACKAGELIST_H

#include <qlist.h>

class UMLPackage;

typedef QList<UMLPackage*> UMLPackageList;
typedef QListIterator<UMLPackage*> UMLPackageListIt;

#endif
