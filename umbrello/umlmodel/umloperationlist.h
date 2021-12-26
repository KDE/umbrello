/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2007 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOPERATIONLIST_H
#define UMLOPERATIONLIST_H

#include <qlist.h>

// forward declaration
class UMLOperation;

typedef QList<UMLOperation*> UMLOperationList;
typedef QListIterator<UMLOperation*> UMLOperationListIt;

#endif
