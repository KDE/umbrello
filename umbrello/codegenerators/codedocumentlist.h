/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEDOCUMENTLIST_H
#define CODEDOCUMENTLIST_H

#include <QList>

// forward declarations
class CodeDocument;

typedef QList<CodeDocument*> CodeDocumentList;
typedef QListIterator<CodeDocument*> CodeDocumentListIt;

#endif
