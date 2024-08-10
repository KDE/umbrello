/*
    SPDX-FileCopyrightText: 2002 Roberto Raggi <roberto@kdevelop.org>
    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef AST_UTILS_H
#define AST_UTILS_H

#include <QString>

class AST;
class DeclaratorAST;
class TypeSpecifierAST;

namespace KTextEditor
{
class EditInterface;
}

void scopeOfNode(AST* ast, QStringList&);
QString typeSpecToString(TypeSpecifierAST* typeSpec);
QString declaratorToString(DeclaratorAST* declarator, const QString& scope = QString(), bool skipPtrOp=false);

#endif // AST_UTILS_H
