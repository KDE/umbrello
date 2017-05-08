/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
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
#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <language/duchain/duchainpointer.h>

#include "phpduchainexport.h"
#include "expressionevaluationresult.h"

namespace Php
{
struct AstNode;
class EditorIntegrator;

class KDEVPHPDUCHAIN_EXPORT ExpressionParser
{
public:
    /**
     * @param debug Enables additional output
     */
    explicit ExpressionParser(bool debug = false);

    /**
     * By default, no problems are created at the top-ducontext.
     */
    void setCreateProblems(bool v);

    /**
     * Evaluate the @p expression and find it's type and last-used declaration.
     *
     * @param offset Set this to the front-edge of the expression.
     *               Used in the ExpressionVisitor to find visible declarations.
     *
     * @see ExpressionVisitor
     */
    ExpressionEvaluationResult evaluateType(const QByteArray& expression, KDevelop::DUContextPointer context,
                                            const KDevelop::CursorInRevision &offset);
    /**
     * Sets up an ExpressionVisitor and returns it's result when visiting @p ast .
     *
     * @see ExpressionVisitor
     */
    ExpressionEvaluationResult evaluateType(AstNode* ast, EditorIntegrator* editor);

private:
    /**
     * This is private instead of reusing the method above with a default argument
     * for the offset, because we _never_ want to use this directly.
     */
    ExpressionEvaluationResult evaluateType(AstNode* ast, EditorIntegrator* editor,
                                            const KDevelop::CursorInRevision &offset);
    bool m_debug;
    bool m_createProblems;
};


}
#endif
