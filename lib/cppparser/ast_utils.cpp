/*
    SPDX-FileCopyrightText: 2002 Roberto Raggi <roberto@kdevelop.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ast_utils.h"

#include "ast.h"
#include "debug_utils.h"

#include <QRegularExpression>
#include <QStringList>

#include <KLocalizedString>

void scopeOfNode(AST* ast, QStringList& scope)
{
    if (!ast)
        return;

    if (ast->parent())
        scopeOfNode(ast->parent(), scope);

    QString s;
    switch (ast->nodeType()) {
    case NodeType_ClassSpecifier:
        if (((ClassSpecifierAST*)ast)->name()) {
            s = ((ClassSpecifierAST*)ast)->name()->text();
            s = s.isEmpty() ? QString::fromLatin1("<unnamed>") : s;
            scope.push_back(s);
        }
        break;

    case NodeType_Namespace: {
        AST* namespaceName = ((NamespaceAST*)ast)->namespaceName();
        s = namespaceName ? namespaceName->text() : QString::fromLatin1("<unnamed>");
        scope.push_back(s);
    }
    break;

    case NodeType_FunctionDefinition: {
        FunctionDefinitionAST* funDef = static_cast<FunctionDefinitionAST*>(ast);
        DeclaratorAST* d = funDef->initDeclarator()->declarator();

        // hotfix for bug #68726
        if (!d->declaratorId())
            break;

        QList<ClassOrNamespaceNameAST*> l = d->declaratorId()->classOrNamespaceNameList();
        for (int i = 0; i < l.size(); ++i) {
            AST* name = l.at(i)->name();
            scope.push_back(name->text());
        }
    }
    break;

    default:
        break;
    }
}

QString typeSpecToString(TypeSpecifierAST* typeSpec)  /// @todo remove
{
    if (!typeSpec)
        return QString();

    return typeSpec->text().replace(QRegularExpression(QLatin1String(" :: ")), QLatin1String("::"));
}

QString declaratorToString(DeclaratorAST* declarator, const QString& scope, bool skipPtrOp)
{
    if (!declarator)
        return QString();

    QString text;

    if (!skipPtrOp) {
        QList<AST*> ptrOpList = declarator->ptrOpList();
        for (int i = 0; i < ptrOpList.size(); ++i) {
            text += ptrOpList.at(i)->text();
        }
        text += QLatin1Char(' ');
    }

    text += scope;

    if (declarator->subDeclarator())
        text += QString::fromLatin1("(") + declaratorToString(declarator->subDeclarator()) + QString::fromLatin1(")");

    if (declarator->declaratorId())
        text += declarator->declaratorId()->text();

    QList<AST*> arrays = declarator->arrayDimensionList();
    for (int i = 0; i < arrays.size(); ++i) {
        text += QLatin1String("[]");
    }

    if (declarator->parameterDeclarationClause()) {
        text += QLatin1Char('(');

        ParameterDeclarationListAST* l = declarator->parameterDeclarationClause()->parameterDeclarationList();
        if (l != 0) {
            QList<ParameterDeclarationAST*> params = l->parameterList();
            for (int i = 0; i < params.size(); ++i) {
                QString type = typeSpecToString(params.at(i)->typeSpec());
                text += type;
                if (!type.isEmpty())
                    text += QLatin1Char(' ');
                text += declaratorToString(params.at(i)->declarator());

                if (params.at(i))
                    text += QLatin1String(", ");
            }
        }

        text += QLatin1Char(')');

        if (declarator->constant() != 0)
            text += QLatin1String(" const");

        if (declarator->override_() != 0)
            text += QLatin1String(" override");

        if (declarator->final_() != 0)
            text += QLatin1String(" final");
    }

    return text.replace(QRegularExpression(QLatin1String(" :: ")), QLatin1String("::")).simplified();
}
