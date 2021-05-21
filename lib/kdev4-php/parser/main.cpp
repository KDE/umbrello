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

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

using namespace Php;

typedef KDevelopUtils::DebugLanguageParserHelper<ParseSession, TokenStream, Parser::Token,
                                            Lexer, StartAst, DebugVisitor, tokenText> PhpParser;

int main(int argc, char* argv[])
{
    KAboutData aboutData( "php-parser", 0, ki18n( "php-parser" ),
                          "1", ki18n("KDevelop PHP parser debugging utility"), KAboutData::License_GPL,
                          ki18n( "(c) 2008 Niko Sams, 2009 Milian Wolff" ), KLocalizedString(), "https://www.kdevelop.org" );

    return KDevelopUtils::initAndRunParser<PhpParser>(aboutData, argc, argv);
}
