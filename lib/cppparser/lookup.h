/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2005 Tobias Erbsland <te@profzone.ch>
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _LOOKUP_H_
#define _LOOKUP_H_

#include <QString>
#include <qmap.h>

#include "lexer.h"
#include <hashedstring.h>

/**
* @short Fast keyword lookup.
*/
class Lookup
{
public:
    typedef QHash<HashedString,Type> KeywordMap;
    /**
    * Find an entry in the table, and return its value
    */
    static int find(const HashedString& s);

    static const KeywordMap& keywords();
};

#endif
