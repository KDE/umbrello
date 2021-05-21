/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "helper.h"

#include <KParts/MainWindow>
#include <KLocalizedString>
#include <KStandardDirs>

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

#include "editorintegrator.h"
#include "../parser/parsesession.h"
#include "phpast.h"
#include "phpdefaultvisitor.h"
#include "declarations/classdeclaration.h"
#include "declarations/classmethoddeclaration.h"
#include "declarations/functiondeclaration.h"

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
    static const QualifiedIdentifier selfQId("self");
    /// Qualified identifier for 'parent'
    static const QualifiedIdentifier parentQId("parent");
    /// Qualified identifier for 'static'
    static const QualifiedIdentifier staticQId("static");

    ifDebug(kDebug() << id.toString() << declarationType;)
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
        DUContext* classCtx = 0;
        if (currentContext->type() == DUContext::Class) {
            classCtx = currentContext;
        } else if (currentContext->parentContext() && currentContext->parentContext()->type() == DUContext::Class) {
            classCtx = currentContext->parentContext();
        }
        if (classCtx) {
            foreach(const DUContext::Import &i, classCtx->importedParentContexts()) {
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

        foreach(Declaration *declaration, foundDeclarations) {
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
            ifDebug(kDebug() << "No declarations found with findDeclarations, trying through PersistentSymbolTable";)
            DeclarationPointer decl;

            decl = findDeclarationInPST(currentContext, id, declarationType);

            if (!decl)
            {
                decl = findDeclarationInPST(currentContext, identifierWithNamespace(id, currentContext), declarationType);
            }

            if (decl) {
                ifDebug(kDebug() << "PST declaration exists";)
            } else {
                ifDebug(kDebug() << "PST declaration does not exist";)
            }
            return decl;
        }
    }

    ifDebug(kDebug() << "returning 0";)
    return DeclarationPointer();
}

DeclarationPointer findDeclarationInPST(DUContext* currentContext, QualifiedIdentifier id, DeclarationType declarationType)
{
    ifDebug(kDebug() << "PST: " << id.toString() << declarationType;)
    uint nr;
    const IndexedDeclaration* declarations = 0;
    DUChainWriteLocker wlock;
    PersistentSymbolTable::self().declarations(id, nr, declarations);
    ifDebug(kDebug() << "found declarations:" << nr;)
    /// Indexed string for 'Php', identifies environment files from this language plugin
    static const IndexedString phpLangString("Php");

    for (uint i = 0; i < nr; ++i) {
        ParsingEnvironmentFilePointer env = DUChain::self()->environmentFileForDocument(declarations[i].indexedTopContext());
        if(!env) {
            ifDebug(kDebug() << "skipping declaration, missing meta-data";)
            continue;
        }
        if(env->language() != phpLangString) {
            ifDebug(kDebug() << "skipping declaration, invalid language" << env->language().str();)
            continue;
        }

        if (!declarations[i].declaration()) {
            ifDebug(kDebug() << "skipping declaration, doesn't have declaration";)
            continue;
        } else if (!isMatch(declarations[i].declaration(), declarationType)) {
            ifDebug(kDebug() << "skipping declaration, doesn't match with declarationType";)
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
            foreach(IProject *project, ICore::self()->projectController()->projects()) {
                if (project->fileSet().contains(top->url())) {
                    loadedProjectContainsUrl = true;
                    break;
                }
            }
            if (!loadedProjectContainsUrl) {
                ifDebug(kDebug() << "skipping declaration, not in loaded project";)
                continue;
            }
        }
        */

        currentContext->topContext()->addImportedParentContext(top);
        currentContext->topContext()->parsingEnvironmentFile()
        ->addModificationRevisions(top->parsingEnvironmentFile()->allModificationRevisions());
        currentContext->topContext()->updateImportsCache();
        ifDebug(kDebug() << "using" << declarations[i].declaration()->toString() << top->url().str();)
        wlock.unlock();
        return DeclarationPointer(declarations[i].declaration());
    }

    wlock.unlock();
    ifDebug(kDebug() << "returning 0";)
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

static bool includeExists(const KUrl &url)
{
    {
        DUChainReadLocker lock(DUChain::lock());
        ///TODO: this may load the chain into memory, do we really want that here?
        if (DUChain::self()->chainForDocument(url)) {
            return true;
        }
    }
    if ( url.isLocalFile() ) {
        return QFile::exists(url.toLocalFile());
    } else {
        return false;
    }
}

KUrl getUrlForBase(const QString &includeFile, const KUrl &baseUrl) {
    if ( includeFile.isEmpty() ) {
        return baseUrl;
    }
    KUrl url = baseUrl;
    if ( includeFile[0] == '/' ) {
        url.setPath(includeFile);
    } else {
        url.addPath(includeFile);
    }
    url.cleanPath();
    return url;
}

IndexedString findIncludeFileUrl(const QString &includeFile, const KUrl &currentUrl)
{
    if ( includeFile.isEmpty() ) {
        return IndexedString();
    }

    // check remote files
    if ( includeFile.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
            || includeFile.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive) ) {
        // always expect remote includes to exist
        return IndexedString(includeFile);
    }

    KUrl url;

    // look for file relative to current url
    url = getUrlForBase(includeFile, currentUrl.upUrl());
    if ( ICore::self()->projectController()->findProjectForUrl(url) || includeExists(url) ) {
        return IndexedString(url);
    }

    // look for file relative to current project base
    IProject* ownProject = ICore::self()->projectController()->findProjectForUrl(currentUrl);
    if ( ownProject ) {
        url = getUrlForBase(includeFile, ownProject->folder());
        if ( ownProject->inProject(IndexedString(url)) || includeExists(url) ) {
            return IndexedString(url);
        }
    }

    // now look in all other projects
    foreach(IProject* project, ICore::self()->projectController()->projects()) {
        if ( project == ownProject ) {
            continue;
        }
        url = getUrlForBase(includeFile, project->folder());
        if ( project->inProject(IndexedString(url)) || includeExists(url) ) {
            return IndexedString(url);
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
            if ( str == "." || str == ".." || str.endsWith('/') ) {
                return IndexedString();
            }
            return findIncludeFileUrl(str, editor->parseSession()->currentDocument().toUrl());
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
    static const KDevelop::IndexedString internalFile(KStandardDirs::locate("data", "kdevphpsupport/phpfunctions.php"));
    return internalFile;
}

const IndexedString& internalTestFile()
{
    static const KDevelop::IndexedString phpUnitFile(KStandardDirs::locate("data", "kdevphpsupport/phpunitdeclarations.php"));
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
