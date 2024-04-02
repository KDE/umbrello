/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "context.h"

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <KLocalizedString>

#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/codemodel.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/unsuretype.h>
#include <language/duchain/parsingenvironment.h>
#include <language/util/includeitem.h>
#include <language/codecompletion/codecompletion.h>

#include <util/pushvalue.h>
#include <util/path.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <project/projectmodel.h>

#include "../duchain/completioncodemodel.h"
#include "../duchain/expressionparser.h"
#include "../duchain/helper.h"
#include "../duchain/declarations/variabledeclaration.h"
#include "../duchain/declarations/classmethoddeclaration.h"
#include "../duchain/types/structuretype.h"

#include "../parser/phpparser.h"
#include "../parser/phptokentext.h"

#include "includefileitem.h"
#include "codemodelitem.h"
#include "completiondebug.h"
#include "helpers.h"
#include "implementationitem.h"
#include "keyworditem.h"

#include <KIO/Global>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())

#define ifDebug(x)

using namespace KDevelop;

namespace Php
{

typedef QList<Parser::TokenType> TokenList;

/**
 * Utility class which makes it easier to access the relevant parts
 * of the token stream for code completion.
 *
 * TODO: This class should be reviewed imo - I just hacked it together, quick'n'dirty
 */
class TokenAccess {
public:
    /// Setup the token stream from the input code
    TokenAccess(const QString &code)
        : m_code(code)
    {

        Lexer lexer(&m_stream, code);
        int token;
        while ((token = lexer.nextTokenKind())) {
            Parser::Token &t = m_stream.push();
            t.begin = lexer.tokenBegin();
            t.end = lexer.tokenEnd();
            t.kind = token;
        }
        // move to last token
        m_pos = m_stream.size() - 1;
    }

    /// returns Token_INVALID if the position is invalid
    /// else returns the type of the current token
    Parser::TokenType type() const {
        if ( m_pos == -1 ) {
            return Parser::Token_INVALID;
        } else {
            return (Parser::TokenType) m_stream.at(m_pos).kind;
        }
    }

    /// convenience comparison to a tokentype
    bool operator==(const Parser::TokenType& other) const {
        return other == type();
    }

    /// move to previous token
    void pop() {
        if ( m_pos >= 0 ) {
            --m_pos;
        }
    }

    /// move relative to current token
    /// NOTE: make sure you honor the boundaries.
    void moveTo(const qint64 &relPos) {
        m_pos += relPos;
        Q_ASSERT(m_pos > 0);
        Q_ASSERT(m_pos < m_stream.size());
    }

    /// get type of token relative to current position
    /// returns Token_INVALID if the position goes out of the boundaries
    int typeAt(const qint64 &relPos) const {
        const qint64 pos = m_pos + relPos;
        if ( pos >= 0 && pos < m_stream.size() ) {
            return m_stream.at(pos).kind;
        } else {
            return Parser::Token_INVALID;
        }
    }

    /// Get string for token at a given position relative to the current one.
    /// NOTE: Make sure you honor the boundaries.
    QString stringAt(const qint64 &relPos) const {
        Parser::Token token = at(relPos);
        return m_code.mid(token.begin, token.end - token.begin + 1);
    }

    /// check whether the current token is prepended by the list of tokens
    /// @return -1 when not prepended by the list, else the relative index-position
    qint64 prependedBy(const TokenList &list, bool skipWhitespace = false ) const {
        // this would be useless, hence forbid it
        Q_ASSERT ( !list.isEmpty() );

        if ( m_pos < list.count() - 1 ) {
            // not enough tokens
            return -1;
        } else {
            uint pos = 1;
            Q_FOREACH( Parser::TokenType type, list ) {
                if ( skipWhitespace && m_stream.at( m_pos - pos).kind == Parser::Token_WHITESPACE ) {
                    ++pos;
                }
                if ( m_stream.at( m_pos - pos).kind == type ) {
                    ++pos;
                    continue;
                } else {
                    return -1;
                }
            }
            return pos;
        }
    }

    /// Get the token relative to the current one.
    /// NOTE: Make sure you honor the boundaries.
    Parser::Token at(const qint64 &relPos) const {
        const qint64 pos = m_pos + relPos;
        Q_ASSERT(pos >= 0);
        Q_ASSERT(pos < m_stream.size());
        return m_stream.at(pos);
    }

private:
    const QString m_code;
    TokenStream m_stream;
    qint64 m_pos;
};

/**
 * Pops all tokens from the @p lastToken and stops at the LPAREN.
 */
void removeOtherArguments(TokenAccess &lastToken)
{
    Q_ASSERT(lastToken.type() == Parser::Token_COMMA);

    // remove all other arguments
    int openLParen = 0;
    do {
        lastToken.pop();
        if ( lastToken.type() == Parser::Token_RPAREN ) {
            ++openLParen;
        } else if ( lastToken.type() == Parser::Token_LPAREN ) {
            if ( openLParen == 0 ) {
                return;
            } else {
                --openLParen;
            }
        }
    } while ( lastToken.type() != Parser::Token_INVALID );
}

/**
 * if token at @p pos is whitespace, decrease pos by one.
 */
inline void skipWhiteSpace(const TokenAccess &lastToken, qint64 &pos)
{
    if ( lastToken.typeAt(pos) == Parser::Token_WHITESPACE ) {
        --pos;
    }
}

/// add keyword to list of completion items
#define ADD_KEYWORD(x) items << CompletionTreeItemPointer( new KeywordItem( QStringLiteral(x), Php::CodeCompletionContext::Ptr(this) ) )
#define ADD_KEYWORD2(x, y) items << CompletionTreeItemPointer( new KeywordItem( QStringLiteral(x), Php::CodeCompletionContext::Ptr(this), QStringLiteral(y) ) )

int completionRecursionDepth = 0;

CodeCompletionContext::CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const QString& followingText, const KDevelop::CursorInRevision& position, int depth)
        : KDevelop::CodeCompletionContext(context, text, position, depth)
        , m_memberAccessOperation(NoMemberAccess), m_parentAccess(false), m_isFileCompletionAfterDirname(false)
{
    // use other ctor for parents
    Q_ASSERT(depth == 0);

    ifDebug(qCDebug(COMPLETION) << "non-processed text: " + text;)

    if ( context->type() == DUContext::Class || context->type() == DUContext::Function || context->type() == DUContext::Other
        || context->type() == DUContext::Namespace )
    {
        if ( !m_parentContext && !m_text.startsWith(QLatin1String("<?php ")) ) {
            ifDebug(qCDebug(COMPLETION) << "added start tag: " + m_text;)
            m_text.prepend("<?php ");
        }
    }

    m_valid = !m_text.isEmpty();

    if (!m_valid) {
        qCDebug(COMPLETION) << "empty completion text";
        return;
    }

    TokenAccess lastToken(m_text);
//     ifDebug(qCDebug(COMPLETION) << "clearing completion text");)
//     m_text.clear();

    /// even when we skip to some more meaning ful token, this will
    /// always be the end position of the last token
    const qint64 lastTokenEnd = lastToken.at(0).end + 1;

    bool lastWasWhitespace = lastToken == Parser::Token_WHITESPACE;
    if ( lastWasWhitespace ) {
        ifDebug(qCDebug(COMPLETION) << "skipping whitespace token";)
        lastToken.pop();
    }

    // when the text after the current token starts with /* we are inside
    // a multi line comment => don't offer completion
    if ( m_text.mid( lastTokenEnd, 2 ) == QLatin1String("/*") ) {
        ifDebug(qCDebug(COMPLETION) << "no completion in comments");
        m_valid = false;
        return;
    }

    ifDebug(qCDebug(COMPLETION) << tokenText(lastToken.type());)

    ///TODO: REFACTOR: push some stuff into its own methods
    ///                and call them from inside the big switch.
    ///                Then we can forget about having additional checks
    ///                beforehand and can handle it all in one place.

    // The following tokens require a whitespace after them for code-completion:
    if ( !lastWasWhitespace ) {
        switch ( lastToken.type() ) {
            case Parser::Token_EXTENDS:
            case Parser::Token_IMPLEMENTS:
            case Parser::Token_NEW:
            case Parser::Token_THROW:
                ifDebug(qCDebug(COMPLETION) << "need whitespace after token for completion";)
                m_valid = false;
                return;
            default:
                break;
        }
    }

    ifDebug(qCDebug(COMPLETION) << tokenText(lastToken.type());)

    switch ( lastToken.type() ) {
        case Parser::Token_COMMENT:
            // don't offer code completion in comments, i.e. single line comments that don't end on \n
            // multi-line comments are handled above
            if ( !lastWasWhitespace && !lastToken.stringAt(0).endsWith('\n')
                    && !lastToken.stringAt(0).startsWith(QLatin1String("/*")) ) {
                ifDebug(qCDebug(COMPLETION) << "no completion in comments";)
                m_valid = false;
            }
            break;
        case Parser::Token_EXTENDS:
            if ( lastToken.prependedBy(TokenList() << Parser::Token_WHITESPACE << Parser::Token_STRING
                                                   << Parser::Token_WHITESPACE << Parser::Token_CLASS) != -1 ) {
                m_memberAccessOperation = ClassExtendsChoose;
                forbidIdentifier(lastToken.stringAt(-2));
            } else if ( lastToken.prependedBy(TokenList() << Parser::Token_WHITESPACE << Parser::Token_STRING
                                                   << Parser::Token_WHITESPACE << Parser::Token_INTERFACE) != -1 ) {
                m_memberAccessOperation = InterfaceChoose;
                forbidIdentifier(lastToken.stringAt(-2));
            } else {
                ifDebug(qCDebug(COMPLETION) << "token prepended by bad tokens, don't do completion";)
                m_valid = false;
            }
            break;
        case Parser::Token_IMPLEMENTS:
            if ( lastToken.prependedBy(TokenList() << Parser::Token_WHITESPACE << Parser::Token_STRING
                                                   << Parser::Token_WHITESPACE << Parser::Token_CLASS) != -1 ) {
                m_memberAccessOperation = InterfaceChoose;
                forbidIdentifier(lastToken.stringAt(-2));
            } else {
                ifDebug(qCDebug(COMPLETION) << "token prepended by bad tokens, don't do completion";)
                m_valid = false;
            }
            break;
        case Parser::Token_COMMA:
            {
            // check if we are in the list after Token_IMPLEMENTS:
            qint64 relPos = -1;
            QList<qint64> identifierPositions;
            while ( true ) {
                skipWhiteSpace(lastToken, relPos);
                if ( lastToken.typeAt(relPos) == Parser::Token_STRING ) {
                    identifierPositions << relPos;
                    --relPos;
                    skipWhiteSpace(lastToken, relPos);
                            // interfaces may extend more than one interface
                    if ( ( lastToken.typeAt(relPos) == Parser::Token_EXTENDS &&
                            lastToken.typeAt(relPos - 1) == Parser::Token_WHITESPACE &&
                            lastToken.typeAt(relPos - 2) == Parser::Token_STRING &&
                            lastToken.typeAt(relPos - 3) == Parser::Token_WHITESPACE &&
                            lastToken.typeAt(relPos - 4) == Parser::Token_INTERFACE )
                        || // classes may implement more than one interface
                         ( lastToken.typeAt(relPos) == Parser::Token_IMPLEMENTS &&
                            lastToken.typeAt(relPos - 1) == Parser::Token_WHITESPACE &&
                            lastToken.typeAt(relPos - 2) == Parser::Token_STRING &&
                            lastToken.typeAt(relPos - 3) == Parser::Token_WHITESPACE &&
                            lastToken.typeAt(relPos - 4) == Parser::Token_CLASS ) )
                    {
                        identifierPositions << (relPos - 2);
                        m_memberAccessOperation = InterfaceChoose;
                        break;
                    } else if ( lastToken.typeAt(relPos) == Parser::Token_COMMA ) {
                        // skip to next entry
                        --relPos;
                        continue;
                    }
                } else {
                    break;
                }
            }
            if ( m_memberAccessOperation == InterfaceChoose ) {
                ifDebug(qCDebug(COMPLETION) << "in implementation list";)
                m_memberAccessOperation = InterfaceChoose;
                Q_FOREACH( qint64 pos, identifierPositions ) {
                    forbidIdentifier(lastToken.stringAt(pos));
                }
            } else {
                // else do function call completion
                m_memberAccessOperation = FunctionCallAccess;

                ///TODO: global, static etc. enumerations.
                removeOtherArguments(lastToken);

                if ( lastToken.type() == Parser::Token_INVALID ) {
                    m_valid = false;
                }
            }
            }
            break;
        case Parser::Token_OPEN_TAG:
            // don't do completion if no whitespace is given and there is some text following,
            // esp. for stuff like <?php <?ph <?p
            if ( !lastWasWhitespace && !followingText.isEmpty() ) {
                ifDebug(qCDebug(COMPLETION) << "no completion because <? is followed by" + followingText;)
                m_valid = false;
            } else {
                // else just do normal completion
                m_memberAccessOperation = NoMemberAccess;
            }
            break;
        case Parser::Token_OBJECT_OPERATOR:
            m_memberAccessOperation = MemberAccess;
            lastToken.pop();
            break;
        case Parser::Token_PAAMAYIM_NEKUDOTAYIM:
            m_memberAccessOperation = StaticMemberAccess;
            lastToken.pop();
            break;
        case Parser::Token_LPAREN:
            {
            qint64 pos = -1;
            skipWhiteSpace(lastToken, pos);
            if ( lastToken.typeAt(pos) == Parser::Token_CATCH ) {
                m_memberAccessOperation = ExceptionChoose;
            } else if ( lastToken.typeAt(pos) == Parser::Token_ARRAY ) {
                m_memberAccessOperation = NoMemberAccess;
                ifDebug(qCDebug(COMPLETION) << "NoMemberAccess";)
                ifDebug(qCDebug(COMPLETION) << "returning early";)
                return;
            } else {
                m_memberAccessOperation = FunctionCallAccess;
            }
            }
            break;
        case Parser::Token_NEW:
            if ( lastToken.prependedBy(TokenList() << Parser::Token_WHITESPACE << Parser::Token_THROW) != -1 ) {
                m_memberAccessOperation = ExceptionChoose;
            } else {
                m_memberAccessOperation = NewClassChoose;
            }
            break;
        case Parser::Token_THROW:
            m_memberAccessOperation = ExceptionInstanceChoose;
            break;
        case Parser::Token_CONSTANT_ENCAPSED_STRING:
            {
                // support something like `include dirname(__FILE__) . "/...`
                ///TODO: include __DIR__ . "/ (php 5.3)
                bool isAfterDirname = false;
                //NOTE: prependedBy will return -1 on failure, this is what we need in these cases
                //      on success it will return a positive number, we'll need to switch it's sign in that case
                qint64 relPos = lastToken.prependedBy(TokenList() << Parser::Token_CONCAT << Parser::Token_RPAREN << Parser::Token_FILE
                                                   << Parser::Token_LPAREN << Parser::Token_STRING, true);
                if ( relPos != -1 ) {
                    // switch sign
                    relPos = -relPos;
                    if ( lastToken.stringAt(relPos + 1).compare(QLatin1String("dirname"), Qt::CaseInsensitive) == 0 ) {
                        isAfterDirname = true;
                    }
                }
                skipWhiteSpace(lastToken, relPos);
                if ( lastToken.typeAt(relPos) == Parser::Token_LPAREN ) {
                    --relPos;
                }
                skipWhiteSpace(lastToken, relPos);
                switch ( lastToken.typeAt(relPos) ) {
                    case Parser::Token_REQUIRE:
                    case Parser::Token_REQUIRE_ONCE:
                    case Parser::Token_INCLUDE:
                    case Parser::Token_INCLUDE_ONCE:
                        m_memberAccessOperation = FileChoose;
                        m_expression = m_text.mid( lastToken.at(0).begin + 1 ).append(followingText).trimmed();
                        m_isFileCompletionAfterDirname = isAfterDirname;
                        break;
                    default:
                        if ( m_text.at( lastToken.at(0).begin ).unicode() == '"' ) {
                            ///TODO: only offer variable completion
                            m_valid = false;
                        } else {
                            // in or after constant strings ('...') don't offer completion at all
                            m_valid = false;
                        }
                        break;
                }
                break;
            }
            break;
        case Parser::Token_INSTANCEOF:
            m_memberAccessOperation = InstanceOfChoose;
            break;
        case Parser::Token_AND_ASSIGN:
        case Parser::Token_ARRAY_CAST:
        case Parser::Token_ASSIGN:
        case Parser::Token_AT:
        case Parser::Token_BANG:
        case Parser::Token_BIT_AND:
        case Parser::Token_BIT_OR:
        case Parser::Token_BIT_XOR:
        case Parser::Token_BOOLEAN_AND:
        case Parser::Token_BOOLEAN_OR:
        case Parser::Token_BOOL_CAST:
        case Parser::Token_COLON:
        case Parser::Token_CONCAT:
        case Parser::Token_CONCAT_ASSIGN:
        case Parser::Token_CURLY_OPEN:
        case Parser::Token_DEC:
        case Parser::Token_DIV:
        case Parser::Token_DIV_ASSIGN:
        case Parser::Token_DOC_COMMENT:
        case Parser::Token_DOLLAR_OPEN_CURLY_BRACES:
        case Parser::Token_DOUBLE_ARROW:
        case Parser::Token_DOUBLE_CAST:
        case Parser::Token_DOUBLE_QUOTE:
        case Parser::Token_ECHO:
        case Parser::Token_ENCAPSED_AND_WHITESPACE:
        case Parser::Token_EXIT:
        case Parser::Token_INC:
        case Parser::Token_INT_CAST:
        case Parser::Token_IS_EQUAL:
        case Parser::Token_IS_GREATER:
        case Parser::Token_IS_GREATER_OR_EQUAL:
        case Parser::Token_IS_IDENTICAL:
        case Parser::Token_IS_NOT_EQUAL:
        case Parser::Token_IS_NOT_IDENTICAL:
        case Parser::Token_IS_SMALLER:
        case Parser::Token_IS_SMALLER_OR_EQUAL:
        case Parser::Token_LBRACE:
        case Parser::Token_LBRACKET:
        case Parser::Token_LOGICAL_AND:
        case Parser::Token_LOGICAL_OR:
        case Parser::Token_LOGICAL_XOR:
        case Parser::Token_MINUS:
        case Parser::Token_MINUS_ASSIGN:
        case Parser::Token_MOD:
        case Parser::Token_MOD_ASSIGN:
        case Parser::Token_MUL:
        case Parser::Token_MUL_ASSIGN:
        case Parser::Token_OBJECT_CAST:
        case Parser::Token_OPEN_TAG_WITH_ECHO:
        case Parser::Token_OR_ASSIGN:
        case Parser::Token_PLUS:
        case Parser::Token_PLUS_ASSIGN:
        case Parser::Token_PRINT:
        case Parser::Token_QUESTION:
        case Parser::Token_RBRACE:
        case Parser::Token_RETURN:
        case Parser::Token_SEMICOLON:
        case Parser::Token_SL:
        case Parser::Token_SL_ASSIGN:
        case Parser::Token_SR:
        case Parser::Token_SR_ASSIGN:
        case Parser::Token_START_HEREDOC:
        case Parser::Token_START_NOWDOC:
        case Parser::Token_STRING:
        case Parser::Token_STRING_CAST:
        case Parser::Token_TILDE:
        case Parser::Token_UNSET_CAST:
        case Parser::Token_XOR_ASSIGN:
        case Parser::Token_EXP:
        case Parser::Token_EXP_ASSIGN:
            // normal completion is valid
            if ( duContext() && duContext()->type() == DUContext::Class ) {
                // when we are inside a class context, give overloadable members as completion
                m_memberAccessOperation = ClassMemberChoose;
            } else {
                m_memberAccessOperation = NoMemberAccess;
            }
            break;
        case Parser::Token_ABSTRACT:
        case Parser::Token_CONST:
        case Parser::Token_FINAL:
        case Parser::Token_PUBLIC:
        case Parser::Token_PRIVATE:
        case Parser::Token_PROTECTED:
        case Parser::Token_STATIC:
        case Parser::Token_VAR:
            if ( duContext() && duContext()->type() == DUContext::Class ) {
                // when we are inside a class context, give overloadable members as completion
                m_memberAccessOperation = ClassMemberChoose;
            } else {
                m_valid = false;
            }
            break;
        case Parser::Token_NAMESPACE:
        case Parser::Token_BACKSLASH:
        {
            QString identifier;
            qint64 relPos = 0;
            while (lastToken.typeAt(relPos) == Parser::Token_STRING || lastToken.typeAt(relPos) == Parser::Token_BACKSLASH) {
                if (lastToken.typeAt(relPos) == Parser::Token_BACKSLASH) {
                    identifier.prepend("::");
                } else {
                    identifier.prepend(lastToken.stringAt(relPos));
                }
                --relPos;
            }
            if ( lastToken.typeAt(relPos) == Parser::Token_NAMESPACE ) {
                m_memberAccessOperation = NamespaceChoose;
            } else {
                m_memberAccessOperation = BackslashAccess;
            }
            m_namespace = QualifiedIdentifier(identifier);
            break;
        }
        case Parser::Token_ARRAY:
        case Parser::Token_AS:
        case Parser::Token_BACKTICK:
        case Parser::Token_BREAK:
        case Parser::Token_CALLABLE:
        case Parser::Token_CASE:
        case Parser::Token_CATCH:
        case Parser::Token_CLASS:
        case Parser::Token_CLASS_C:
        case Parser::Token_CLONE:
        case Parser::Token_CLOSE_TAG:
        case Parser::Token_CONTINUE:
        case Parser::Token_DECLARE:
        case Parser::Token_DEFAULT:
        case Parser::Token_DNUMBER:
        case Parser::Token_DO:
        case Parser::Token_DOLLAR:
        case Parser::Token_ELSE:
        case Parser::Token_ELSEIF:
        case Parser::Token_EMPTY:
        case Parser::Token_ENDDECLARE:
        case Parser::Token_ENDFOR:
        case Parser::Token_ENDFOREACH:
        case Parser::Token_ENDIF:
        case Parser::Token_ENDSWITCH:
        case Parser::Token_ENDWHILE:
        case Parser::Token_END_HEREDOC:
        case Parser::Token_END_NOWDOC:
        case Parser::Token_EOF:
        case Parser::Token_EVAL:
        case Parser::Token_FILE:
        case Parser::Token_FINALLY:
        case Parser::Token_FOR:
        case Parser::Token_FOREACH:
        case Parser::Token_FUNCTION:
        case Parser::Token_FUNC_C:
        case Parser::Token_GLOBAL:
        case Parser::Token_HALT_COMPILER:
        case Parser::Token_IF:
        case Parser::Token_INCLUDE:
        case Parser::Token_INCLUDE_ONCE:
        case Parser::Token_INLINE_HTML:
        case Parser::Token_INSTEADOF:
        case Parser::Token_INTERFACE:
        case Parser::Token_INVALID:
        case Parser::Token_ISSET:
        case Parser::Token_LINE:
        case Parser::Token_LIST:
        case Parser::Token_LNUMBER:
        case Parser::Token_METHOD_C:
        case Parser::Token_NAMESPACE_C:
        case Parser::Token_NUM_STRING:
        case Parser::Token_REQUIRE:
        case Parser::Token_REQUIRE_ONCE:
        case Parser::Token_RBRACKET:
        case Parser::Token_RPAREN:
        case Parser::Token_STRING_VARNAME:
        case Parser::Token_SWITCH:
        case Parser::Token_TRAIT:
        case Parser::Token_TRY:
        case Parser::Token_UNSET:
        case Parser::Token_USE:
        case Parser::Token_VARIABLE:
        case Parser::Token_WHILE:
        case Parser::Token_WHITESPACE:
        /// TODO: code completion after goto
        case Parser::Token_GOTO:
        case Parser::TokenTypeSize:
            ifDebug(qCDebug(COMPLETION) << "no completion after this token";)
            m_valid = false;
            break;
    }

    ifDebug(
        switch ( m_memberAccessOperation ) {
            case FileChoose:
                qCDebug(COMPLETION) << "FileChoose";
                break;
            case ExceptionInstanceChoose:
                qCDebug(COMPLETION) << "ExceptionInstanceChoose";
                break;
            case ExceptionChoose:
                qCDebug(COMPLETION) << "ExceptionChoose";
                break;
            case ClassMemberChoose:
                qCDebug(COMPLETION) << "ClassMemberChoose";
                break;
            case NoMemberAccess:
                qCDebug(COMPLETION) << "NoMemberAccess";
                break;
            case NewClassChoose:
                qCDebug(COMPLETION) << "NewClassChoose";
                break;
            case FunctionCallAccess:
                qCDebug(COMPLETION) << "FunctionCallAccess";
                break;
            case InterfaceChoose:
                qCDebug(COMPLETION) << "InterfaceChoose";
                break;
            case ClassExtendsChoose:
                qCDebug(COMPLETION) << "ClassExtendsChoose";
                break;
            case MemberAccess:
                qCDebug(COMPLETION) << "MemberAccess";
                break;
            case StaticMemberAccess:
                qCDebug(COMPLETION) << "StaticMemberAccess";
                break;
            case InstanceOfChoose:
                qCDebug(COMPLETION) << "InstanceOfChoose";
                break;
            case NamespaceChoose:
                qCDebug(COMPLETION) << "NamespaceChoose";
                break;
            case BackslashAccess:
                qCDebug(COMPLETION) << "BackslashAccess";
                break;
        }
    )

    ifDebug(qCDebug(COMPLETION) << tokenText(lastToken.type());)

    // if it's not valid, we should return early
    if ( !m_valid ) {
        ifDebug(qCDebug(COMPLETION) << "invalid completion";)
        return;
    }

    // trim the text to the end position of the current token
    m_text = m_text.left(lastToken.at(0).end + 1).trimmed();
    ifDebug(qCDebug(COMPLETION) << "trimmed text: " << m_text;)

    // check whether we need the expression or have everything we need and can return early
    switch ( m_memberAccessOperation ) {
        // these access operations don't need the previous expression evaluated
        case FileChoose:
        case ClassMemberChoose:
        case InterfaceChoose:
        case NewClassChoose:
        case ExceptionChoose:
        case ExceptionInstanceChoose:
        case ClassExtendsChoose:
        case NoMemberAccess:
        case InstanceOfChoose:
        case NamespaceChoose:
        case BackslashAccess:
            ifDebug(qCDebug(COMPLETION) << "returning early";)
            return;
        case FunctionCallAccess:
            m_memberAccessOperation = NoMemberAccess;
            Q_ASSERT(lastToken.type() == Parser::Token_LPAREN);
            if ( lastToken.prependedBy(TokenList() << Parser::Token_STRING, true) == -1 &&
                 lastToken.prependedBy(TokenList() << Parser::Token_VARIABLE, true) == -1 )
            {
                // handle for, foreach, while, etc.
                ifDebug(qCDebug(COMPLETION) << "NoMemberAccess (no function call)";)
            } else {
                //The first context should never be a function-call context,
                //so make this a NoMemberAccess context and the parent a function-call context.
                ifDebug(qCDebug(COMPLETION) << "NoMemberAccess (creating parentContext for function call)";)
                m_parentContext = new CodeCompletionContext(m_duContext, m_position, lastToken, depth + 1);
            }
            return;
        case MemberAccess:
        case StaticMemberAccess:
            // these types need the expression evaluated
            break;
    }

    evaluateExpression(lastToken);
}

CodeCompletionContext::CodeCompletionContext(KDevelop::DUContextPointer context, const KDevelop::CursorInRevision& position,
                                             TokenAccess& lastToken,  int depth)
        : KDevelop::CodeCompletionContext(context, QString(), position, depth)
        , m_memberAccessOperation(NoMemberAccess), m_parentAccess(false), m_isFileCompletionAfterDirname(false)
{
    switch ( lastToken.type() ) {
        case Parser::Token_LPAREN:
            m_memberAccessOperation = FunctionCallAccess;
            break;
        default:
            qCDebug(COMPLETION) << "unhandled token type for parent context" << tokenText(lastToken.typeAt(0));
            Q_ASSERT(false);
            m_valid = false;
            return;
    }

    evaluateExpression(lastToken);
}

void CodeCompletionContext::evaluateExpression(TokenAccess& lastToken)
{
    /// token pos
    qint64 startPos = 0;
    int openLParen = 0;

    if ( m_memberAccessOperation == FunctionCallAccess ) {
        Q_ASSERT(lastToken.type() == Parser::Token_LPAREN);
        // check ctor call
        qint64 pos = lastToken.prependedBy(TokenList() << Parser::Token_STRING << Parser::Token_NEW, true);
        if ( pos != -1 ) {
            startPos = -pos;
            ifDebug(qCDebug(COMPLETION) << "ctor call";)
        } else {
            // simple function call, get it's expression
            startPos = -1;
            ifDebug(qCDebug(COMPLETION) << "simple function call";)
        }
    }

    static const QList<int> defaultStopTokens = QList<int>()
            << Parser::Token_SEMICOLON << Parser::Token_INVALID << Parser::Token_OPEN_TAG
            << Parser::Token_OPEN_TAG_WITH_ECHO << Parser::Token_LBRACE << Parser::Token_RBRACE
            << Parser::Token_IF << Parser::Token_WHILE << Parser::Token_FOR << Parser::Token_FOREACH
            << Parser::Token_SWITCH << Parser::Token_ELSEIF;


    // find expression start
    while ( !defaultStopTokens.contains(lastToken.typeAt(startPos)) &&
            (m_memberAccessOperation == FunctionCallAccess || lastToken.typeAt(startPos) != Parser::Token_COMMA) )
    {
        if ( lastToken.typeAt(startPos) == Parser::Token_LPAREN ) {
            ++openLParen;
            if ( openLParen > 0 ) {
                break;
            }
        } else if ( lastToken.typeAt(startPos) == Parser::Token_RPAREN ) {
            --openLParen;
        }
        --startPos;
    }

    if ( openLParen < 0 ) {
        ifDebug(qCDebug(COMPLETION) << "too many closed parenthesis";)
        m_valid = false;
        return;
    }

    // we actually incorporate the not-wanted token, hence move forward
    ++startPos;

    if ( lastToken.typeAt(startPos) == Parser::Token_WHITESPACE ) {
        ++startPos;
    }

    if ( lastToken.typeAt(startPos) == Parser::Token_RETURN ) {
        ///TODO: match against function return type
        ++startPos;

        if ( lastToken.typeAt(startPos) == Parser::Token_WHITESPACE ) {
            ++startPos;
        }
    }

    if ( m_memberAccessOperation == StaticMemberAccess ) {
        if ( lastToken.typeAt(startPos) != Parser::Token_STRING ) {
            ifDebug(qCDebug(COMPLETION) << "unsupported token for start member access:" << tokenText(lastToken.typeAt(startPos));)
            m_valid = false;
            return;
        }

        const QString identifier(lastToken.stringAt(startPos).toLower());

        if ( identifier == QLatin1String("self") || identifier == QLatin1String("parent") || identifier == QLatin1String("static") ) {
            // self and parent are only accessible from within a member function of a class
            if (DUContext* parent = m_duContext->parentContext()) {
                LOCKDUCHAIN;
                ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(parent->owner());
                if (classDec) {
                    if (identifier == QLatin1String("parent")) {
                        FOREACH_FUNCTION(const BaseClassInstance& base, classDec->baseClasses) {
                            if (StructureType::Ptr classType = base.baseClass.type<StructureType>()) {
                                if (ClassDeclaration* baseClass = dynamic_cast<ClassDeclaration*>(classType->declaration(m_duContext->topContext()))) {
                                    if (baseClass->classType() == ClassDeclarationData::Class
                                            && baseClass->classModifier() != ClassDeclarationData::Abstract) {
                                        ifDebug(qCDebug(COMPLETION) << "correction: parent can do MemberAccess");
                                        m_parentAccess = true;
                                        m_memberAccessOperation = MemberAccess;
                                        m_expressionResult.setDeclaration(baseClass);
                                        break;
                                    }
                                }
                            }
                        }
                        if (!m_parentAccess) {
                            ifDebug(qCDebug(COMPLETION) << "class has no accessible parent class");
                            m_valid = false;
                            return;
                        }
                    } else {
                        m_expressionResult.setDeclaration(parent->owner());
                    }
                }
            }
        } else {
            QualifiedIdentifier id(identifier);

            m_expressionResult.setDeclaration(findDeclarationImportHelper(duContext(), id, ClassDeclarationType));
        }
    } else {
        // Now get the string of the expression and evaluate it
        Q_ASSERT(m_expression.isEmpty());

        for (qint64 i = startPos; i <= 0; ++i ) {
            m_expression += lastToken.stringAt(i);
        }

        m_expression = m_expression.trimmed();

        // make sure the expression is valid
        if (m_memberAccessOperation == FunctionCallAccess) {
            m_expression.append(')');
        }
        for ( int i = openLParen; i > 0; --i ) {
            m_expression.append(')');
        }

        ifDebug(qCDebug(COMPLETION) << "expression: " << m_expression;)

        if ( !m_expression.isEmpty() ) {
            ExpressionParser expressionParser;
            m_expressionResult = expressionParser.evaluateType(m_expression.toUtf8(), m_duContext, m_position);
        }

        if (m_expressionResult.type()) {
            LOCKDUCHAIN;
            ifDebug(qCDebug(COMPLETION) << "expression type: " << m_expressionResult.type()->toString();)
        } else {
            ifDebug(qCDebug(COMPLETION) << QString("expression could not be evaluated"));
            if ( m_memberAccessOperation == FunctionCallAccess ) {
                ifDebug(qCDebug(COMPLETION) << "function not found";)
                return;
            }
            m_valid = false;
            return;
        }
    }

    lastToken.moveTo(startPos);

    // Handle recursive contexts (Example: "ret = function1(param1, function2(" )
    if ( lastToken.typeAt(-1) == Parser::Token_LPAREN ||
         lastToken.typeAt(-1) == Parser::Token_COMMA ) {
        //Our expression is within a function-call. We need to find out the possible argument-types we need to match, and show an argument-hint.

        lastToken.moveTo(-1);
        if ( lastToken.type() == Parser::Token_COMMA ) {
            removeOtherArguments(lastToken);
            if ( lastToken.type() == Parser::Token_INVALID ) {
                ifDebug(qCDebug(COMPLETION) << QString("Could not find start position for parent function-call. Aborting.");)
                m_valid = false;
                return;
            }
        }

        if ( lastToken.prependedBy(TokenList() << Parser::Token_STRING, true) == -1 ) {
            // handle for, foreach, while, etc.
            ifDebug(qCDebug(COMPLETION) << "No real function call";)
            return;
        }

        ifDebug(qCDebug(COMPLETION) << QString("Recursive function-call: creating parent context"));
        m_parentContext = new CodeCompletionContext(m_duContext, m_position, lastToken, m_depth + 1);

        if (!m_parentContext->isValid()) {
            m_parentContext = 0;
            m_valid = false;
            return;
        }
    }
}

void CodeCompletionContext::forbidIdentifier(const QString& identifier)
{
    QualifiedIdentifier id(identifier.toLower());

    ClassDeclaration *dec = dynamic_cast<ClassDeclaration*>(
                                findDeclarationImportHelper(m_duContext.data(), id,
                                                            ClassDeclarationType).data()
                            );
    if (dec) {
        forbidIdentifier(dec);
    } else {
        // might be a class we are currently writing, i.e. without a proper declaration
        m_forbiddenIdentifiers << id.index();
    }
}

void CodeCompletionContext::forbidIdentifier(ClassDeclaration* klass)
{
    uint id;
    {
        LOCKDUCHAIN;
        // TODO: qualifiedIdentifier is marked as expensive - any other way
        //       we can do what we are doing here?
        // TODO: maybe we should clar the m_fobiddenIdentifiers after we got
        //       our list of items...
        id = klass->qualifiedIdentifier().index();
    }
    if (m_forbiddenIdentifiers.contains(id)) {
        // nothing to do
        return;
    }

    m_forbiddenIdentifiers << id;

    // add parents so those are excluded from the completion items as well
    if (klass->baseClassesSize() > 0) {
        FOREACH_FUNCTION(const BaseClassInstance& base, klass->baseClasses) {
            StructureType::Ptr type = base.baseClass.type<StructureType>();
            if (type.data()) {
                ClassDeclaration* parent;
                {
                    LOCKDUCHAIN;
                    parent = dynamic_cast<ClassDeclaration*>(
                                               type->declaration(m_duContext->topContext())
                                           );
                }
                if (parent) {
                    forbidIdentifier(parent);
                }
            }
        }
    }
}

CodeCompletionContext::~CodeCompletionContext()
{
}

CodeCompletionContext::MemberAccessOperation CodeCompletionContext::memberAccessOperation() const
{
    return m_memberAccessOperation;
}

ExpressionEvaluationResult CodeCompletionContext::memberAccessContainer() const
{
    return m_expressionResult;
}

CodeCompletionContext* CodeCompletionContext::parentContext()
{
    return static_cast<CodeCompletionContext*>(KDevelop::CodeCompletionContext::parentContext());
}

QList<DUContext*> CodeCompletionContext::memberAccessContainers() const
{
    QList<DUContext*> ret;
    QList<AbstractType::Ptr> types;
    AbstractType::Ptr expressionTarget = m_expressionResult.type();
    if (UnsureType::Ptr unsureType = UnsureType::Ptr::dynamicCast(m_expressionResult.type())) {
        FOREACH_FUNCTION(const IndexedType& t, unsureType->types) {
            types << t.abstractType();
        }
    } else if (ReferenceType::Ptr referencedType = ReferenceType::Ptr::dynamicCast(m_expressionResult.type()) ) {
        types << referencedType->baseType();
    } else {
        types << expressionTarget;
    }
    Q_FOREACH(const AbstractType::Ptr &type, types) {
        const IdentifiedType* idType = dynamic_cast<const IdentifiedType*>(type.data());
        Declaration* declaration = 0;
        if (idType) {
            declaration = idType->declaration(m_duContext->topContext());
        }
        DUContext* ctx = 0;
        if (declaration) {
            ctx = declaration->logicalInternalContext(m_duContext->topContext());
        }
        if (ctx) {
            ret << ctx;
        } else if (declaration) {
            //Print some debug-output
            qCDebug(COMPLETION) << "Could not get internal context from" << declaration->toString();
        } else {
            //Print some debug-output
            qCDebug(COMPLETION) << "Could not get declaration";
        }
    }
    return ret;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    //TODO: how should this be handled?
    Q_UNUSED(fullCompletion)

    /// Indexed string for 'Php', identifies environment files from this language plugin
    static const IndexedString phpLangString("Php");

    LOCKDUCHAIN;

    QList<CompletionTreeItemPointer> items;

    if (!m_duContext)
        return items;

    typedef QPair<Declaration*, int> DeclarationDepthPair;

    if ( memberAccessOperation() == FileChoose ) {
        if ( !ICore::self() ) {
            // in unit tests we can't do anything
            qCDebug(COMPLETION) << "no core found";
            return items;
        }
        // file completion
        const Path currentDocument(m_duContext->url().str());
        Path path;
        Path base;
        if ( !m_isFileCompletionAfterDirname ) {
            path = Path(currentDocument.parent(), m_expression);
            base = path;
            if ( !m_expression.isEmpty() && !m_expression.endsWith('/') ) {
                base = base.parent();
            }
        } else {
            if ( m_expression.startsWith('/') ) {
                path = Path(currentDocument.parent(), m_expression.mid(1));
            } else {
                path = currentDocument.parent();
            }
            base = path;
            if ( !m_expression.isEmpty() && !m_expression.endsWith('/') && m_expression != QLatin1String("/") ) {
                base = base.parent();
            }
        }
        QList<Path> addedPaths;
        bool addedParentDir = false;
        const QUrl baseUrl = base.toUrl();
        Q_FOREACH( ProjectBaseItem* item, ICore::self()->projectController()->projectModel()->itemsForPath(IndexedString(base.toUrl())) ) {
            if ( abort || !item->folder() ) {
                break;
            }
            auto folder = item->folder();
            Q_FOREACH( ProjectFileItem* subFile, folder->fileList() ) {
                if ( abort ) {
                    break;
                }
                if ( addedPaths.contains(subFile->path()) ) {
                    continue;
                } else {
                    addedPaths << subFile->path();
                }
                IncludeItem item;
                item.isDirectory = false;
                item.basePath = baseUrl;
                item.name = subFile->fileName();
                if ( m_isFileCompletionAfterDirname && !m_expression.startsWith('/') ) {
                    item.name.prepend('/');
                }
                items << CompletionTreeItemPointer(new IncludeFileItem(item));
            }
            Q_FOREACH( ProjectFolderItem* subFolder, folder->folderList() ) {
                if ( abort ) {
                    break;
                }
                if ( addedPaths.contains(subFolder->path()) ) {
                    continue;
                } else {
                    addedPaths << subFolder->path();
                }
                IncludeItem item;
                item.isDirectory = true;
                item.basePath = baseUrl;
                item.name = subFolder->folderName();
                if ( m_isFileCompletionAfterDirname && !m_expression.startsWith('/') ) {
                    item.name.prepend('/');
                }
                items << CompletionTreeItemPointer(new IncludeFileItem(item));
            }
            if ( !folder->parent() && !addedParentDir && m_expression.isEmpty() ) {
                // expect a parent dir
                IncludeItem item;
                item.isDirectory = true;
                item.basePath = baseUrl;
                item.name = QStringLiteral("..");
                items << CompletionTreeItemPointer(new IncludeFileItem(item));
            }
        }

        return items;
    } else if (memberAccessOperation() == ClassMemberChoose) {
        // get current class
        if (ClassDeclaration * currentClass = dynamic_cast<ClassDeclaration*>(m_duContext->owner())) {
            // whether we want to show a list of overloadable functions
            // i.e. not after we have typed one of the keywords var,const or abstract
            bool showOverloadable = true;
            // whether we want to remove static functions from the overloadable list
            // i.e. after we have typed "public function"
            bool filterStatic = false;
            // whether we want to remove non-static functions from the overloadable list
            // i.e. after we have typed "public static function"
            bool filterNonStatic = false;
            // private functions are always removed from the overloadable list
            // but when we type "protected function" only protected functions may be shown
            bool filterPublic = false;

            {
                // add typical keywords for class member definitions
                QStringList modifiers = getMethodTokens(m_text);

                // don't add keywords when "function" was already typed
                bool addKeywords = !modifiers.contains(QStringLiteral("function"));

                if (currentClass->classModifier() == ClassDeclarationData::Abstract) {
                    // abstract is only allowed in abstract classes
                    if (modifiers.contains(QStringLiteral("abstract"))) {
                        // don't show overloadable functions when we are defining an abstract function
                        showOverloadable = false;
                    } else if (addKeywords) {
                        ADD_KEYWORD("abstract");
                    }
                } else {
                    // final is only allowed in non-abstract classes
                    if (addKeywords && !modifiers.contains(QStringLiteral("final"))) {
                        ADD_KEYWORD("final");
                    }
                }

                if (modifiers.contains(QStringLiteral("private"))) {
                    // overloadable functions must not be declared private
                    showOverloadable = false;
                } else if (modifiers.contains(QStringLiteral("protected"))) {
                    // only show protected overloadable methods
                    filterPublic = true;
                } else if (addKeywords && !modifiers.contains(QStringLiteral("public"))) {
                    ADD_KEYWORD("public");
                    ADD_KEYWORD("protected");
                    ADD_KEYWORD("private");
                }

                if (modifiers.contains(QStringLiteral("static"))) {
                    filterNonStatic = true;
                } else {
                    if (addKeywords) {
                        ADD_KEYWORD("static");
                    } else {
                        filterStatic = true;
                    }
                }

                if (addKeywords) {
                    ADD_KEYWORD("function");
                }

                if (modifiers.isEmpty()) {
                    // var and const may not have any modifiers
                    ADD_KEYWORD("var");
                    ADD_KEYWORD("const");
                }
            }
            ifDebug( qCDebug(COMPLETION) << "showOverloadable" << showOverloadable; )
            // complete overloadable methods from parents
            if (showOverloadable) {
                // TODO: use m_duContext instead of ctx
                // overloadable choose is only possible inside classes which extend/implement others
                if (currentClass->baseClassesSize()) {
                    DUContext* ctx = currentClass->internalContext();
                    if (!ctx) {
                        qCDebug(COMPLETION) << "invalid class context";
                        return items;
                    }
                    QList<uint> alreadyImplemented;
                    //TODO: use the token stream here as well
                    //TODO: always add __construct, __destruct and maby other magic functions
                    // get all visible declarations and add inherited to the completion items
                    Q_FOREACH(DeclarationDepthPair decl, ctx->allDeclarations(ctx->range().end, m_duContext->topContext(), false)) {
                        ClassMemberDeclaration *member = dynamic_cast<ClassMemberDeclaration*>(decl.first);
                        ClassFunctionDeclaration *classFunc = dynamic_cast<ClassFunctionDeclaration*>(decl.first);
                        if (member) {
                            if (decl.second == 0) {
                                // this function is already implemented
                                alreadyImplemented << decl.first->indexedIdentifier().getIndex();
                                continue;
                            }
                            // skip already implemented functions
                            if (alreadyImplemented.contains(decl.first->indexedIdentifier().getIndex())) {
                                continue;
                            }
                            // skip non-static functions when requested
                            if (filterNonStatic && !member->isStatic()) {
                                continue;
                            }
                            // skip static functions when requested
                            if (filterStatic && member->isStatic()) {
                                continue;
                            }
                            // always skip private functions
                            if (member->accessPolicy() == Declaration::Private) {
                                continue;
                            }
                            // skip public functions when requested
                            if (filterPublic && member->accessPolicy() == Declaration::Public) {
                                // make sure no non-public base members are added
                                alreadyImplemented << decl.first->indexedIdentifier().getIndex();
                                continue;
                            }
                            // skip final members
                            if (classFunc && classFunc->isFinal()) {
                                // make sure no non-final base members are added
                                alreadyImplemented << decl.first->indexedIdentifier().getIndex();
                                continue;
                            }
                            // make sure we inherit or implement the base class of this member
                            if (!member->context() || !member->context()->owner()) {
                                qCDebug(COMPLETION) << "invalid parent context/owner:" << member->toString();
                                continue;
                            }
                            if (!currentClass->isPublicBaseClass(dynamic_cast<ClassDeclaration*>(member->context()->owner()),
                                                                    m_duContext->topContext())) {
                                continue;
                            }

                            ImplementationItem::HelperType itype;
                            if (!member->isFunctionDeclaration()) {
                                itype = ImplementationItem::OverrideVar;
                            } else if (member->isAbstract()) {
                                itype = ImplementationItem::Implement;
                            } else {
                                itype = ImplementationItem::Override;
                            }
                            ifDebug( qCDebug(COMPLETION) << "ImplementationItem" << itype; )

                            items << CompletionTreeItemPointer(new ImplementationItem(itype, DeclarationPointer(decl.first),
                                                                Php::CodeCompletionContext::Ptr(this), decl.second));
                            // don't add identical items twice to the completion choices
                            alreadyImplemented << decl.first->indexedIdentifier().getIndex();
                        }
                    }
                }
            } else {
                qCDebug(COMPLETION) << "invalid owner declaration for overloadable completion";
            }
        }
    } else if (m_memberAccessOperation == BackslashAccess || m_memberAccessOperation == NamespaceChoose) {
        DUContext* ctx = 0;
        if (m_namespace.isEmpty()) {
            ctx = m_duContext->topContext();
        } else {
            Q_FOREACH(Declaration* dec, m_duContext->topContext()->findDeclarations(m_namespace)) {
                if (dec->kind() == Declaration::Namespace) {
                    ctx = dec->internalContext();
                    break;
                }
            }
        }
        if (!ctx) {
            qCDebug(COMPLETION) << "could not find namespace:" << m_namespace.toString();
            return items;
        }
        Q_FOREACH(Declaration* dec, ctx->localDeclarations()) {
            if (!isValidCompletionItem(dec)) {
                continue;
            } else {
                items << CompletionTreeItemPointer(
                            new NormalDeclarationCompletionItem(
                                    DeclarationPointer(dec),
                                    Php::CodeCompletionContext::Ptr(this), depth()
                                )
                         );
            }
        }
    } else if (m_expressionResult.type()) {
        QList<DUContext*> containers = memberAccessContainers();
        qCDebug(COMPLETION) << "containers: " << containers.count();
        if (!containers.isEmpty()) {
            // get the parent class when we are inside a method declaration
            ClassDeclaration* currentClass = 0;
            if (m_duContext->owner() && m_duContext->owner()->isFunctionDeclaration() &&
                    m_duContext->parentContext() && m_duContext->parentContext()->owner()) {
                currentClass = dynamic_cast<ClassDeclaration*>(m_duContext->parentContext()->owner());
            }

            Q_FOREACH(DUContext* ctx, containers) {
                ClassDeclaration* accessedClass = dynamic_cast<ClassDeclaration*>(ctx->owner());
                if (abort)
                    return items;

                Q_FOREACH(DeclarationDepthPair decl, ctx->allDeclarations(
                                                            ctx->range().end, m_duContext->topContext(), false))
                {
                    //If we have StaticMemberAccess, which means A::Bla, show only static members,
                    //except if we're within a class that derives from the container
                    ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(decl.first);
                    if (memberAccessOperation() != StaticMemberAccess) {
                        if (classMember && classMember->isStatic())
                            continue; //Skip static class members when not doing static access
                    } else {
                        if (!classMember || !classMember->isStatic())
                            continue; //Skip static class members when not doing static access
                    }

                    // check access policy
                    if (classMember && accessedClass) {
                        // by default only show public declarations
                        Declaration::AccessPolicy ap = Declaration::Public;
                        if (currentClass) {
                            // if we are inside a class, we might want to show protected or private members
                            ClassDeclaration* memberClass = dynamic_cast<ClassDeclaration*>(classMember->context()->owner());
                            if (memberClass) {
                                if (currentClass == accessedClass) {
                                    if (currentClass == memberClass) {
                                        // we can show all members of the current class
                                        ap = Declaration::Private;
                                    } else if (currentClass->isPublicBaseClass(memberClass, m_duContext->topContext())) {
                                        // we can show all but private members of ancestors of the current class
                                        ap = Declaration::Protected;
                                    }
                                } else if (currentClass->isPublicBaseClass(accessedClass, m_duContext->topContext())
                                            && (accessedClass == memberClass ||
                                                accessedClass->isPublicBaseClass(memberClass, m_duContext->topContext()))) {
                                    // we can show all but private members of ancestors of the current class
                                    ap = Declaration::Protected;
                                }
                            }
                        }
                        if (ap < classMember->accessPolicy()) {
                            continue;
                        }
                    }

                    if (!decl.first->identifier().isEmpty())
                        items << CompletionTreeItemPointer(
                                    new NormalDeclarationCompletionItem(
                                            DeclarationPointer(
                                                decl.first),
                                                Php::CodeCompletionContext::Ptr(this),
                                                decl.second
                                            )
                                    );
                }
            }
        } else {
            qCDebug(COMPLETION) << "setContext: no container-type";
        }

    } else {
        //Show all visible declarations
        QSet<uint> existingIdentifiers;
        QList<DeclarationDepthPair> decls = m_duContext->allDeclarations(
            CursorInRevision::invalid(),
            m_duContext->topContext()
        );

        qCDebug(COMPLETION) << "setContext: using all declarations visible:" << decls.size();

        QListIterator<DeclarationDepthPair> i(decls);
        i.toBack();
        while (i.hasPrevious()) {
            DeclarationDepthPair decl = i.previous();
            Declaration* dec = decl.first;
            if (dec->kind() == Declaration::Instance) {
                // filter non-superglobal vars of other contexts
                if (dec->context() != m_duContext.data() && !m_duContext->imports(dec->context())) {
                    VariableDeclaration* vDec = dynamic_cast<VariableDeclaration*>(dec);
                    if ( vDec && !vDec->isSuperglobal() ) {
                        continue;
                    }
                }

                if (existingIdentifiers.contains(dec->indexedIdentifier().getIndex())) continue;
                existingIdentifiers.insert(dec->indexedIdentifier().getIndex());
            }
            if (abort)
                return items;
            if (!isValidCompletionItem(dec)) continue;
            items << CompletionTreeItemPointer(
                        new NormalDeclarationCompletionItem(
                                DeclarationPointer(dec),
                                Php::CodeCompletionContext::Ptr(this),
                                decl.second
                        )
                    );
        }

        Q_FOREACH(QSet<IndexedString> urlSets, completionFiles()) {
            Q_FOREACH(const IndexedString &url, urlSets) {
                if (url == m_duContext->url()) {
                    continue;
                }
                uint count = 0;
                const CodeModelItem* foundItems = 0;
                CodeModel::self().items(url, count, foundItems);
                for (uint i = 0; i < count; ++i) {
                    CodeModelItem::Kind k = foundItems[i].kind;
                    if (((k & CodeModelItem::Function) || (k & CodeModelItem::Variable)) && !(k & CodeModelItem::ClassMember)) {
                        Q_FOREACH(const ParsingEnvironmentFilePointer &env, DUChain::self()->allEnvironmentFiles(url)) {
                            if (env->language() != phpLangString) continue;
                            TopDUContext* top = env->topContext();
                            if(!top) continue;
                            if (m_duContext->imports(top)) continue;
                            QList<Declaration*> decls = top->findDeclarations(foundItems[i].id);
                            Q_FOREACH(Declaration* decl, decls) {
                                if (abort) return items;
                                // we don't want to have class methods/properties, just normal functions
                                // and other global stuff
                                if ( decl->context() && decl->context()->type() == DUContext::Class ) {
                                    continue;
                                }
                                if (!isValidCompletionItem(decl)) continue;
                                if ( VariableDeclaration* vDec = dynamic_cast<VariableDeclaration*>(decl) ) {
                                    if ( !vDec->isSuperglobal() ) {
                                        continue;
                                    }
                                }
                                items << CompletionTreeItemPointer(
                                            new NormalDeclarationCompletionItem(
                                                    DeclarationPointer(decl),
                                                    Php::CodeCompletionContext::Ptr(this)
                                            )
                                        );
                            }
                        }
                    }
                }
            }
        }

        Q_FOREACH(QSet<IndexedString> urlSets, completionFiles()) {
            Q_FOREACH(const IndexedString &url, urlSets) {
                uint count = 0;
                const CompletionCodeModelItem* foundItems = 0;
                CompletionCodeModel::self().items(url, count, foundItems);
                for (uint i = 0; i < count; ++i) {
                    if (abort) return items;
                    if (m_memberAccessOperation == ExceptionChoose) {
                        if (!(foundItems[i].kind & CompletionCodeModelItem::Exception)) continue;
                    }
                    auto files = DUChain::self()->allEnvironmentFiles(url);
                    items.reserve(files.size());
                    Q_FOREACH(const ParsingEnvironmentFilePointer &env, files) {
                        Q_ASSERT(env->language() == phpLangString);
                        items << CompletionTreeItemPointer ( new CodeModelCompletionItem(env, foundItems[i]));
                    }
                }
            }
        }
    }

    ///Find all recursive function-calls that should be shown as call-tips
    CodeCompletionContext::Ptr parentContext(this);
    do {
        if (abort)
            return items;

        parentContext = parentContext->parentContext();
        if (parentContext) {
            if (parentContext->memberAccessOperation() == CodeCompletionContext::FunctionCallAccess) {
                if (!parentContext->memberAccessContainer().allDeclarationIds().isEmpty()) {
                    Declaration* decl = parentContext->memberAccessContainer().allDeclarationIds().first()
                                            .getDeclaration(m_duContext->topContext());

                    if (!isValidCompletionItem(decl)) {
                        continue;
                    }
                    if ( !decl->isFunctionDeclaration() ) {
                        if ( ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(decl) ) {
                            // search for ctor
                            decl = 0;
                            Q_FOREACH( Declaration* dec, classDec->internalContext()->findDeclarations(Identifier("__construct")) ) {
                                if ( dec->isFunctionDeclaration() ) {
                                    decl = dec;
                                    break;
                                }
                            }
                            if ( !decl ) {
                                Q_FOREACH( Declaration* dec, classDec->internalContext()->findDeclarations(classDec->identifier()) ) {
                                    if ( dec->isFunctionDeclaration() ) {
                                        decl = dec;
                                        break;
                                    }
                                }
                            }
                            if ( !decl ) {
                                continue;
                            }
                        } else if ( !decl->type<FunctionType>() ) {
                            qCDebug(COMPLETION) << "parent decl is neither function nor class nor closure, skipping" << decl->toString();
                            continue;
                        }
                    }
                    items << CompletionTreeItemPointer(
                                new NormalDeclarationCompletionItem(
                                        DeclarationPointer(decl),
                                        Php::CodeCompletionContext::Ptr(parentContext.data())
                                )
                            );
                }
            } else {
                qCDebug(COMPLETION) << "parent-context has non function-call access type";
            }
        }
    } while (parentContext);

    if ( m_memberAccessOperation == NoMemberAccess ) {
        ///TODO: function-like statements should just be handled as a function with declaration etc.
        ///      e.g.: empty, eval, die, exit, isset, unset
        ///      but _not_ echo, print, catch, include*, require*
        ///TODO: use user's style for indentation etc.
        ADD_KEYWORD2("abstract class", "abstract class %SELECT%NAME%ENDSELECT% {\n%INDENT%\n}\n");
        ADD_KEYWORD2("final class", "final class %SELECT%NAME%ENDSELECT% {\n%INDENT%\n}\n");
        ADD_KEYWORD2("class", "class %SELECT%NAME%ENDSELECT% {\n%INDENT%\n}\n");
        ADD_KEYWORD2("interface", "interface %SELECT%NAME%ENDSELECT% {\n%INDENT%\n}\n");
        ADD_KEYWORD2("array", "array(\n%INDENT%%CURSOR%\n)");
        ADD_KEYWORD2("break", "break;\n");
        ADD_KEYWORD2("case", "case %SELECT%CASE%ENDSELECT%:\n%INDENT%\n%INDENT%break;\n");
        ADD_KEYWORD2("throw", "throw %CURSOR%;\n");
        ADD_KEYWORD2("try", "try {\n%INDENT%%CURSOR%\n} catch() {\n$%INDENT%\n}\n");
        ADD_KEYWORD2("catch", "catch(%CURSOR%) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("clone", "clone %CURSOR%;\n");
        ADD_KEYWORD2("continue", "continue;\n");
        ADD_KEYWORD2("declare", "declare(%CURSOR%);\n");
        ADD_KEYWORD2("default", "default:\n%INDENT%%CURSOR%\n%INDENT%break;\n");
        ADD_KEYWORD2("do", "do {\n%INDENT%%CURSOR%\n} while();\n");
        ADD_KEYWORD2("echo", "echo %CURSOR%;\n");
        ADD_KEYWORD2("else", "else {\n%INDENT%%CURSOR%\n}\n");
        ADD_KEYWORD2("elseif", "elseif (%CURSOR%) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("endif", "endif;");
        ADD_KEYWORD2("endforeach", "endforeach;");
        ADD_KEYWORD2("endswitch", "endswitch;");
        ADD_KEYWORD2("endwhile", "endwhile;");
        ADD_KEYWORD2("endfor", "endfor;");
        ADD_KEYWORD2("enddeclare", "enddeclare;");
        ADD_KEYWORD2("empty", "empty(%CURSOR%)");
        ADD_KEYWORD2("eval", "eval(%CURSOR%)");
        ADD_KEYWORD2("die", "die(%CURSOR%);\n");
        ADD_KEYWORD2("exit", "exit(%CURSOR%);\n");
        ///TODO: only activate when after "class NAME "
        ADD_KEYWORD("extends");
        ADD_KEYWORD("implements");
        ADD_KEYWORD2("for", "for(%CURSOR%;;) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("foreach", "Q_FOREACH(%CURSOR%) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("function", "function %SELECT%NAME%ENDSELECT%() {\n%INDENT%\n}\n");
        ADD_KEYWORD2("global", "global $%CURSOR%;");
        ADD_KEYWORD2("if", "if (%CURSOR%) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("include", "include '%CURSOR%';\n");
        ADD_KEYWORD2("include_once", "include_once '%CURSOR%';\n");
        ADD_KEYWORD2("require", "require '%CURSOR%';\n");
        ADD_KEYWORD2("require_once", "require_once '%CURSOR%';\n");
        ADD_KEYWORD2("isset", "isset(%CURSOR%)");
        ADD_KEYWORD2("list", "list(%CURSOR%)");
        ADD_KEYWORD2("print", "print %CURSOR%;\n");
        ADD_KEYWORD2("return", "return %CURSOR%;\n");
        ADD_KEYWORD2("static", "static $%CURSOR%%;\n");
        ADD_KEYWORD2("unset", "unset(%CURSOR%);\n");
        ADD_KEYWORD2("while", "while (%CURSOR%) {\n%INDENT%\n}\n");
        ADD_KEYWORD2("switch", "switch (%CURSOR%) {\n%INDENT%\n}\n");
    }

    return items;
}

inline bool CodeCompletionContext::isValidCompletionItem(Declaration* dec)
{
    if ( !dec || dec->range().isEmpty() ) {
        // hack for included files
        return false;
    }
    if ( dec->kind() == Declaration::Type && dec->qualifiedIdentifier().isEmpty() ) {
        // filter closures
        return false;
    }

    static DUChainPointer<ClassDeclaration> exceptionDecl;
    if (!exceptionDecl) {
        /// Qualified identifier for 'exception'
        static const KDevelop::QualifiedIdentifier exceptionQId(QStringLiteral("exception"));
        QList<Declaration*> decs = dec->context()->findDeclarations(exceptionQId);
        Q_ASSERT(decs.count());
        if (!decs.isEmpty()) { // additional safe-guard, see e.g. https://bugs.kde.org/show_bug.cgi?id=294218
            exceptionDecl = dynamic_cast<ClassDeclaration*>(decs.first());
            Q_ASSERT(exceptionDecl);
        }
    }
    if (!exceptionDecl) {
        // safe-guard, see: https://bugs.kde.org/show_bug.cgi?id=294218
        qWarning() << "could not find PHP-Exception declaration, related code completion will be broken.";
    }

    if (m_memberAccessOperation == ExceptionChoose
            || m_memberAccessOperation == NewClassChoose
            || m_memberAccessOperation == InterfaceChoose
            || m_memberAccessOperation == ClassExtendsChoose
            || m_memberAccessOperation == InstanceOfChoose) {
        // filter current class
        if (!m_forbiddenIdentifiers.isEmpty() && m_forbiddenIdentifiers.contains(dec->qualifiedIdentifier().index())) {
            return false;
        }
        ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);

        // filter non-classes
        if (!classDec) {
            return false;
        }
        // show non-interface and non-abstract
        else if (m_memberAccessOperation == NewClassChoose) {
            return !(classDec->classModifier() & ClassDeclarationData::Abstract)
                   && classDec->classType() == ClassDeclarationData::Class;
        }
        // filter non-exception classes
        else if (m_memberAccessOperation == ExceptionChoose) {
            if (!exceptionDecl) {
                // safe-guard, see: https://bugs.kde.org/show_bug.cgi?id=294218
                return false;
            }
            return classDec->equalQualifiedIdentifier(exceptionDecl.data())
                   || classDec->isPublicBaseClass(exceptionDecl.data(), m_duContext->topContext());
        }
        // show interfaces
        else if (m_memberAccessOperation == InterfaceChoose) {
            return classDec->classType() == ClassDeclarationData::Interface;
        }
        // show anything but final classes and interfaces
        else if (m_memberAccessOperation == ClassExtendsChoose) {
            return !(classDec->classModifier() & ClassDeclarationData::Final)
                   && classDec->classType() == ClassDeclarationData::Class;
        }
        else if (m_memberAccessOperation == InstanceOfChoose) {
            return true;
        }
    }

    if (m_memberAccessOperation == ExceptionInstanceChoose) {
        if (!exceptionDecl) {
            // safe-guard, see: https://bugs.kde.org/show_bug.cgi?id=294218
            return false;
        }
        if (dec->kind() != Declaration::Instance) {
            return false;
        }
        StructureType::Ptr structType = dec->type<StructureType>();
        if (!structType) {
            return false;
        }
        ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(structType->declaration(dec->topContext()));
        if (!classDec) {
            return false;
        }
        return classDec->isPublicBaseClass(exceptionDecl.data(), m_duContext->topContext());
    }

    if (m_memberAccessOperation == NoMemberAccess) {
        // filter private methods and class members when doing a global completion
        // when we are inside a class function, don't filter the private stuff
        // of the current class
        // NOTE: ClassFunctionDeclaration inherits ClassMemberDeclaration
        // NOTE: both have to have a parent context with type class
        if ( dec->context() && dec->context()->type() == DUContext::Class
                && m_duContext->parentContext() != dec->context() )
        {
            if ( ClassMemberDeclaration* memberDec = dynamic_cast<ClassMemberDeclaration*>(dec) ) {
                if ( memberDec->accessPolicy() == Declaration::Private ) {
                    return false;
                }
            }
        }
        if ( !dec->isFunctionDeclaration() && m_duContext.data() == dec->context() && m_position < dec->range().start ) {
            return false;
        }
    }

    if (m_memberAccessOperation == NamespaceChoose) {
        return dec->kind() == Declaration::Namespace;
    }

    return true;
}

QList<QSet<IndexedString> > CodeCompletionContext::completionFiles()
{
    QList<QSet<IndexedString> > ret;
    if (ICore::self()) {
        auto projects = ICore::self()->projectController()->projects();
        ret.reserve(projects.size());
        Q_FOREACH(IProject* project, projects) {
            ret << project->fileSet();
        }
    }
    return ret;
}

}
