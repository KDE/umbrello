/* This file is part of KDevelop
    Copyright (C) 2005 Tobias Erbsland <te@profzone.ch>
    Copyright (C) 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, see
    <http://www.gnu.org/licenses/>.
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
