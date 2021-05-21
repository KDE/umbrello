/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "errors.h"
#include <KLocalizedString>

Error& Errors::_InternalError()
{
    static Error *error = 0;
    if (error == 0)
        error = new Error(1, -1, i18n("Internal Error"));
    return *error;
}

Error &Errors::_SyntaxError()
{
    static Error *error = 0;
    if (error == 0)
        error = new Error(2, -1, i18n("Syntax Error before '%1'"));
    return *error;
}

Error &Errors::_ParseError()
{
    static Error *error = 0;
    if (error == 0)
        error = new Error(3, -1, i18n("Parse Error before '%1'"));
    return *error;
}
