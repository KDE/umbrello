/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PHPEDITORINTEGRATOR_H
#define PHPEDITORINTEGRATOR_H

#include <language/editor/rangeinrevision.h>

#include "phpduchainexport.h"

#include "kdev-pg-token-stream.h"

namespace Php
{

class ParseSession;
struct AstNode;


/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
 *
 * \todo introduce stacks for the state?
 */
class KDEVPHPDUCHAIN_EXPORT EditorIntegrator
{
public:
    EditorIntegrator(ParseSession* session);

    ParseSession* parseSession() const;

    enum Edge {
        FrontEdge,
        BackEdge
    };

    enum RangeEdge {
        InnerEdge,
        OuterEdge
    };

    /**
     * Finds the location and \a file where the given \a token was parsed from.
     *
     * \param token token to locate
     * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
     *
     * \returns the requested cursor relating to the start or end of the given token.
     */
    KDevelop::CursorInRevision findPosition(const KDevPG::TokenStream::Token& token, Edge edge = BackEdge) const;

    /**
     * Finds the location and \a file where the given \a token was parsed from.
     *
     * \param token token to locate
     * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
     *
     * \returns the requested cursor relating to the start or end of the given token.
     */
    KDevelop::CursorInRevision findPosition(qint64 token, Edge edge = BackEdge) const;

    /**
     * Create a range encompassing the given AstNode \a node.
     *
     * \overload
     */
    KDevelop::RangeInRevision findRange(AstNode* node, RangeEdge = OuterEdge) const;

    /**
     * Create a range encompassing the given AstNode \a nodes.
     *
     * \overload
     */
    KDevelop::RangeInRevision findRange(AstNode* from, AstNode* to) const;

    /**
     * Create a range encompassing the given AstNode \a token.
     *
     * \overload
     */
    KDevelop::RangeInRevision findRange(const KDevPG::TokenStream::Token& token) const;

    /**
     * Create a range encompassing the given AstNode \a token.
     *
     * \overload
     */
    KDevelop::RangeInRevision findRange(qint64 token) const;

    /**
     * Create a range encompassing the given AstNode \a tokens.
     *
     * \overload
     */
    KDevelop::RangeInRevision findRange(qint64 start_token, qint64 end_token) const;

    /**
     * Retrieve the string represented by a token.
     */
    QString tokenToString(qint64 token) const;

private:
    ParseSession* m_session;
};

}

#endif // PHPEDITORINTEGRATOR_H

