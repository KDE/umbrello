/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2007 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLASSOCIATIONLIST_H
#define UMLASSOCIATIONLIST_H

#include <qlist.h>

// forward declaration
class UMLAssociation;

typedef QList<UMLAssociation*> UMLAssociationList;
typedef QListIterator<UMLAssociation*> UMLAssociationListIt;

#endif
