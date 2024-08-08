/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "expressionparser.h"
#include "parsesession.h"
#include "editorintegrator.h"
#include "phpast.h"
#include "phpparser.h"
#include "phpdebugvisitor.h"
#include "expressionvisitor.h"

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

#include "duchaindebug.h"

using namespace KDevelop;

namespace Php
{

ExpressionParser::ExpressionParser(bool debug)
        : m_debug(debug), m_createProblems(false)
{
}

void ExpressionParser::setCreateProblems(bool v)
{
    m_createProblems = v;
}

ExpressionEvaluationResult ExpressionParser::evaluateType(const QByteArray& expression, DUContextPointer context,
                                                          const CursorInRevision &offset)
{
    if (m_debug)
        qCDebug(DUCHAIN) << "==== .Evaluating ..:" << endl << expression;

    ParseSession* session = new ParseSession();
    session->setContents(expression);
    Parser* parser = session->createParser(Parser::DefaultState);
    ExprAst* ast = nullptr;
    if (!parser->parseExpr(&ast)) {
        qCDebug(DUCHAIN) << "Failed to parse \"" << expression << "\"";
        delete session;
        delete parser;
        return ExpressionEvaluationResult();
    }
    ast->ducontext = dynamic_cast<DUContext*>(context.data());

    EditorIntegrator* editor = new EditorIntegrator(session);
    ExpressionEvaluationResult ret = evaluateType(ast, editor, offset);
    delete editor;
    delete session;
    delete parser;

    return ret;
}

ExpressionEvaluationResult ExpressionParser::evaluateType(AstNode* ast, EditorIntegrator* editor)
{
    return evaluateType(ast, editor, CursorInRevision::invalid());
}

ExpressionEvaluationResult ExpressionParser::evaluateType(AstNode* ast, EditorIntegrator* editor,
                                                          const CursorInRevision &offset)
{
    if (m_debug) {
        qCDebug(DUCHAIN) << "===== AST:";
        DebugVisitor debugVisitor(editor->parseSession()->tokenStream(), editor->parseSession()->contents());
        debugVisitor.visitNode(ast);
    }

    ExpressionVisitor v(editor);
    v.setOffset(offset);
    v.setCreateProblems(m_createProblems);
    v.visitNode(ast);

    return v.result();
}

}
