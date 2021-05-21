/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ERRORS_H
#define ERRORS_H

#include <QString>


struct Error {
    int code;
    int level;
    QString text;

    Error(int c, int l, const QString& s)
        : code(c), level(l), text(s)
    {}
};

class Errors
{
public:
    static Error& _InternalError();
    static Error& _SyntaxError();
    static Error& _ParseError();
};

#define InternalError _InternalError()
#define SyntaxError _SyntaxError()
#define ParseError _ParseError()


#endif
