/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODECLASSFIELDLIST_H
#define CODECLASSFIELDLIST_H

#include <QList>

// forward declarations
class CodeClassField;

typedef QList<CodeClassField*> CodeClassFieldList;
typedef QListIterator<CodeClassField*> CodeClassFieldListIt;

#endif
