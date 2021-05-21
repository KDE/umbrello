/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2010 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLCLASSIFIERLIST_H
#define UMLCLASSIFIERLIST_H

#include <QList>

// forward declaration
class UMLClassifier;

typedef QList<UMLClassifier*> UMLClassifierList;
typedef QListIterator<UMLClassifier*> UMLClassifierListIt;

#endif
