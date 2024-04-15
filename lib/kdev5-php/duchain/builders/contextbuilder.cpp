/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contextbuilder.h"

#include <KLocalizedString>

#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "../editorintegrator.h"
#include "../helper.h"
#include "../duchaindebug.h"
#include "../phpducontext.h"

#include "../parser/parsesession.h"
#include "../parser/phpast.h"

using namespace KDevelop;

namespace Php
{

ContextBuilder::ContextBuilder()
    : m_isInternalFunctions(false), m_reportErrors(true),
      m_mapAst(false), m_hadUnresolvedIdentifiers(false),
      m_editor(0), m_openNamespaces(0)
{
}

ContextBuilder::~ContextBuilder()
{
}

EditorIntegrator* ContextBuilder::editor() const
{
    return m_editor;
}

ReferencedTopDUContext ContextBuilder::build(const IndexedString& url, AstNode* node,
                                             ReferencedTopDUContext updateContext)
{
    m_isInternalFunctions = url == internalFunctionFile();
    if ( m_isInternalFunctions ) {
        m_reportErrors = false;
    } else if ( ICore::self() ) {
        m_reportErrors = ICore::self()->languageController()->completionSettings()->highlightSemanticProblems();
    }

    if (!updateContext) {
        DUChainReadLocker lock(DUChain::lock());
        updateContext = DUChain::self()->chainForDocument(url);
    }
    if (updateContext) {
        qCDebug(DUCHAIN) << "re-compiling" << url.str();
        DUChainWriteLocker lock(DUChain::lock());
        updateContext->clearImportedParentContexts();
        updateContext->parsingEnvironmentFile()->clearModificationRevisions();
        updateContext->clearProblems();
        updateContext->updateImportsCache();
    } else {
        qCDebug(DUCHAIN) << "compiling" << url.str();
    }
    ReferencedTopDUContext top = ContextBuilderBase::build(url, node, updateContext);

    {
        DUChainWriteLocker lock(DUChain::lock());
        top->updateImportsCache();
    }

    return top;
}

bool ContextBuilder::hadUnresolvedIdentifiers() const
{
    return m_hadUnresolvedIdentifiers;
}


void ContextBuilder::startVisiting(AstNode* node)
{
    if (compilingContexts()) {
        TopDUContext* top = dynamic_cast<TopDUContext*>(currentContext());
        Q_ASSERT(top);
        {
            DUChainWriteLocker lock(DUChain::lock());
            top->updateImportsCache(); //Mark that we will use a cached import-structure
        }

        bool hasImports;
        {
            DUChainReadLocker lock(DUChain::lock());
            hasImports = !top->importedParentContexts().isEmpty();
        }
        if (!hasImports && top->url() != internalFunctionFile()) {
            DUChainWriteLocker lock(DUChain::lock());
            TopDUContext* import = DUChain::self()->chainForDocument(internalFunctionFile());
            if (!import) {
                qWarning() << "importing internalFunctions failed" << currentContext()->url().str();
                Q_ASSERT(false);
            } else {
                top->addImportedParentContext(import);
                top->updateImportsCache();
            }
        }

    }
    visitNode(node);
    if (m_openNamespaces) {
        closeNamespaces(m_openNamespaces);
        m_openNamespaces = 0;
    }
}

DUContext* ContextBuilder::newContext(const RangeInRevision& range)
{
    return new PhpDUContext<DUContext>(range, currentContext());
}

TopDUContext* ContextBuilder::newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file)
{
    if (!file) {
        file = new ParsingEnvironmentFile(m_editor->parseSession()->currentDocument());
        /// Indexed string for 'Php', identifies environment files from this language plugin
        static const IndexedString phpLangString("Php");
        file->setLanguage(phpLangString);
    }
    TopDUContext* ret = new PhpDUContext<TopDUContext>(m_editor->parseSession()->currentDocument(), range, file);
    ret->setType(DUContext::Global);
    return ret;
}

void ContextBuilder::setContextOnNode(AstNode* node, DUContext* ctx)
{
    node->ducontext = ctx;
}

DUContext* ContextBuilder::contextFromNode(AstNode* node)
{
    return node->ducontext;
}

RangeInRevision ContextBuilder::editorFindRange(AstNode* fromRange, AstNode* toRange)
{
    return m_editor->findRange(fromRange, toRange ? toRange : fromRange);
}

CursorInRevision ContextBuilder::startPos(AstNode* node)
{
    return m_editor->findPosition(node->startToken, EditorIntegrator::FrontEdge);
}

QualifiedIdentifier ContextBuilder::identifierForNode(IdentifierAst* id)
{
    if (!id)
        return QualifiedIdentifier();

    return QualifiedIdentifier(stringForNode(id));
}
QualifiedIdentifier ContextBuilder::identifierForNode(VariableIdentifierAst* id)
{
    if (!id)
        return QualifiedIdentifier();
    QString ret(stringForNode(id));
    ret = ret.mid(1); //cut off $
    return QualifiedIdentifier(ret);
}

IdentifierPair ContextBuilder::identifierPairForNode( IdentifierAst* id )
{
    if (!id) {
        return qMakePair(IndexedString(), QualifiedIdentifier());
    }
    const QString ret = stringForNode(id);

    return qMakePair(IndexedString(ret), QualifiedIdentifier(ret.toLower()));
}

QString ContextBuilder::stringForNode(IdentifierAst* node) const
{
    return m_editor->parseSession()->symbol(node->string);
}
QString ContextBuilder::stringForNode(VariableIdentifierAst* node) const
{
    return m_editor->parseSession()->symbol(node->variable);
}

void ContextBuilder::visitClassDeclarationStatement(ClassDeclarationStatementAst* node)
{
    openContext(node, editorFindRange(node, node), DUContext::Class, identifierPairForNode(node->className).second);
    classContextOpened(currentContext()); //This callback is needed, so we can set the internal context and so find the declaration for the context (before closeDeclaration())
    DefaultVisitor::visitClassDeclarationStatement(node);
    closeContext();
}

void ContextBuilder::classContextOpened(DUContext* context)
{
    Q_UNUSED(context);
}

void ContextBuilder::visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst* node)
{
    openContext(node, editorFindRange(node, node), DUContext::Class, identifierPairForNode(node->interfaceName).second);
    classContextOpened(currentContext()); //This callback is needed, so we can set the internal context and so find the declaration for the context (before closeDeclaration())
    DefaultVisitor::visitInterfaceDeclarationStatement(node);
    closeContext();
}

void ContextBuilder::visitTraitDeclarationStatement(TraitDeclarationStatementAst* node)
{
    openContext(node, editorFindRange(node, node), DUContext::Class, identifierPairForNode(node->traitName).second);
    classContextOpened(currentContext()); //This callback is needed, so we can set the internal context and so find the declaration for the context (before closeDeclaration())
    DefaultVisitor::visitTraitDeclarationStatement(node);
    closeContext();
}

void ContextBuilder::visitClassStatement(ClassStatementAst *node)
{
    visitOptionalModifiers(node->modifiers);
    if (node->methodName) {
        //method declaration
        DUContext* parameters = openContext(node->parameters, DUContext::Function, node->methodName);
        Q_ASSERT(!parameters->inSymbolTable());

        visitParameterList(node->parameters);
        closeContext();

        if ( !m_isInternalFunctions && node->methodBody ) {
            // the internal functions file has only empty method bodies, so skip them
            DUContext* body = openContext(node->methodBody, DUContext::Other, node->methodName);
            if (compilingContexts()) {
                DUChainWriteLocker lock(DUChain::lock());
                body->addImportedParentContext(parameters);
                body->setInSymbolTable(false);
            }
            visitMethodBody(node->methodBody);
            closeContext();
        }
    } else {
        //member-variable or const
        DefaultVisitor::visitClassStatement(node);
    }
}

void ContextBuilder::visitFunctionDeclarationStatement(FunctionDeclarationStatementAst* node)
{
    visitIdentifier(node->functionName);

    DUContext* parameters = openContext(node->parameters, DUContext::Function, node->functionName);
    Q_ASSERT(!parameters->inSymbolTable());

    visitParameterList(node->parameters);
    closeContext();

    if ( !m_isInternalFunctions && node->functionBody ) {
        // the internal functions file has only empty method bodies, so skip them
        DUContext* body = openContext(node->functionBody, DUContext::Other, node->functionName);
        if (compilingContexts()) {
            DUChainWriteLocker lock(DUChain::lock());
            body->addImportedParentContext(parameters);
            body->setInSymbolTable(false);
        }
        visitInnerStatementList(node->functionBody);
        closeContext();
    }
}

void ContextBuilder::visitClosure(ClosureAst* node)
{
    DUContext* parameters = openContext(node->parameters, DUContext::Function);
    Q_ASSERT(!parameters->inSymbolTable());

    visitParameterList(node->parameters);
    closeContext();

    DUContext* imported = nullptr;
    if ( node->lexicalVars ) {
        imported = openContext(node->lexicalVars, DUContext::Other);
        Q_ASSERT(!imported->inSymbolTable());

        visitLexicalVarList(node->lexicalVars);
        closeContext();
    }

    if ( !m_isInternalFunctions && node->functionBody ) {
        // the internal functions file has only empty method bodies, so skip them
        DUContext* body = openContext(node->functionBody, DUContext::Other);
        if (compilingContexts()) {
            DUChainWriteLocker lock;
            body->addImportedParentContext(parameters);
            if (imported) {
                body->addImportedParentContext(imported, CursorInRevision::invalid(), true);
            }
            body->setInSymbolTable(false);
        }
        visitInnerStatementList(node->functionBody);
        closeContext();
    }
}

void ContextBuilder::visitNamespaceDeclarationStatement(NamespaceDeclarationStatementAst* node)
{
    // close existing namespace context
    if (m_openNamespaces) {
        closeNamespaces(m_openNamespaces);
        m_openNamespaces = 0;
    }

    if ( !node->namespaceNameSequence ) {
        if (node->body) {
            // global namespace
            DefaultVisitor::visitInnerStatementList(node->body);
        }
        return;
    }

    { // open
    ///TODO: support \ as separator

    RangeInRevision bodyRange;
    if (node->body) {
        bodyRange = editorFindRange(node->body, node->body);
    } else {
        bodyRange = RangeInRevision(m_editor->findPosition(node->endToken), currentContext()->topContext()->range().end);
    }
    const KDevPG::ListNode< IdentifierAst* >* it = node->namespaceNameSequence->front();
    do {
        openNamespace(node, it->element, identifierPairForNode(it->element), bodyRange);
    } while(it->hasNext() && (it = it->next));
    }

    if (node->body) {
        DefaultVisitor::visitInnerStatementList(node->body);
        closeNamespaces(node);
    } else {
        m_openNamespaces = node;
    }
}

void ContextBuilder::closeNamespaces(NamespaceDeclarationStatementAst* namespaces)
{
    ///TODO: support \ as separator
    const KDevPG::ListNode< IdentifierAst* >* it = namespaces->namespaceNameSequence->front();
    do {
        Q_ASSERT(currentContext()->type() == DUContext::Namespace);
        closeNamespace(namespaces, it->element, identifierPairForNode(it->element));
    } while(it->hasNext() && (it = it->next));
}

void ContextBuilder::openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const RangeInRevision& range)
{
    if ( node == parent->namespaceNameSequence->back()->element ) {
        openContext(node, range, DUContext::Namespace, identifier.second);
    } else {
        openContext(node, range, DUContext::Namespace, identifier.second);
    }
}

void ContextBuilder::closeNamespace(NamespaceDeclarationStatementAst* /*parent*/, IdentifierAst* /*node*/, const IdentifierPair& /*identifier*/)
{
    closeContext();
}

void ContextBuilder::addBaseType(NamespacedIdentifierAst * identifier)
{
    DUChainWriteLocker lock(DUChain::lock());

    Q_ASSERT(currentContext()->type() == DUContext::Class);

    ClassDeclaration* currentClass = dynamic_cast<ClassDeclaration*>(currentContext()->owner());

    ClassDeclaration* baseClass = dynamic_cast<ClassDeclaration*>(
        findDeclarationImport(ClassDeclarationType, identifierForNamespace(identifier, m_editor)).data() );

    if (currentClass && baseClass) {
        if (DUContext* baseContext = baseClass->logicalInternalContext(0)) {
            // prevent circular context imports which could lead to segfaults
            if (!baseContext->imports(currentContext()) && !currentContext()->imports(baseContext)) {
                currentContext()->addImportedParentContext(baseContext);
                BaseClassInstance base;
                base.baseClass = baseClass->indexedType();
                base.access = Declaration::Public;
                base.virtualInheritance = false;
                currentClass->addBaseClass(base);
            } else if (m_reportErrors) {
                reportError(i18n("Circular inheritance of %1 and %2", currentClass->toString(), baseClass->toString()), identifier);
            }
        }
    }
    if (!baseClass) {
        qCDebug(DUCHAIN) << "unresolved identifier";
        m_hadUnresolvedIdentifiers = true;
    }
}


void ContextBuilder::visitUnaryExpression(UnaryExpressionAst* node)
{
    DefaultVisitor::visitUnaryExpression(node);
    if (!compilingContexts()) {
        return;
    }
    IndexedString includeFile = getIncludeFileForNode(node, m_editor);

    if ( !includeFile.isEmpty() ) {
        DUChainWriteLocker lock(DUChain::lock());
        TopDUContext *top = DUChain::self()->chainForDocument(includeFile);
        if (top) {
            currentContext()->topContext()->addImportedParentContext(top);
            currentContext()->topContext()->parsingEnvironmentFile()
            ->addModificationRevisions(top->parsingEnvironmentFile()->allModificationRevisions());
        }
    }
}

void ContextBuilder::reportError(const QString& errorMsg, AstNode* node, IProblem::Severity severity)
{
    reportError(errorMsg, m_editor->findRange(node), severity);
}

void ContextBuilder::reportError(const QString& errorMsg, QList< AstNode* > nodes, IProblem::Severity severity)
{
    RangeInRevision range = RangeInRevision::invalid();
    for( AstNode* node : nodes ) {
        if ( !range.isValid() ) {
            range = m_editor->findRange(node);
        } else {
            range.end = m_editor->findPosition(node->endToken);
        }
    }
    reportError(errorMsg, range, severity);
}

void ContextBuilder::reportError(const QString& errorMsg, RangeInRevision range, IProblem::Severity severity)
{
    Problem *p = new Problem();
    p->setSeverity(severity);
    p->setSource(IProblem::DUChainBuilder);
    p->setDescription(errorMsg);
    p->setFinalLocation(DocumentRange(m_editor->parseSession()->currentDocument(),
                                                range.castToSimpleRange()));
    {
        DUChainWriteLocker lock(DUChain::lock());
        qCDebug(DUCHAIN) << "Problem" << p->description() << p->finalLocation();
        currentContext()->topContext()->addProblem(ProblemPointer(p));
    }
}

DeclarationPointer ContextBuilder::findDeclarationImport(DeclarationType declarationType,
                                                         IdentifierAst* node)
{
    QualifiedIdentifier id;
    if ( declarationType == ClassDeclarationType || declarationType == FunctionDeclarationType ) {
        id = identifierPairForNode(node).second;
    } else {
        id = identifierForNode(node);
    }
    return findDeclarationImportHelper(currentContext(), id, declarationType);
}

DeclarationPointer ContextBuilder::findDeclarationImport(DeclarationType declarationType,
                                                         VariableIdentifierAst* node)
{
    return findDeclarationImportHelper(currentContext(), identifierForNode(node), declarationType);
}

DeclarationPointer ContextBuilder::findDeclarationImport(DeclarationType declarationType,
                                                         const QualifiedIdentifier &identifier)
{
    return findDeclarationImportHelper(currentContext(), identifier, declarationType);
}

}
