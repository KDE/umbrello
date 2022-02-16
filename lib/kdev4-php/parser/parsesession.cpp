/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parsesession.h"

#include "phpast.h"

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-token-stream.h"

#include <QFile>
#include <QTextCodec>
#include <KLocalizedString>

namespace Php
{

ParseSession::ParseSession()
        :   m_debug(false),
        m_pool(new KDevPG::MemoryPool()),
        m_tokenStream(new TokenStream())
{
}
ParseSession::~ParseSession()
{
    delete m_pool;
    delete m_tokenStream;
}

QString ParseSession::contents() const
{
    return m_contents;
}

void ParseSession::setContents(const QString& contents)
{
    m_contents = contents;
}

void ParseSession::setCurrentDocument(const KDevelop::IndexedString& filename)
{
    m_currentDocument = filename;
}

KDevelop::IndexedString ParseSession::currentDocument() const
{
    return m_currentDocument;
}

bool ParseSession::readFile(const QString& filename, const char* codec)
{
    m_currentDocument = KDevelop::IndexedString(filename);

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KDevelop::ProblemPointer p(new KDevelop::Problem());
        p->setSource(KDevelop::ProblemData::Disk);
        p->setDescription(i18n("Could not open file '%1'", filename));
        switch (f.error()) {
        case QFile::ReadError:
            p->setExplanation(i18n("File could not be read from."));
            break;
        case QFile::OpenError:
            p->setExplanation(i18n("File could not be opened."));
            break;
        case QFile::PermissionsError:
            p->setExplanation(i18n("File permissions prevent opening for read."));
            break;
        default:
            break;
        }
        p->setFinalLocation(KDevelop::DocumentRange(m_currentDocument, KDevelop::SimpleRange(0, 0, 0, 0)));
        m_problems << p;
        kWarning() << "Could not open file" << filename;
        return false;
    }
    QTextStream s(&f);
    if (codec)
        s.setCodec(QTextCodec::codecForName(codec));
    m_contents = s.readAll();
    return true;
}

void ParseSession::setDebug(bool debug)
{
    m_debug = debug;
}

TokenStream* ParseSession::tokenStream() const
{
    return m_tokenStream;
}


Parser* ParseSession::createParser(int initialState)
{
    Parser* parser = new Parser;
    parser->setTokenStream(m_tokenStream);
    parser->setMemoryPool(m_pool);
    parser->setDebug(m_debug);
    parser->setCurrentDocument(m_currentDocument);

    parser->tokenize(m_contents, initialState);
    return parser;
}

bool ParseSession::parse(Php::StartAst** ast)
{
    Parser* parser = createParser();
    StartAst* phpAst;
    bool matched = parser->parseStart(&phpAst);
    if (matched) {
        kDebug() << "Successfully parsed";
        *ast = phpAst;
    } else {
        *ast = 0;
        parser->expectedSymbol(AstNode::StartKind, "start");
        kDebug() << "Couldn't parse content";
    }
    m_problems << parser->problems();
    delete parser;
    return matched;
}

KDevelop::CursorInRevision ParseSession::positionAt(qint64 offset) const
{
    qint64 line, column;
    m_tokenStream->locationTable()->positionAt(offset, &line, &column);
    return KDevelop::CursorInRevision(line, column);
}

QString ParseSession::symbol(qint64 token) const
{
    const TokenStream::Token& tok = m_tokenStream->at(token);
    return m_contents.mid(tok.begin, tok.end - tok.begin + 1);
}

QString ParseSession::symbol(AstNode* node) const
{
    const TokenStream::Token& startTok = m_tokenStream->at(node->startToken);
    const TokenStream::Token& endTok = m_tokenStream->at(node->endToken);
    return m_contents.mid(startTok.begin, endTok.end - startTok.begin + 1);
}

QString ParseSession::docComment(qint64 token) const
{
    const TokenStream::Token& tok = m_tokenStream->at(token);
    if (!tok.docCommentEnd) return QString();
    return m_contents.mid(tok.docCommentBegin, tok.docCommentEnd - tok.docCommentBegin + 1);
}

QList<KDevelop::ProblemPointer> ParseSession::problems()
{
    return m_problems;
}

}
