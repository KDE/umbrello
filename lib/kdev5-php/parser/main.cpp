/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parsesession.h"
#include "phplexer.h"
#include "phpparser.h"
#include "phpdebugvisitor.h"
#include "phpast.h"
#include "tokenstream.h"
#include "phptokentext.h"
#include <language/util/debuglanguageparserhelper.h>

using namespace Php;

typedef KDevelopUtils::DebugLanguageParserHelper<ParseSession, TokenStream, Parser::Token,
                                            Lexer, StartAst, DebugVisitor, tokenText> PhpParser;

int main(int argc, char* argv[])
{
    KAboutData aboutData( QStringLiteral("php-parser"), i18n( "php-parser" ),
                          QStringLiteral("1"), i18n("KDevelop PHP parser debugging utility"), KAboutLicense::GPL,
                          i18n( "(c) 2008 Niko Sams, 2009 Milian Wolff" ), {}, QStringLiteral("https://www.kdevelop.org") );

    return KDevelopUtils::initAndRunParser<PhpParser>(aboutData, argc, argv);
}
