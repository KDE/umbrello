/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AST_UTILS_H
#define AST_UTILS_H

#include <QtCore/QString>

class AST;
class DeclaratorAST;
class TypeSpecifierAST;
class QStringList;

namespace KTextEditor{ class EditInterface; }

AST* findNodeAt( AST* unit, int line, int column );
void scopeOfNode( AST* ast, QStringList& );
QString typeSpecToString( TypeSpecifierAST* typeSpec );
QString declaratorToString( DeclaratorAST* declarator, const QString& scope = QString(), bool skipPtrOp=false );

#endif // AST_UTILS_H
