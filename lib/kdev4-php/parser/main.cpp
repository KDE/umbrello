/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
 *   Copyright 2009 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
