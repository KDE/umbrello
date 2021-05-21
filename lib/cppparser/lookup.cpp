/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2005 Tobias Erbsland <te@profzone.ch>
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lookup.h"

int Lookup::find(const HashedString& s)
{
    KeywordMap::const_iterator it = keywords().find(s);
    if (it == keywords().end())
        return -1;
    return static_cast<int>((*it));
}

const Lookup::KeywordMap& Lookup::keywords()
{
    static KeywordMap keywords;

    if (keywords.empty()) {
#include "keywords.h"
    }

    return keywords;
}

