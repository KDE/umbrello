/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "helper.h"

#include <KParts/MainWindow>
#include <KLocalizedString>

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/duchain.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include "editorintegrator.h"
#include "../parser/parsesession.h"
#include "phpast.h"
#include "phpdefaultvisitor.h"
#include "declarations/classdeclaration.h"
#include "declarations/classmethoddeclaration.h"
#include "declarations/functiondeclaration.h"

#include "duchaindebug.h"

#define ifDebug(x)

using namespace KDevelop;

namespace Php
{

bool isMatch(Declaration* declaration, DeclarationType declarationType)
{
    if (declarationType == ClassDeclarationType
            && dynamic_cast<ClassDeclaration*>(declaration)
       ) {
        return true;
    } else if (declarationType == FunctionDeclarationType
               && dynamic_cast<FunctionDeclaration*>(declaration)
              ) {
        return true;
    } else if (declarationType == ConstantDeclarationType
               && declaration->abstractType() && declaration->abstractType()->modifiers() & AbstractType::ConstModifier
               && (!declaration->context() || declaration->context()->type() != DUContext::Class)
              ) {
        return true;
    } else if (declarationType == GlobalVariableDeclarationType
               && declaration->kind() == Declaration::Instance
               && !(declaration->abstractType() && declaration->abstractType()->modifiers() & AbstractType::ConstModifier)
              ) {
        return true;
    } else if (declarationType == NamespaceDeclarationType
               && (declaration->kind() == Declaration::Namespace || declaration->kind() == Declaration::NamespaceAlias || dynamic_cast<ClassDeclaration*>(declaration)) )
    {
        return true;
    }
    return false;
}

DeclarationPointer findDeclarationImportHelper(DUContext* currentContext, const QualifiedIdentifier& id,
        DeclarationType declarationType)
{
    /// Qualified identifier for 'self'
    static const QualifiedIdentifier selfQId(QStringLiteral("self"));
    /// Qualified identifier for 'parent'
    static const QualifiedIdentifier parentQId(QStringLiteral("parent"));
    /// Qualified identifier for 'static'
    static const QualifiedIdentifier staticQId(QStringLiteral("static"));

    ifDebug(qCDebug(DUCHAIN) << id.toString() << declarationType;)
    if (declarationType == ClassDeclarationType && id == selfQId) {
        DUChainReadLocker lock(DUChain::lock());
        if (currentContext->type() == DUContext::Class) {
            return DeclarationPointer(currentContext->owner());
        } else if (currentContext->parentContext() && currentContext->parentContext()->type() == DUContext::Class) {
            return DeclarationPointer(currentContext->parentContext()->owner());
        } else {
            return DeclarationPointer();
        }
    } else if (declarationType == ClassDeclarationType && id == staticQId) {
        DUChainReadLocker lock;
        if (currentContext->type() == DUContext::Class) {
            return DeclarationPointer(currentContext->owner());
        } else if (currentContext->parentContext() && currentContext->parentContext()->type() == DUContext::Class) {
            return DeclarationPointer(currentContext->parentContext()->owner());
        } else {
            return DeclarationPointer();
        }
    } else if (declarationType == ClassDeclarationType && id == parentQId) {
        //there can be just one Class-Context imported
        DUChainReadLocker lock;
        DUContext* classCtx = nullptr;
        if (currentContext->type() == DUContext::Class) {
            classCtx = currentContext;
        } else if (currentContext->parentContext() && currentContext->parentContext()->type() == DUContext::Class) {
            classCtx = currentContext->parentContext();
        }
        if (classCtx) {
            for(const DUContext::Import &i : classCtx->importedParentContexts()) {
                DUContext* ctx = i.context(classCtx->topContext());
                if (ctx && ctx->type() == DUContext::Class) {
                    return DeclarationPointer(ctx->owner());
                }
            }
        }
        return DeclarationPointer();
    } else {
        DUChainReadLocker lock;
        QList<Declaration*> foundDeclarations = currentContext->topContext()->findDeclarations(id);
        if (foundDeclarations.isEmpty()) {
            // If it's not in the top context, try the current context (namespaces...)
            // this fixes the bug: https://bugs.kde.org/show_bug.cgi?id=322274
            foundDeclarations = currentContext->findDeclarations(id);
        }
        if (foundDeclarations.isEmpty()) {
            // If it is neither in the top not the current context it might be defined in a different context
            // Look up with fully qualified identifier
            foundDeclarations = currentContext->topContext()->findDeclarations(identifierWithNamespace(id, currentContext));
        }

        for(Declaration *declaration : foundDeclarations) {
            if (isMatch(declaration, declarationType)) {
                return DeclarationPointer(declaration);
            }
        }
        if ( currentContext->url() == internalFunctionFile() ) {
            // when compiling php internal functions, we don't need to ask the persistent symbol table for anything
            return DeclarationPointer();
        }

        lock.unlock();

        if (declarationType != GlobalVariableDeclarationType) {
            ifDebug(qCDebug(DUCHAIN) << "No declarations found with findDeclarations, trying through PersistentSymbolTable";)
            DeclarationPointer decl;

            decl = findDeclarationInPST(currentContext, id, declarationType);

            if (!decl)
            {
                decl = findDeclarationInPST(currentContext, identifierWithNamespace(id, currentContext), declarationType);
            }

            if (decl) {
                ifDebug(qCDebug(DUCHAIN) << "PST declaration exists";)
            } else {
                ifDebug(qCDebug(DUCHAIN) << "PST declaration does not exist";)
            }
            return decl;
        }
    }

    ifDebug(qCDebug(DUCHAIN) << "returning 0";)
    return DeclarationPointer();
}

DeclarationPointer findDeclarationInPST(DUContext* currentContext, QualifiedIdentifier id, DeclarationType declarationType)
{
    ifDebug(qCDebug(DUCHAIN) << "PST: " << id.toString() << declarationType;)
    uint nr;
    const IndexedDeclaration* declarations = nullptr;
    DUChainWriteLocker wlock;
    PersistentSymbolTable::self().declarations(id, nr, declarations);
    ifDebug(qCDebug(DUCHAIN) << "found declarations:" << nr;)
    /// Indexed string for 'Php', identifies environment files from this language plugin
    static const IndexedString phpLangString("Php");

    for (uint i = 0; i < nr; ++i) {
        ParsingEnvironmentFilePointer env = DUChain::self()->environmentFileForDocument(declarations[i].indexedTopContext());
        if(!env) {
            ifDebug(qCDebug(DUCHAIN) << "skipping declaration, missing meta-data";)
            continue;
        }
        if(env->language() != phpLangString) {
            ifDebug(qCDebug(DUCHAIN) << "skipping declaration, invalid language" << env->language().str();)
            continue;
        }

        if (!declarations[i].declaration()) {
            ifDebug(qCDebug(DUCHAIN) << "skipping declaration, doesn't have declaration";)
            continue;
        } else if (!isMatch(declarations[i].declaration(), declarationType)) {
            ifDebug(qCDebug(DUCHAIN) << "skipping declaration, doesn't match with declarationType";)
            continue;
        }
        TopDUContext* top = declarations[i].declaration()->context()->topContext();

        /*
            * NOTE:
            * To enable PHPUnit test classes, this check has been disabled.
            * Formerly it only loaded declarations from open projects, but PHPUnit declarations
            * belong to no project.
            *
            * If this behavior is unwanted, reinstate the check.
            * Miha Cancula <miha@noughmad.eu>
            */
        /*
        if (ICore::self() && !ICore::self()->projectController()->projects().isEmpty()) {
            bool loadedProjectContainsUrl = false;
            Q_FOREACH(IProject *project, ICore::self()->projectController()->projects()) {
                if (project->fileSet().contains(top->url())) {
                    loadedProjectContainsUrl = true;
                    break;
                }
            }
            if (!loadedProjectContainsUrl) {
                ifDebug(qCDebug(DUCHAIN) << "skipping declaration, not in loaded project";)
                continue;
            }
        }
        */

        currentContext->topContext()->addImportedParentContext(top);
        currentContext->topContext()->parsingEnvironmentFile()
        ->addModificationRevisions(top->parsingEnvironmentFile()->allModificationRevisions());
        currentContext->topContext()->updateImportsCache();
        ifDebug(qCDebug(DUCHAIN) << "using" << declarations[i].declaration()->toString() << top->url();)
        wlock.unlock();
        return DeclarationPointer(declarations[i].declaration());
    }

    wlock.unlock();
    ifDebug(qCDebug(DUCHAIN) << "returning 0";)
    return DeclarationPointer();
}

QByteArray formatComment(AstNode* node, EditorIntegrator* editor)
{
    return KDevelop::formatComment(editor->parseSession()->docComment(node->startToken).toUtf8());
}

//Helper visitor to extract a commonScalar node
//used to get the value of an function call argument
class ScalarExpressionVisitor : public DefaultVisitor
{
public:
    ScalarExpressionVisitor() : m_node(0) {}
    CommonScalarAst* node() const {
        return m_node;
    }
private:
    virtual void visitCommonScalar(CommonScalarAst* node) {
        m_node = node;
    }
    CommonScalarAst* m_node;
};

CommonScalarAst* findCommonScalar(AstNode* node)
{
    ScalarExpressionVisitor visitor;
    visitor.visitNode(node);
    return visitor.node();
}

static bool includeExists(const Path &include)
{
    const QString path = include.pathOrUrl();
    {
        DUChainReadLocker lock;
        if (DUChain::self()->chainForDocument(IndexedString(path))) {
            return true;
        }
    }
    if ( include.isLocalFile() ) {
        return QFile::exists(path);
    } else {
        return false;
    }
}

static IndexedString findIncludeFile(const QString &includePath, const IndexedString &currentDocument)
{
    if ( includePath.isEmpty() ) {
        return IndexedString();
    }

    // check remote files
    if ( includePath.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
            || includePath.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive) ) {
        // always expect remote includes to exist
        return IndexedString(includePath);
    }

    const Path currentPath(currentDocument.str());

    // look for file relative to current url
    Path include = Path(currentPath.parent(), includePath);
    if ( includeExists(include) ) {
        return IndexedString(include.pathOrUrl());
    }

    // in the first round look for a project that is a parent of the current document
    // in the next round look for any project
    for (int i = 0; i < 2; ++i) {
        for(IProject* project : ICore::self()->projectController()->projects()) {
            if ( !i && !project->path().isParentOf(currentPath)) {
                continue;
            }
            include = Path(project->path(), includePath);
            if ( includeExists(include) ) {
                return IndexedString(include.pathOrUrl());
            }
        }
    }

    //TODO configurable include paths

    return IndexedString();
}

IndexedString getIncludeFileForNode(UnaryExpressionAst* node, EditorIntegrator* editor) {
    if ( node->includeExpression ) {
        //find name of the constant (first argument of the function call)
        CommonScalarAst* scalar = findCommonScalar(node->includeExpression);
        if (scalar && scalar->string != -1) {
            QString str = editor->parseSession()->symbol(scalar->string);
            str = str.mid(1, str.length() - 2);
            if ( str == QLatin1String(".") || str == QLatin1String("..") || str.endsWith('/') ) {
                return IndexedString();
            }
            return findIncludeFile(str, editor->parseSession()->currentDocument());
        }
    }

    return IndexedString();
}

QString prettyName(Declaration* dec) {
    if (!dec) {
        return {};
    } else if ( dec->context() && dec->context()->type() == DUContext::Class && dec->isFunctionDeclaration() ) {
        ClassMethodDeclaration* classMember = dynamic_cast<ClassMethodDeclaration*>(dec);
        Q_ASSERT(classMember);
        return classMember->prettyName().str();
    } else if ( dec->isFunctionDeclaration() ) {
        FunctionDeclaration* func = dynamic_cast<FunctionDeclaration*>(dec);
        Q_ASSERT(func);
        return func->prettyName().str();
    } else if ( dec->internalContext() && dec->internalContext()->type() == DUContext::Class ) {
        ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);
        Q_ASSERT(classDec);
        return classDec->prettyName().str();
    } else {
        return dec->identifier().toString();
    }
}

const KDevelop::IndexedString& internalFunctionFile()
{
    static const KDevelop::IndexedString internalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevphpsupport/phpfunctions.php")));
    return internalFile;
}

const KDevelop::IndexedString& phpLanguageString()
{
    static const KDevelop::IndexedString phpLangString("Php");
    return phpLangString;
}

const IndexedString& internalTestFile()
{
    static const KDevelop::IndexedString phpUnitFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevphpsupport/phpunitdeclarations.php")));
    return phpUnitFile;
}

QualifiedIdentifier identifierForNamespace(NamespacedIdentifierAst* node, EditorIntegrator* editor, bool lastIsConstIdentifier)
{
    QualifiedIdentifier id;
    if (node->isGlobal != -1) {
        id.setExplicitlyGlobal(true);
    }
    const KDevPG::ListNode< IdentifierAst* >* it = node->namespaceNameSequence->front();
    do {
        if (lastIsConstIdentifier && !it->hasNext()) {
            id.push(Identifier(editor->parseSession()->symbol(it->element)));
        } else {
            id.push(Identifier(editor->parseSession()->symbol(it->element).toLower()));
        }
    } while (it->hasNext() && (it = it->next));
    return id;
}

QualifiedIdentifier identifierWithNamespace(const QualifiedIdentifier& base, DUContext* context)
{
    DUChainReadLocker lock;
    auto scope = context;
    while (scope && scope->type() != DUContext::Namespace) {
        scope = scope->parentContext();
    }

    if (scope) {
        return scope->scopeIdentifier() + base;
    } else {
        return base;
    }
}

}
