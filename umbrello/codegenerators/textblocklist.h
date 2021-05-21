/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TEXTBLOCKLIST_H
#define TEXTBLOCKLIST_H

#include <QList>

// forward declarations
class TextBlock;

typedef QList<TextBlock*> TextBlockList;
typedef QListIterator<TextBlock*> TextBlockListIt;

#endif
