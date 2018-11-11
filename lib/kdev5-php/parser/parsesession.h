/*****************************************************************************
 * Copyright (c) 2007 Piyush verma <piyush.verma@gmail.com>                  *
 * Copyright (c) 2008 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#ifndef PHP_PARSESESSION_H
#define PHP_PARSESESSION_H

#include <QString>
#include "phpparser.h"
#include "parserexport.h"

namespace KDevPG
{
class MemoryPool;
}
namespace Php
{
class TokenStream;
struct StartAst;

typedef QPair<KDevelop::DUContextPointer, KDevelop::RangeInRevision> SimpleUse;

class KDEVPHPPARSER_EXPORT ParseSession
{
public:
    ParseSession();
    ~ParseSession();

    void setContents(const QString& contents);
    void setCurrentDocument(const KDevelop::IndexedString& filename);
    KDevelop::IndexedString currentDocument() const;
    bool readFile(const QString& filename, const char* charset = 0);
    void setDebug(bool);
    TokenStream* tokenStream() const;
    QString contents() const;

    bool parse(Php::StartAst**);
    Parser* createParser(int initialState = Parser::HtmlState);

    QString symbol(qint64 token) const;
    QString symbol(AstNode* node) const;

    /**
     * Return the DocBlock before this token, if any
     */
    QString docComment(qint64 token) const;

    /**
     * Return the position (\a line%, \a column%) of the \a offset in the file.
     *
     * \note the line starts from 0.
     */
    KDevelop::CursorInRevision positionAt(qint64 offset) const;

    QList<KDevelop::ProblemPointer> problems();

    /// @TODO implement this
    void mapAstUse(AstNode* node, const SimpleUse& use)
    {
        Q_UNUSED(node);
        Q_UNUSED(use);
    }

private:
    QString m_contents;
    bool m_debug;
    KDevelop::IndexedString m_currentDocument;
    KDevPG::MemoryPool* m_pool;
    TokenStream* m_tokenStream;
    QList<KDevelop::ProblemPointer> m_problems;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
