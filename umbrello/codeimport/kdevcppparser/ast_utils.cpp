/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ast_utils.h"

#include "ast.h"
#include "debug_utils.h"

#include <QtCore/QStringList>
#include <QtCore/QRegExp>

#include <klocale.h>

AST* findNodeAt( AST* node, Position const& position )
{
    // uDebug() << "findNodeAt(" << node << ")";

    if( !node )
        return 0;

    Position startPosition = node->getStartPosition();
    Position endPosition = node->getEndPosition();

    if( (position >= startPosition) && (position < endPosition) ) {
        QList<AST*> children = node->children();
        for( int i = 0; i < children.size(); ++i ) {
            AST* a = children.at(i);
            AST* r = findNodeAt( a, position );
            if( r )
                return r;
        }

        return node;
    }

    return 0;
}

void scopeOfNode( AST* ast, QStringList& scope )
{
    if( !ast )
        return;

    if( ast->parent() )
        scopeOfNode( ast->parent(), scope );

    QString s;
    switch( ast->nodeType() )
    {
    case NodeType_ClassSpecifier:
        if( ((ClassSpecifierAST*)ast)->name() ){
            s = ((ClassSpecifierAST*)ast)->name()->text();
            s = s.isEmpty() ? QString::fromLatin1("<unnamed>") : s;
            scope.push_back( s );
        }
        break;

    case NodeType_Namespace:
    {
        AST* namespaceName = ((NamespaceAST*)ast)->namespaceName();
        s = namespaceName ? namespaceName->text() : QString::fromLatin1("<unnamed>");
        scope.push_back( s );
    }
    break;

    case NodeType_FunctionDefinition:
    {
        FunctionDefinitionAST* funDef = static_cast<FunctionDefinitionAST*>( ast );
        DeclaratorAST* d = funDef->initDeclarator()->declarator();

        // hotfix for bug #68726
        if ( !d->declaratorId() )
            break;

        QList<ClassOrNamespaceNameAST*> l = d->declaratorId()->classOrNamespaceNameList();
        for( int i = 0; i < l.size(); ++i ) {
            AST* name = l.at(i)->name();
            scope.push_back( name->text() );
        }
    }
    break;

    default:
        break;
    }
}

QString typeSpecToString( TypeSpecifierAST* typeSpec )  /// @todo remove
{
    if( !typeSpec )
        return QString();

    return typeSpec->text().replace( QRegExp(" :: "), "::" );
}

QString declaratorToString( DeclaratorAST* declarator, const QString& scope, bool skipPtrOp )
{
   if( !declarator )
       return QString();

   QString text;

   if( !skipPtrOp ){
       QList<AST*> ptrOpList = declarator->ptrOpList();
       for( int i = 0; i < ptrOpList.size(); ++i ) {
          text += ptrOpList.at(i)->text();
       }
       text += ' ';
   }

   text += scope;

   if( declarator->subDeclarator() )
       text += QString::fromLatin1("(") + declaratorToString(declarator->subDeclarator()) + QString::fromLatin1(")");

   if( declarator->declaratorId() )
       text += declarator->declaratorId()->text();

   QList<AST*> arrays = declarator->arrayDimensionList();
   for( int i = 0; i < arrays.size(); ++i ) {
       text += "[]";
   }

   if( declarator->parameterDeclarationClause() ){
       text += "( ";

       ParameterDeclarationListAST* l = declarator->parameterDeclarationClause()->parameterDeclarationList();
       if( l != 0 ){
           QList<ParameterDeclarationAST*> params = l->parameterList();
           for( int i = 0; i < params.size(); ++i ) {
               QString type = typeSpecToString( params.at(i)->typeSpec() );
               text += type;
               if( !type.isEmpty() )
                   text += ' ';
               text += declaratorToString( params.at(i)->declarator() );

               if( params.at(i) )
                   text += ", ";
           }
       }

       text += " )";

       if( declarator->constant() != 0 )
           text += " const";
   }

   return text.replace( QRegExp(" :: "), "::" ).simplified();
}
