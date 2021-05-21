/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2010 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLENTITYLIST_H
#define UMLENTITYLIST_H

#include <QList>

// forward declaration
class UMLEntity;

typedef QList<UMLEntity*> UMLEntityList;
typedef QListIterator<UMLEntity*> UMLEntityListIt;

#endif
