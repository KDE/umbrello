/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PHP_PARSESESSION_H
#define PHP_PARSESESSION_H

#include <QtCore/QString>
#include <language/editor/simplecursor.h>
#include "phpparser.h"
#include "parserexport.h"

namespace KDevPG
{
class MemoryPool;
}
namespace KDevelop
{
class SimpleCursor;
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
