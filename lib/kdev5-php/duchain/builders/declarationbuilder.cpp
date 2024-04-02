/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "declarationbuilder.h"

#include <KLocalizedString>

#include <language/duchain/stringhelpers.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/unsuretype.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <util/pushvalue.h>


#include "../declarations/variabledeclaration.h"
#include "../declarations/classmethoddeclaration.h"
#include "../declarations/classdeclaration.h"
#include "../declarations/functiondeclaration.h"
#include "../declarations/namespacedeclaration.h"
#include "../declarations/namespacealiasdeclaration.h"
#include "../declarations/traitmethodaliasdeclaration.h"
#include "../declarations/traitmemberaliasdeclaration.h"

#include "../parser/phpast.h"
#include "../parser/parsesession.h"

#include "../helper.h"
#include "../duchaindebug.h"
#include "../expressionvisitor.h"

#include "predeclarationbuilder.h"

#define ifDebug(x)

using namespace KDevelop;

namespace Php
{

DeclarationBuilder::FindVariableResults::FindVariableResults()
: find(true)
, isArray(false)
, node(0)
{

}

void DeclarationBuilder::getVariableIdentifier(VariableAst* node,
                                                QualifiedIdentifier &identifier,
                                                QualifiedIdentifier &parent,
                                                AstNode* &targetNode,
                                                bool &arrayAccess)
{
    parent = QualifiedIdentifier();
    if ( node->variablePropertiesSequence ) {
        // at least one "->" in the assignment target
        // => find he parent of the target
        // => find the target (last object property)
        if ( node->variablePropertiesSequence->count() == 1 ) {
            // $parent->target
            ///TODO: $parent[0]->target = ... (we don't know the type of [0] yet, need proper array handling first)
            if ( node->var && node->var->baseVariable && node->var->baseVariable->var
                && !node->var->baseVariable->offsetItemsSequence ) {
                parent = identifierForNode(
                    node->var->baseVariable->var->variable
                );
            }
        } else {
            // $var->...->parent->target
            ///TODO: $var->...->parent[0]->target = ... (we don't know the type of [0] yet, need proper array handling first)
            const KDevPG::ListNode< VariableObjectPropertyAst* >* parentNode = node->variablePropertiesSequence->at(
                node->variablePropertiesSequence->count() - 2
            );
            if ( parentNode->element && parentNode->element->variableProperty
                && parentNode->element->variableProperty->objectProperty
                && parentNode->element->variableProperty->objectProperty->objectDimList
                && parentNode->element->variableProperty->objectProperty->objectDimList->variableName
                && !parentNode->element->variableProperty->objectProperty->objectDimList->offsetItemsSequence ) {
                parent = identifierForNode(
                    parentNode->element->variableProperty->objectProperty->objectDimList->variableName->name
                );
            }
        }

        if ( !parent.isEmpty() ) {
            const KDevPG::ListNode< VariableObjectPropertyAst* >* tNode = node->variablePropertiesSequence->at(
                node->variablePropertiesSequence->count() - 1
            );
            if ( tNode->element && tNode->element->variableProperty
                && tNode->element->variableProperty->objectProperty
                && tNode->element->variableProperty->objectProperty->objectDimList
                && tNode->element->variableProperty->objectProperty->objectDimList->variableName ) {
                arrayAccess = (bool) tNode->element->variableProperty->objectProperty->objectDimList->offsetItemsSequence;
                identifier = identifierForNode(
                    tNode->element->variableProperty->objectProperty->objectDimList->variableName->name
                );
                targetNode = tNode->element->variableProperty->objectProperty->objectDimList->variableName->name;
            }
        }
    } else {
        // simple assignment to $var
        if ( node->var && node->var->baseVariable && node->var->baseVariable->var ) {
            arrayAccess = (bool) node->var->baseVariable->offsetItemsSequence;
            identifier = identifierForNode(
                node->var->baseVariable->var->variable
            );
            targetNode = node->var->baseVariable->var->variable;
        }
    }
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url, AstNode* node,
        ReferencedTopDUContext updateContext)
{
    //Run DeclarationBuilder twice, to find uses of declarations that are
    //declared after the use. ($a = new Foo; class Foo {})
    {
        PreDeclarationBuilder prebuilder(&m_types, &m_functions, &m_namespaces,
                                         &m_upcomingClassVariables, m_editor);
        updateContext = prebuilder.build(url, node, updateContext);
        m_actuallyRecompiling = prebuilder.didRecompile();
    }

    // now skip through some things the DeclarationBuilderBase (ContextBuilder) would do,
    // most significantly don't clear imported parent contexts
    m_isInternalFunctions = url == internalFunctionFile();
    if ( m_isInternalFunctions ) {
        m_reportErrors = false;
    } else if ( ICore::self() ) {
        m_reportErrors = ICore::self()->languageController()->completionSettings()->highlightSemanticProblems();
    }

    return ContextBuilderBase::build(url, node, updateContext);
}

void DeclarationBuilder::startVisiting(AstNode* node)
{
    setRecompiling(m_actuallyRecompiling);
    setCompilingContexts(false);
    DeclarationBuilderBase::startVisiting(node);
}

void DeclarationBuilder::closeDeclaration()
{
    if (currentDeclaration() && lastType()) {
        DUChainWriteLocker lock(DUChain::lock());
        currentDeclaration()->setType(lastType());
    }

    eventuallyAssignInternalContext();

    DeclarationBuilderBase::closeDeclaration();
}

void DeclarationBuilder::classContextOpened(DUContext* context)
{
    DUChainWriteLocker lock(DUChain::lock());
    currentDeclaration()->setInternalContext(context);
}

void DeclarationBuilder::visitClassDeclarationStatement(ClassDeclarationStatementAst * node)
{
    ClassDeclaration* classDec = openTypeDeclaration(node->className, ClassDeclarationData::Class);
    openType(classDec->abstractType());
    DeclarationBuilderBase::visitClassDeclarationStatement(node);
    {
        DUChainWriteLocker lock;
        classDec->updateCompletionCodeModelItem();
    }
    closeType();
    closeDeclaration();
    m_upcomingClassVariables.clear();
}

void DeclarationBuilder::visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst *node)
{
    ClassDeclaration* interfaceDec = openTypeDeclaration(node->interfaceName, ClassDeclarationData::Interface);
    openType(interfaceDec->abstractType());
    DeclarationBuilderBase::visitInterfaceDeclarationStatement(node);
    closeType();
    closeDeclaration();
}

void DeclarationBuilder::visitTraitDeclarationStatement(TraitDeclarationStatementAst * node)
{
    ClassDeclaration* traitDec = openTypeDeclaration(node->traitName, ClassDeclarationData::Trait);
    openType(traitDec->abstractType());
    DeclarationBuilderBase::visitTraitDeclarationStatement(node);
    closeType();
    closeDeclaration();
    m_upcomingClassVariables.clear();
}

ClassDeclaration* DeclarationBuilder::openTypeDeclaration(IdentifierAst* name, ClassDeclarationData::ClassType type)
{
    ClassDeclaration* classDec = m_types.value(name->string, 0);
    Q_ASSERT(classDec);
    isGlobalRedeclaration(identifierForNode(name), name, ClassDeclarationType);
    Q_ASSERT(classDec->classType() == type);
    Q_UNUSED(type);

    // seems like we have to do that manually, else the usebuilder crashes...
    setEncountered(classDec);
    openDeclarationInternal(classDec);

    return classDec;
}

bool DeclarationBuilder::isBaseMethodRedeclaration(const IdentifierPair &ids, ClassDeclaration *curClass,
        ClassStatementAst *node)
{
    DUChainWriteLocker lock(DUChain::lock());
    while (curClass->baseClassesSize() > 0) {
        StructureType::Ptr type;
        FOREACH_FUNCTION(const BaseClassInstance& base, curClass->baseClasses) {
            DUChainReadLocker lock(DUChain::lock());
            type = base.baseClass.type<StructureType>();
            if (!type) {
                continue;
            }
            ClassDeclaration *nextClass = dynamic_cast<ClassDeclaration*>(type->declaration(currentContext()->topContext()));
            if (!nextClass || nextClass->classType() != ClassDeclarationData::Class) {
                type.reset();
                continue;
            }
            curClass = nextClass;
            break;
        }
        if (!type) {
            break;
        }
        {
            if (!type->internalContext(currentContext()->topContext())) {
                continue;
            }
            Q_FOREACH(Declaration * dec, type->internalContext(currentContext()->topContext())->findLocalDeclarations(ids.second.first(), startPos(node)))
            {
                if (dec->isFunctionDeclaration()) {
                    ClassMethodDeclaration* func = dynamic_cast<ClassMethodDeclaration*>(dec);
                    if (!func || !wasEncountered(func)) {
                        continue;
                    }
                    // we cannot redeclare final classes ever
                    if (func->isFinal()) {
                        reportRedeclarationError(dec, node->methodName);
                        return true;
                    }
                    // also we may not redeclare an already abstract method, we would have to implement it
                    // TODO: original error message?
                    // -> Can't inherit abstract function class::func() (previously declared in otherclass)
                    else if (func->isAbstract() && node->modifiers->modifiers & ModifierAbstract) {
                        reportRedeclarationError(dec, node->methodName);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void DeclarationBuilder::visitClassStatement(ClassStatementAst *node)
{
    setComment(formatComment(node, m_editor));

    ClassDeclaration *parent =  dynamic_cast<ClassDeclaration*>(currentDeclaration());
    Q_ASSERT(parent);

    if (node->methodName) {
        //method declaration

        IdentifierPair ids = identifierPairForNode(node->methodName);
        if (m_reportErrors) {   // check for redeclarations
            Q_ASSERT(currentContext()->type() == DUContext::Class);
            bool localError = false;
            {
                DUChainWriteLocker lock(DUChain::lock());
                Q_FOREACH(Declaration * dec, currentContext()->findLocalDeclarations(ids.second.first(), startPos(node->methodName)))
                {
                    if (wasEncountered(dec) && dec->isFunctionDeclaration() && !dynamic_cast<TraitMethodAliasDeclaration*>(dec)) {
                        reportRedeclarationError(dec, node->methodName);
                        localError = true;
                        break;
                    }
                }
            }

            if (!localError) {
                // if we have no local error, check that we don't try to overwrite a final method of a baseclass
                isBaseMethodRedeclaration(ids, parent, node);
            }
        }

        {
            DUChainWriteLocker lock(DUChain::lock());
            ClassMethodDeclaration* dec = openDefinition<ClassMethodDeclaration>(ids.second, editorFindRange(node->methodName, node->methodName));
            dec->setPrettyName(ids.first);
            dec->clearDefaultParameters();
            dec->setKind(Declaration::Type);
            if (node->modifiers->modifiers & ModifierPublic) {
                dec->setAccessPolicy(Declaration::Public);
            } else if (node->modifiers->modifiers & ModifierProtected) {
                dec->setAccessPolicy(Declaration::Protected);
            } else if (node->modifiers->modifiers & ModifierPrivate) {
                dec->setAccessPolicy(Declaration::Private);
            }
            if (node->modifiers->modifiers & ModifierStatic) {
                dec->setStatic(true);
            }
            if (parent->classType() == ClassDeclarationData::Interface) {
                if (m_reportErrors) {
                    if (node->modifiers->modifiers & ModifierFinal || node->modifiers->modifiers & ModifierAbstract) {
                        reportError(i18n("Access type for interface method %1 must be omitted.",
                                         dec->toString()), node->modifiers);
                    }
                    if (!isEmptyMethodBody(node->methodBody)) {
                        reportError(i18n("Interface function %1 cannot contain body.",
                                         dec->toString()), node->methodBody);
                    }
                }
                // handle interface methods like abstract methods
                dec->setIsAbstract(true);
            } else {
                if (node->modifiers->modifiers & ModifierAbstract) {
                    if (!m_reportErrors) {
                        dec->setIsAbstract(true);
                    } else {
                        if (parent->classModifier() != ClassDeclarationData::Abstract && parent->classType() != ClassDeclarationData::Trait) {
                            reportError(i18n("Class %1 contains abstract method %2 and must therefore be declared abstract "
                                             "or implement the method.",
                                             parent->identifier().toString(),
                                             dec->identifier().toString()),
                                        node->modifiers);
                        } else if (!isEmptyMethodBody(node->methodBody)) {
                            reportError(i18n("Abstract function %1 cannot contain body.",
                                             dec->toString()), node->methodBody);
                        } else if (node->modifiers->modifiers & ModifierFinal) {
                            reportError(i18n("Cannot use the final modifier on an abstract class member."),
                                        node->modifiers);
                        } else {
                            dec->setIsAbstract(true);
                        }
                    }
                } else if (node->modifiers->modifiers & ModifierFinal) {
                    dec->setIsFinal(true);
                }
                if (m_reportErrors && !dec->isAbstract() && isEmptyMethodBody(node->methodBody)) {
                    reportError(i18n("Non-abstract method %1 must contain body.", dec->toString()), node->methodBody);
                }
            }
        }

        DeclarationBuilderBase::visitClassStatement(node);

        closeDeclaration();
    } else if (node->traitsSequence) {
        DeclarationBuilderBase::visitClassStatement(node);

        importTraitMethods(node);
    } else {
        if (node->modifiers) {
            m_currentModifers = node->modifiers->modifiers;
            if (m_reportErrors) {
                // have to report the errors here to get a good problem range
                if (m_currentModifers & ModifierFinal) {
                    reportError(i18n("Properties cannot be declared final."), node->modifiers);
                }
                if (m_currentModifers & ModifierAbstract) {
                    reportError(i18n("Properties cannot be declared abstract."), node->modifiers);
                }
            }
        } else {
            m_currentModifers = 0;
        }
        DeclarationBuilderBase::visitClassStatement(node);
        m_currentModifers = 0;
    }
}

void DeclarationBuilder::importTraitMethods(ClassStatementAst *node)
{
    // Add trait members that don't need special handling
    const KDevPG::ListNode< NamespacedIdentifierAst* >* it = node->traitsSequence->front();
    DUChainWriteLocker lock;
    forever {
        DeclarationPointer dec =  findDeclarationImport(ClassDeclarationType, identifierForNamespace(it->element, m_editor));

        if (!dec || !dec->internalContext()) {
            break;
        }

        QVector <Declaration*> declarations = dec.data()->internalContext()->localDeclarations(0);
        QVector <Declaration*> localDeclarations = currentContext()->localDeclarations(0);

        ifDebug(qCDebug(DUCHAIN) << "Importing from" << dec.data()->identifier().toString() << "to" << currentContext()->localScopeIdentifier().toString();)

        Q_FOREACH(Declaration* import, declarations) {
            Declaration* found = nullptr;
            Q_FOREACH(Declaration* local, localDeclarations) {
                ifDebug(qCDebug(DUCHAIN) << "Comparing" << import->identifier().toString() << "with" << local->identifier().toString();)
                if (auto trait = dynamic_cast<TraitMethodAliasDeclaration*>(local)) {
                    if (trait->aliasedDeclaration().data() == import) {
                        ifDebug(qCDebug(DUCHAIN) << "Already imported";)
                        found = local;
                        break;
                    }
                    if (local->identifier() == import->identifier()) {
                        ClassMethodDeclaration* importMethod = dynamic_cast<ClassMethodDeclaration*>(import);
                        if (trait->isOverriding(import->context()->indexedLocalScopeIdentifier())) {
                            ifDebug(qCDebug(DUCHAIN) << "Is overridden";)
                            found = local;
                            break;
                        } else if (importMethod) {
                            reportError(
                                i18n("Trait method %1 has not been applied, because there are collisions with other trait methods on %2")
                                .arg(importMethod->prettyName().str(),
                                     dynamic_cast<ClassDeclaration*>(currentDeclaration())->prettyName().str())
                                , it->element, IProblem::Error
                            );
                            found = local;
                            break;
                        }
                    }
                }
                if (auto trait = dynamic_cast<TraitMemberAliasDeclaration*>(local)) {
                    if (trait->aliasedDeclaration().data() == import) {
                        ifDebug(qCDebug(DUCHAIN) << "Already imported";)
                        found = local;
                        break;
                    }
                }
                if (local->identifier() == import->identifier()) {
                    if (dynamic_cast<ClassMemberDeclaration*>(local) && dynamic_cast<ClassMemberDeclaration*>(import)) {
                        found = local;
                        break;
                    }
                }
            }

            if (found) {
                setEncountered(found);
                continue;
            }

            ifDebug(qCDebug(DUCHAIN) << "Importing new declaration";)

            CursorInRevision cursor = m_editor->findRange(it->element).start;

            if (auto olddec = dynamic_cast<const ClassMethodDeclaration*>(import)) {
                TraitMethodAliasDeclaration* newdec = openDefinition<TraitMethodAliasDeclaration>(olddec->qualifiedIdentifier(), RangeInRevision(cursor, cursor));
                openAbstractType(olddec->abstractType());
                newdec->setPrettyName(olddec->prettyName());
                newdec->setAccessPolicy(olddec->accessPolicy());
                newdec->setKind(Declaration::Type);
                newdec->setAliasedDeclaration(IndexedDeclaration(olddec));
                newdec->setStatic(olddec->isStatic());
                closeType();
                closeDeclaration();
            } else if (auto olddec = dynamic_cast<const ClassMemberDeclaration*>(import)) {
                TraitMemberAliasDeclaration* newdec = openDefinition<TraitMemberAliasDeclaration>(olddec->qualifiedIdentifier(), RangeInRevision(cursor, cursor));
                openAbstractType(olddec->abstractType());
                newdec->setAccessPolicy(olddec->accessPolicy());
                newdec->setKind(Declaration::Instance);
                newdec->setAliasedDeclaration(IndexedDeclaration(olddec));
                newdec->setStatic(olddec->isStatic());
                closeType();
                closeDeclaration();
            }

        }

        if ( it->hasNext() ) {
            it = it->next;
        } else {
            break;
        }
    }
}

void DeclarationBuilder::visitClassExtends(ClassExtendsAst *node)
{
    addBaseType(node->identifier);
}

void DeclarationBuilder::visitClassImplements(ClassImplementsAst *node)
{
    const KDevPG::ListNode<NamespacedIdentifierAst*> *__it = node->implementsSequence->front(), *__end = __it;
    do {
        addBaseType(__it->element);
        __it = __it->next;
    } while (__it != __end);
    DeclarationBuilderBase::visitClassImplements(node);
}

void DeclarationBuilder::visitClassVariable(ClassVariableAst *node)
{
    QualifiedIdentifier name = identifierForNode(node->variable);
    if (m_reportErrors) {   // check for redeclarations
        DUChainWriteLocker lock(DUChain::lock());
        Q_ASSERT(currentContext()->type() == DUContext::Class);
        Q_FOREACH(Declaration * dec, currentContext()->findLocalDeclarations(name.first(), startPos(node)))
        {
            if (wasEncountered(dec) && !dec->isFunctionDeclaration() && !(dec->abstractType()->modifiers() & AbstractType::ConstModifier)) {
                reportRedeclarationError(dec, node);
                break;
            }
        }
    }
    openClassMemberDeclaration(node->variable, name);
    DeclarationBuilderBase::visitClassVariable(node);
    closeDeclaration();
}

void DeclarationBuilder::openClassMemberDeclaration(AstNode* node, const QualifiedIdentifier &name)
{
    DUChainWriteLocker lock(DUChain::lock());

    // dirty hack: declarations of class members outside the class context would
    //             make the class context encompass the newRange. This is not what we want.
    RangeInRevision oldRange = currentContext()->range();

    RangeInRevision newRange = editorFindRange(node, node);
    openDefinition<ClassMemberDeclaration>(name, newRange);

    ClassMemberDeclaration* dec = dynamic_cast<ClassMemberDeclaration*>(currentDeclaration());
    Q_ASSERT(dec);
    if (m_currentModifers & ModifierPublic) {
        dec->setAccessPolicy(Declaration::Public);
    } else if (m_currentModifers & ModifierProtected) {
        dec->setAccessPolicy(Declaration::Protected);
    } else if (m_currentModifers & ModifierPrivate) {
        dec->setAccessPolicy(Declaration::Private);
    }
    if (m_currentModifers & ModifierStatic) {
        dec->setStatic(true);
    }
    dec->setKind(Declaration::Instance);

    currentContext()->setRange(oldRange);
}

void DeclarationBuilder::declareClassMember(DUContext *parentCtx, AbstractType::Ptr type,
                                                const QualifiedIdentifier& identifier,
                                                AstNode* node )
{
    if ( m_upcomingClassVariables.contains(identifier) ) {
        if (m_actuallyRecompiling) {
            DUChainWriteLocker lock;
            if (Declaration* dec = currentContext()->findDeclarationAt(startPos(node))) {
                if (dynamic_cast<ClassMemberDeclaration*>(dec)) {
                    // invalidate declaration, it got added
                    // see also bug https://bugs.kde.org/show_bug.cgi?id=241750
                    delete dec;
                }
            }
        }
        return;
    }

    DUChainWriteLocker lock(DUChain::lock());

    // check for redeclaration of private or protected stuff
    {
        // only interesting context might be the class context when we are inside a method
        DUContext *ctx = currentContext()->parentContext();
        Q_FOREACH( Declaration* dec, parentCtx->findDeclarations(identifier) ) {
            if ( ClassMemberDeclaration* cdec = dynamic_cast<ClassMemberDeclaration*>(dec) ) {
                if ( cdec->accessPolicy() == Declaration::Private && cdec->context() != ctx ) {
                    reportError(i18n("Cannot redeclare private property %1 from this context.",
                                        cdec->toString()), node);
                    return;
                } else if ( cdec->accessPolicy() == Declaration::Protected
                            && cdec->context() != ctx
                            && ( !ctx || !ctx->imports(cdec->context()) ) ) {
                    reportError(i18n("Cannot redeclare protected property %1 from this context.",
                                        cdec->toString()), node);
                    return;
                }
                if ( cdec->abstractType()->indexed() == type->indexed() ) {
                    encounter(dec);
                    return;
                }
            }
        }
    }

    // this member should be public and non-static
    m_currentModifers = ModifierPublic;
    injectContext(parentCtx);
    openClassMemberDeclaration(node, identifier);
    m_currentModifers = 0;
    //own closeDeclaration() that doesn't use lastType()
    currentDeclaration()->setType(type);
    eventuallyAssignInternalContext();
    DeclarationBuilderBase::closeDeclaration();
    closeInjectedContext();
}

void DeclarationBuilder::visitConstantDeclaration(ConstantDeclarationAst *node)
{
    if (m_reportErrors) {
        // Check for constants in traits
        if (isMatch(currentDeclaration(), ClassDeclarationType)) {
            ClassDeclaration *parent =  dynamic_cast<ClassDeclaration*>(currentDeclaration());
            Q_ASSERT(parent);

            if (parent->classType() == ClassDeclarationData::Trait) {
                reportError(i18n("Traits cannot have constants."), node);
            }
        }

        // check for redeclarations
        DUChainWriteLocker lock(DUChain::lock());
        Q_FOREACH(Declaration * dec, currentContext()->findLocalDeclarations(identifierForNode(node->identifier).first(), startPos(node->identifier)))
        {
            if (wasEncountered(dec) && !dec->isFunctionDeclaration() && dec->abstractType()->modifiers() & AbstractType::ConstModifier) {
                reportRedeclarationError(dec, node->identifier);
                break;
            }
        }
    }
    ClassMemberDeclaration* dec = openDefinition<ClassMemberDeclaration>(node->identifier, node->identifier);
    {
        DUChainWriteLocker lock(DUChain::lock());
        dec->setAccessPolicy(Declaration::Public);
        dec->setStatic(true);
        dec->setKind(Declaration::Instance);
    }
    DeclarationBuilderBase::visitConstantDeclaration(node);
    closeDeclaration();
    if ( m_reportErrors ) {
        // const class members may only be ints, floats, bools or strings
        bool badType = true;
        if ( IntegralType* type = fastCast<IntegralType*>(lastType().data()) ) {
            switch( type->dataType() ) {
                case IntegralType::TypeBoolean:
                case IntegralType::TypeFloat:
                case IntegralType::TypeInt:
                case IntegralType::TypeString:
                case IntegralType::TypeNull:
                    badType = false;
                    break;
                default:
                    // every other type is a badType (see above)
                    break;
            }
        }
        if ( badType ) {
            reportError(i18n("Only booleans, ints, floats and strings are allowed for class constants."), node->scalar);
        }
    }
}

void DeclarationBuilder::visitTraitAliasStatement(TraitAliasStatementAst *node)
{
    DUChainWriteLocker lock;

    DeclarationPointer dec = findDeclarationImport(ClassDeclarationType, identifierForNamespace(node->importIdentifier->identifier, m_editor));

    if (dec && dec.data()->internalContext()) {
        createTraitAliasDeclarations(node, dec);
    }

    lock.unlock();

    DeclarationBuilderBase::visitTraitAliasStatement(node);
}

void DeclarationBuilder::createTraitAliasDeclarations(TraitAliasStatementAst *node, DeclarationPointer dec)
{
    QualifiedIdentifier original = identifierPairForNode(node->importIdentifier->methodIdentifier).second;
    QList <Declaration*> list = dec.data()->internalContext()->findLocalDeclarations(original.last(), dec.data()->internalContext()->range().start);

    QualifiedIdentifier alias;
    if (node->aliasIdentifier) {
        alias = identifierPairForNode(node->aliasIdentifier).second;
    } else {
        alias = original;
    }

    if (!list.isEmpty()) {
        ClassMethodDeclaration* olddec = dynamic_cast<ClassMethodDeclaration*>(list.first());
        TraitMethodAliasDeclaration* newdec;

        // no existing declaration found, create one
        if (node->aliasIdentifier) {
            newdec = openDefinition<TraitMethodAliasDeclaration>(alias, m_editor->findRange(node->aliasIdentifier));
            newdec->setPrettyName(identifierPairForNode(node->aliasIdentifier).first);
            newdec->setAccessPolicy(olddec->accessPolicy());
            openAbstractType(olddec->abstractType());
            if (node->modifiers) {
                if (node->modifiers->modifiers & ModifierPublic) {
                    newdec->setAccessPolicy(Declaration::Public);
                } else if (node->modifiers->modifiers & ModifierProtected) {
                    newdec->setAccessPolicy(Declaration::Protected);
                } else if (node->modifiers->modifiers & ModifierPrivate) {
                    newdec->setAccessPolicy(Declaration::Private);
                }

                if (node->modifiers->modifiers & ModifierFinal) {
                    reportError(i18n("Cannot use 'final' as method modifier"), node->modifiers, IProblem::Error);
                }
                if (node->modifiers->modifiers & ModifierStatic) {
                    reportError(i18n("Cannot use 'static' as method modifier"), node->modifiers, IProblem::Error);
                }

            }
        } else {
            CursorInRevision cursor = m_editor->findRange(node->importIdentifier).start;
            newdec = openDefinition<TraitMethodAliasDeclaration>(alias, RangeInRevision(cursor, cursor));
            newdec->setPrettyName(identifierPairForNode(node->importIdentifier->methodIdentifier).first);
            newdec->setAccessPolicy(olddec->accessPolicy());
            openAbstractType(olddec->abstractType());
        }
        newdec->setKind(Declaration::Type);
        newdec->setAliasedDeclaration(IndexedDeclaration(olddec));
        newdec->setStatic(olddec->isStatic());

        QVector <IndexedQualifiedIdentifier> ids;

        if (node->conflictIdentifierSequence) {
            const KDevPG::ListNode< NamespacedIdentifierAst* >* it = node->conflictIdentifierSequence->front();
            forever {
                DeclarationPointer dec =  findDeclarationImport(ClassDeclarationType, identifierForNamespace(it->element, m_editor));
                if (dec) {
                    ids.append(IndexedQualifiedIdentifier(dec.data()->qualifiedIdentifier()));
                }

                if ( it->hasNext() ) {
                    it = it->next;
                } else {
                    break;
                }
            }

            newdec->setOverrides(ids);
        }

        closeType();
        closeDeclaration();
    }
}

void DeclarationBuilder::visitParameter(ParameterAst *node)
{
    AbstractFunctionDeclaration* funDec = dynamic_cast<AbstractFunctionDeclaration*>(currentDeclaration());
    Q_ASSERT(funDec);
    if (node->defaultValue) {
        QString symbol = m_editor->parseSession()->symbol(node->defaultValue);
        funDec->addDefaultParameter(IndexedString(symbol));
        if ( node->parameterType && symbol.compare(QLatin1String("null"), Qt::CaseInsensitive) != 0 ) {
            reportError(i18n("Default value for parameters with a class type hint can only be NULL."), node->defaultValue);
        }
    } else if ( !node->defaultValue && funDec->defaultParametersSize() ) {
        reportError(i18n("Following parameters must have a default value assigned."), node);
    }
    {
        // create variable declaration for argument
        DUChainWriteLocker lock(DUChain::lock());
        RangeInRevision newRange = editorFindRange(node->variable, node->variable);
        openDefinition<VariableDeclaration>(identifierForNode(node->variable), newRange);
        currentDeclaration()->setKind(Declaration::Instance);
    }

    DeclarationBuilderBase::visitParameter(node);
    closeDeclaration();
}

void DeclarationBuilder::visitFunctionDeclarationStatement(FunctionDeclarationStatementAst* node)
{
    isGlobalRedeclaration(identifierForNode(node->functionName), node->functionName, FunctionDeclarationType);

    FunctionDeclaration* dec = m_functions.value(node->functionName->string, 0);
    Q_ASSERT(dec);
    // seems like we have to set that, else the usebuilder crashes

    DeclarationBuilderBase::setEncountered(dec);

    openDeclarationInternal(dec);
    openType(dec->abstractType());

    DeclarationBuilderBase::visitFunctionDeclarationStatement(node);

    closeType();
    closeDeclaration();
}
void DeclarationBuilder::visitClosure(ClosureAst* node)
{
    setComment(formatComment(node, editor()));
    {
        DUChainWriteLocker lock;
        FunctionDeclaration *dec = openDefinition<FunctionDeclaration>(QualifiedIdentifier(),
                                                                       editor()->findRange(node->startToken));
        dec->setKind(Declaration::Type);
        dec->clearDefaultParameters();
    }

    DeclarationBuilderBase::visitClosure(node);

    closeDeclaration();
}
void DeclarationBuilder::visitLexicalVar(LexicalVarAst* node)
{
    DeclarationBuilderBase::visitLexicalVar(node);

    QualifiedIdentifier id = identifierForNode(node->variable);
    DUChainWriteLocker lock;
    if ( recompiling() ) {
        // sadly we can't use findLocalDeclarations() here, since it un-aliases declarations
        Q_FOREACH( Declaration* dec, currentContext()->localDeclarations() ) {
            if ( dynamic_cast<AliasDeclaration*>(dec) && dec->identifier() == id.first() ) {
                // don't redeclare but reuse the existing declaration
                encounter(dec);
                return;
            }
        }
    }

    // no existing declaration found, create one
    Q_FOREACH(Declaration* aliasedDeclaration, currentContext()->findDeclarations(id)) {
        if (aliasedDeclaration->kind() == Declaration::Instance) {
            AliasDeclaration* dec = openDefinition<AliasDeclaration>(id, editor()->findRange(node->variable));
            dec->setAliasedDeclaration(aliasedDeclaration);
            closeDeclaration();
            break;
        }
    }
}

bool DeclarationBuilder::isGlobalRedeclaration(const QualifiedIdentifier &identifier, AstNode* node,
        DeclarationType type)
{
    if (!m_reportErrors) {
        return false;
    }
    ///TODO: method redeclaration etc.
    if (type != ClassDeclarationType
            && type != FunctionDeclarationType
            && type != ConstantDeclarationType) {
        // the other types can be redeclared
        return false;
    }

    DUChainWriteLocker lock(DUChain::lock());
    QList<Declaration*> declarations = currentContext()->topContext()->findDeclarations( identifier, startPos(node) );
    Q_FOREACH(Declaration* dec, declarations) {
        if (wasEncountered(dec) && isMatch(dec, type)) {
            reportRedeclarationError(dec, node);
            return true;
        }
    }
    return false;
}

void DeclarationBuilder::reportRedeclarationError(Declaration* declaration, AstNode* node)
{
    if (declaration->range().contains(startPos(node))) {
        // make sure this is not a wrongly reported redeclaration error
        return;
    }
    if (declaration->context()->topContext()->url() == internalFunctionFile()) {
        reportError(i18n("Cannot redeclare PHP internal %1.", declaration->toString()), node);
    } else if (auto trait = dynamic_cast<TraitMemberAliasDeclaration*>(declaration)) {
        reportError(
            i18n("%1 and %2 define the same property (%3) in the composition of %1. This might be incompatible, to improve maintainability consider using accessor methods in traits instead.")
            .arg(dynamic_cast<ClassDeclaration*>(currentDeclaration())->prettyName().str(),
                 dynamic_cast<ClassDeclaration*>(trait->aliasedDeclaration().data()->context()->owner())->prettyName().str(),
                 dynamic_cast<ClassMemberDeclaration*>(trait)->identifier().toString()), node, IProblem::Warning
        );
    } else {
        ///TODO: try to shorten the filename by removing the leading path to the current project
        reportError(
            i18n("Cannot redeclare %1, already declared in %2 on line %3.",
                 declaration->toString(), declaration->context()->topContext()->url().str(), declaration->range().start.line + 1
                ), node
        );
    }
}
void DeclarationBuilder::visitOuterTopStatement(OuterTopStatementAst* node)
{
    //docblock of an AssignmentExpression
    setComment(formatComment(node, m_editor));
    m_lastTopStatementComment = m_editor->parseSession()->docComment(node->startToken);

    DeclarationBuilderBase::visitOuterTopStatement(node);
}

void DeclarationBuilder::visitAssignmentExpression(AssignmentExpressionAst* node)
{
    if ( node->assignmentExpressionEqual ) {
        PushValue<FindVariableResults> restore(m_findVariable);

        DeclarationBuilderBase::visitAssignmentExpression(node);
    } else {
        DeclarationBuilderBase::visitAssignmentExpression(node);
    }
}

void DeclarationBuilder::visitVariable(VariableAst* node)
{
    if ( m_findVariable.find ) {
        getVariableIdentifier(node, m_findVariable.identifier, m_findVariable.parentIdentifier,
                              m_findVariable.node, m_findVariable.isArray);
        m_findVariable.find = false;
    }
    DeclarationBuilderBase::visitVariable(node);
}

void DeclarationBuilder::declareVariable(DUContext* parentCtx, AbstractType::Ptr type,
                                            const QualifiedIdentifier& identifier,
                                            AstNode* node)
{
    DUChainWriteLocker lock(DUChain::lock());

    // we must not re-assign $this in a class context
    /// Qualified identifier for 'this'
    static const QualifiedIdentifier thisQId(QStringLiteral("this"));
    if ( identifier == thisQId
            && currentContext()->parentContext()
            && currentContext()->parentContext()->type() == DUContext::Class ) {

        // checks if imports \ArrayAccess
        ClassDeclaration* currentClass = dynamic_cast<ClassDeclaration*>(currentContext()->parentContext()->owner());
        ClassDeclaration* arrayAccess = nullptr;

        auto imports = currentContext()->parentContext()->importedParentContexts();
        for( const DUContext::Import& ctx : imports ) {
            DUContext* import = ctx.context(topContext());
            if(import->type() == DUContext::Class) {
                ClassDeclaration* importedClass = dynamic_cast<ClassDeclaration*>(import->owner());
                if(importedClass) {
                    if(importedClass->prettyName().str() == "ArrayAccess" && importedClass->classType() == ClassDeclarationData::ClassType::Interface && !import->parentContext()->owner()) {
                        arrayAccess = importedClass;
                    }
                }
            }
        }

        IntegralType* thisVar = static_cast<IntegralType*>(type.data());
        // check if this is used as array
        if(arrayAccess && currentClass && thisVar && thisVar->dataType() == AbstractType::TypeArray)
        {
            uint noOfFunc = 0;
            auto declarations = currentContext()->parentContext()->localDeclarations();
            // check if class implements all 4 functions
            for(auto &dec : declarations) {
                if(dec->isFunctionDeclaration()) {
                    QualifiedIdentifier func = dec->qualifiedIdentifier();
                    QString funcname = func.last().identifier().str();
                    if(funcname == "offsetexists" || funcname == "offsetget" || funcname == "offsetset" || funcname == "offsetunset") {
                        noOfFunc++;
                    }
                }
            }

            if(noOfFunc < 4) {
                // check if class is not abstract
                if(currentClass->classModifier() != ClassDeclarationData::ClassModifier::Abstract) {
                    reportError(i18n("Class %1 contains %2 abstract methods and must therefore be declared abstract or implement the remaining methods.",currentClass->prettyName().str(),4-noOfFunc), QList<AstNode*>() << node);
                }
            }

            return;
        }

        reportError(i18n("Cannot re-assign $this."), QList<AstNode*>() << node);
        return;
    }

    const RangeInRevision newRange = editorFindRange(node, node);

    // check if this variable is already declared
    {
        QList< Declaration* > decs = parentCtx->findDeclarations(identifier.first(), startPos(node), 0, DUContext::DontSearchInParent);
        if ( !decs.isEmpty() ) {
            QList< Declaration* >::const_iterator it = decs.constEnd() - 1;
            while ( true ) {
                // we expect that the list of declarations has the newest declaration at back
                if ( dynamic_cast<VariableDeclaration*>( *it ) ) {
                    if (!wasEncountered(*it)) {
                        encounter(*it);
                        // force new range https://bugs.kde.org/show_bug.cgi?id=262189,
                        // might be wrong when we had syntax errors in there before
                        (*it)->setRange(newRange);
                    }
                    if ( (*it)->abstractType() && !(*it)->abstractType()->equals(type.data()) ) {
                        // if it's currently mixed and we now get something more definite, use that instead
                        if ( ReferenceType::Ptr rType = ReferenceType::Ptr::dynamicCast((*it)->abstractType()) ) {
                            if ( IntegralType::Ptr integral = IntegralType::Ptr::dynamicCast(rType->baseType()) ) {
                                if ( integral->dataType() == IntegralType::TypeMixed ) {
                                    // referenced mixed to referenced @p type
                                    ReferenceType::Ptr newType(new ReferenceType());
                                    newType->setBaseType(type);
                                    (*it)->setType(newType);
                                    return;
                                }
                            }
                        }
                        if ( IntegralType::Ptr integral = IntegralType::Ptr::dynamicCast((*it)->abstractType()) ) {
                            if ( integral->dataType() == IntegralType::TypeMixed ) {
                                // mixed to @p type
                                (*it)->setType(type);
                                return;
                            }
                        }
                        // else make it unsure
                        UnsureType::Ptr unsure = UnsureType::Ptr::dynamicCast((*it)->abstractType());
                        // maybe it's referenced?
                        ReferenceType::Ptr rType = ReferenceType::Ptr::dynamicCast((*it)->abstractType());
                        if ( !unsure && rType ) {
                            unsure = UnsureType::Ptr::dynamicCast(rType->baseType());
                        }
                        if ( !unsure ) {
                            unsure = UnsureType::Ptr(new UnsureType());
                            if ( rType ) {
                                unsure->addType(rType->baseType()->indexed());
                            } else {
                                unsure->addType((*it)->indexedType());
                            }
                        }
                        unsure->addType(type->indexed());
                        if ( rType ) {
                            rType->setBaseType(AbstractType::Ptr(unsure.data()));
                            (*it)->setType(rType);
                        } else {
                            (*it)->setType(unsure);
                        }
                    }
                    return;
                }
                if ( it == decs.constBegin() ) {
                    break;
                }
                --it;
            }
        }
    }

    VariableDeclaration *dec = openDefinition<VariableDeclaration>(identifier, newRange);
    dec->setKind(Declaration::Instance);
    if (!m_lastTopStatementComment.isEmpty()) {
        QRegularExpression rx("(\\*|///)\\s*@superglobal");
        if (rx.indexIn(m_lastTopStatementComment) != -1) {
            dec->setSuperglobal(true);
        }
    }
    //own closeDeclaration() that doesn't use lastType()
    dec->setType(type);

    // variable declarations are not namespaced in PHP
    if (currentContext()->type() == DUContext::Namespace) {
        dec->setContext(currentContext()->topContext());
    }

    eventuallyAssignInternalContext();
    DeclarationBuilderBase::closeDeclaration();
}

DUContext* getClassContext(const QualifiedIdentifier &identifier, DUContext* currentCtx) {
    /// Qualified identifier for 'this'
    static const QualifiedIdentifier thisQId(QStringLiteral("this"));
    if ( identifier == thisQId ) {
        if ( currentCtx->parentContext() && currentCtx->parentContext()->type() == DUContext::Class ) {
            return currentCtx->parentContext();
        }
    } else {
        DUChainReadLocker lock(DUChain::lock());
        Q_FOREACH( Declaration* parent, currentCtx->topContext()->findDeclarations(identifier) ) {
            if ( StructureType::Ptr ctype = parent->type<StructureType>() ) {
                return ctype->internalContext(currentCtx->topContext());
            }
        }
        ///TODO: if we can't find anything here we might have to use the findDeclarationImport helper
    }
    return 0;
}

///TODO: we need to handle assignment to array-members properly
///      currently we just make sure the array is declared, but don't know
///      anything about its contents
void DeclarationBuilder::visitAssignmentExpressionEqual(AssignmentExpressionEqualAst *node)
{
    DeclarationBuilderBase::visitAssignmentExpressionEqual(node);

    if ( !m_findVariable.identifier.isEmpty() && currentAbstractType()) {
        //create new declaration assignments to not-yet declared variables and class members

        AbstractType::Ptr type;
        if ( m_findVariable.isArray ) {
            // implicit array declaration
            type = AbstractType::Ptr(new IntegralType(IntegralType::TypeArray));
        } else {
            type = currentAbstractType();
        }

        if ( !m_findVariable.parentIdentifier.isEmpty() ) {
            // assignment to class members

            if ( DUContext* ctx = getClassContext(m_findVariable.parentIdentifier, currentContext()) ) {
                declareClassMember(ctx, type, m_findVariable.identifier, m_findVariable.node);
            }
        } else {
            // assignment to other variables
            declareVariable(currentContext(), type, m_findVariable.identifier, m_findVariable.node );
        }
    }
}

void DeclarationBuilder::visitFunctionCall(FunctionCallAst* node)
{
    QualifiedIdentifier id;
    if (!m_isInternalFunctions) {
        FunctionType::Ptr oldFunction = m_currentFunctionType;

        DeclarationPointer dec;
        if ( node->stringFunctionName ) {
            dec = findDeclarationImport(FunctionDeclarationType, node->stringFunctionName);
        } else if ( node->stringFunctionNameOrClass ) {
            id = identifierForNamespace(node->stringFunctionNameOrClass, m_editor);
            dec = findDeclarationImport(FunctionDeclarationType, id);
        } else {
            ///TODO: node->varFunctionName
        }

        if ( dec ) {
            m_currentFunctionType = dec->type<FunctionType>();
        } else {
            m_currentFunctionType = 0;
        }

        DeclarationBuilderBase::visitFunctionCall(node);

        m_currentFunctionType = oldFunction;
    } else {
        // optimize for internal function file
        DeclarationBuilderBase::visitFunctionCall(node);
    }

    if (node->stringFunctionNameOrClass && !node->stringFunctionName && !node->varFunctionName) {
        if (id.toString(RemoveExplicitlyGlobalPrefix) == QLatin1String("define")
                && node->stringParameterList && node->stringParameterList->parametersSequence
                && node->stringParameterList->parametersSequence->count() > 0) {
            //constant, defined through define-function

            //find name of the constant (first argument of the function call)
            CommonScalarAst* scalar = findCommonScalar(node->stringParameterList->parametersSequence->at(0)->element);
            if (scalar && scalar->string != -1) {
                QString constant = m_editor->parseSession()->symbol(scalar->string);
                constant = constant.mid(1, constant.length() - 2);
                RangeInRevision newRange = editorFindRange(scalar, scalar);
                AbstractType::Ptr type;
                if (node->stringParameterList->parametersSequence->count() > 1) {
                    type = getTypeForNode(node->stringParameterList->parametersSequence->at(1)->element);
                    Q_ASSERT(type);
                    type->setModifiers(type->modifiers() | AbstractType::ConstModifier);
                } // TODO: else report error?
                DUChainWriteLocker lock;
                // find fitting context to put define in,
                // pick first namespace or global context otherwise
                DUContext* ctx = currentContext();
                while (ctx->type() != DUContext::Namespace && ctx->parentContext()) {
                    ctx = ctx->parentContext();
                }
                injectContext(ctx); //constants are always global
                QualifiedIdentifier identifier(constant);
                isGlobalRedeclaration(identifier, scalar, ConstantDeclarationType);
                Declaration* dec = openDefinition<Declaration>(identifier, newRange);
                dec->setKind(Declaration::Instance);
                if (type) {
                    dec->setType(type);
                    injectType(type);
                }
                closeDeclaration();
                closeInjectedContext();
            }
        }
    }
}

void DeclarationBuilder::visitFunctionCallParameterList(FunctionCallParameterListAst* node)
{
    int oldPos = m_functionCallParameterPos;
    m_functionCallParameterPos = 0;

    DeclarationBuilderBase::visitFunctionCallParameterList(node);

    m_functionCallParameterPos = oldPos;
}

void DeclarationBuilder::visitFunctionCallParameterListElement(FunctionCallParameterListElementAst* node)
{
    PushValue<FindVariableResults> restore(m_findVariable);

    DeclarationBuilderBase::visitFunctionCallParameterListElement(node);

    if ( m_findVariable.node && m_currentFunctionType &&
            m_currentFunctionType->arguments().count() > m_functionCallParameterPos) {
        ReferenceType::Ptr refType = m_currentFunctionType->arguments()
                                        .at(m_functionCallParameterPos).cast<ReferenceType>();
        if ( refType ) {
            // this argument is referenced, so if the node contains undeclared variables we have
            // to declare them with a NULL type, see also:
            // https://de.php.net/manual/en/language.references.whatdo.php

            // declare with NULL type, just like PHP does
            declareFoundVariable(AbstractType::Ptr(new IntegralType(IntegralType::TypeNull)));
        }
    }

    ++m_functionCallParameterPos;
}

void DeclarationBuilder::visitAssignmentListElement(AssignmentListElementAst* node)
{
    PushValue<FindVariableResults> restore(m_findVariable);

    DeclarationBuilderBase::DefaultVisitor::visitAssignmentListElement(node);

    if ( m_findVariable.node ) {
        ///TODO: get a proper type here, if possible
        declareFoundVariable(AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed)));
    }
}

void DeclarationBuilder::declareFoundVariable(AbstractType::Ptr type)
{
    Q_ASSERT(m_findVariable.node);

    ///TODO: support something like: foo($var[0])
    if ( !m_findVariable.isArray ) {
        DUContext *ctx = nullptr;
        if ( m_findVariable.parentIdentifier.isEmpty() ) {
            ctx = currentContext();
        } else {
            ctx = getClassContext(m_findVariable.parentIdentifier, currentContext());
        }
        if ( ctx ) {
            bool isDeclared = false;
            {
                DUChainWriteLocker lock(DUChain::lock());
                RangeInRevision range = m_editor->findRange(m_findVariable.node);
                Q_FOREACH( Declaration* dec, ctx->findDeclarations(m_findVariable.identifier) ) {
                    if ( dec->kind() == Declaration::Instance ) {
                        if (!wasEncountered(dec) || (dec->context() == ctx && range < dec->range())) {
                            // just like a "redeclaration", hence we must update the range
                            // TODO: do the same for all other uses of "encounter"?
                            dec->setRange(editorFindRange(m_findVariable.node));
                            encounter(dec);
                        }
                        isDeclared = true;
                        break;
                    }
                }
            }
            if ( !isDeclared && m_findVariable.parentIdentifier.isEmpty() ) {
                // check also for global vars
                isDeclared = findDeclarationImport(GlobalVariableDeclarationType, m_findVariable.identifier);
            }
            if ( !isDeclared ) {
                // couldn't find the dec, declare it
                if ( !m_findVariable.parentIdentifier.isEmpty() ) {
                    declareClassMember(ctx, type, m_findVariable.identifier, m_findVariable.node);
                } else {
                    declareVariable(ctx, type, m_findVariable.identifier, m_findVariable.node);
                }
            }
        }
    }
}

void DeclarationBuilder::visitStatement(StatementAst* node)
{
    DeclarationBuilderBase::visitStatement(node);

    if (node->foreachVariable) {
        PushValue<FindVariableResults> restore(m_findVariable);
        visitForeachVariable(node->foreachVariable);
        if (m_findVariable.node) {
            declareFoundVariable(lastType());
        }
    }

    if (node->foreachVarAsVar) {
        PushValue<FindVariableResults> restore(m_findVariable);
        visitForeachVariable(node->foreachVarAsVar);
        if (m_findVariable.node) {
            declareFoundVariable(lastType());
        }
    }

    if (node->foreachExprAsVar) {
        PushValue<FindVariableResults> restore(m_findVariable);
        visitVariable(node->foreachExprAsVar);
        if (m_findVariable.node) {
            declareFoundVariable(lastType());
        }
    }

}

void DeclarationBuilder::visitStaticVar(StaticVarAst* node)
{
    DeclarationBuilderBase::visitStaticVar(node);

    DUChainWriteLocker lock(DUChain::lock());
    openDefinition<VariableDeclaration>(identifierForNode(node->var),
                                        editorFindRange(node->var, node->var));
    currentDeclaration()->setKind(Declaration::Instance);

    closeDeclaration();
}

void DeclarationBuilder::visitGlobalVar(GlobalVarAst* node)
{
    DeclarationBuilderBase::visitGlobalVar(node);
    if (node->var) {
        QualifiedIdentifier id = identifierForNode(node->var);
        if ( recompiling() ) {
            DUChainWriteLocker lock(DUChain::lock());
            // sadly we can't use findLocalDeclarations() here, since it un-aliases declarations
            Q_FOREACH( Declaration* dec, currentContext()->localDeclarations() ) {
                if ( dynamic_cast<AliasDeclaration*>(dec) && dec->identifier() == id.first() ) {
                    // don't redeclare but reuse the existing declaration
                    encounter(dec);
                    return;
                }
            }
        }
        // no existing declaration found, create one
        DeclarationPointer aliasedDeclaration = findDeclarationImport(GlobalVariableDeclarationType, node->var);
        if (aliasedDeclaration) {
            DUChainWriteLocker lock(DUChain::lock());
            AliasDeclaration* dec = openDefinition<AliasDeclaration>(id, m_editor->findRange(node->var));
            dec->setAliasedDeclaration(aliasedDeclaration.data());
            closeDeclaration();
        }
    }
}

void DeclarationBuilder::visitCatchItem(CatchItemAst *node)
{
    DeclarationBuilderBase::visitCatchItem(node);

    DUChainWriteLocker lock(DUChain::lock());
    openDefinition<VariableDeclaration>(identifierForNode(node->var),
                                        editorFindRange(node->var, node->var));
    currentDeclaration()->setKind(Declaration::Instance);
    closeDeclaration();
}

void DeclarationBuilder::visitUnaryExpression(UnaryExpressionAst* node)
{
    DeclarationBuilderBase::visitUnaryExpression(node);
    IndexedString includeFile = getIncludeFileForNode(node, m_editor);
    if ( !includeFile.isEmpty() ) {
        DUChainWriteLocker lock;
        TopDUContext* includedCtx = DUChain::self()->chainForDocument(includeFile);
        if ( !includedCtx ) {
            // invalid include
            return;
        }

        QualifiedIdentifier identifier(includeFile.str());

        Q_FOREACH( Declaration* dec, includedCtx->findDeclarations(identifier, CursorInRevision(0, 1)) ) {
            if ( dec->kind() == Declaration::Import ) {
                encounter(dec);
                return;
            }
        }
        injectContext(includedCtx);
        openDefinition<Declaration>(identifier, RangeInRevision(0, 0, 0, 0));
        currentDeclaration()->setKind(Declaration::Import);
        eventuallyAssignInternalContext();
        DeclarationBuilderBase::closeDeclaration();
        closeInjectedContext();
    }
}

void DeclarationBuilder::openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const RangeInRevision& range)
{
    NamespaceDeclaration* dec = m_namespaces.value(node->string, 0);
    Q_ASSERT(dec);
    DeclarationBuilderBase::setEncountered(dec);
    openDeclarationInternal(dec);

    DeclarationBuilderBase::openNamespace(parent, node, identifier, range);
}

void DeclarationBuilder::closeNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier)
{
    DeclarationBuilderBase::closeNamespace(parent, node, identifier);
    closeDeclaration();
}

void DeclarationBuilder::visitUseNamespace(UseNamespaceAst* node)
{
    DUChainWriteLocker lock;

    if ( currentContext()->type() != DUContext::Namespace &&
            !node->aliasIdentifier && node->identifier->namespaceNameSequence->count() == 1 ) {
        reportError(i18n("The use statement with non-compound name '%1' has no effect.",
                        identifierForNode(node->identifier->namespaceNameSequence->front()->element).toString()),
                    node->identifier, IProblem::Warning);
        return;
    }
    IdentifierAst* idNode = node->aliasIdentifier ? node->aliasIdentifier : node->identifier->namespaceNameSequence->back()->element;
    IdentifierPair id = identifierPairForNode(idNode);

    ///TODO: case insensitive!
    QualifiedIdentifier qid = identifierForNamespace(node->identifier, m_editor);
    ///TODO: find out why this must be done (see mail to kdevelop-devel on jan 18th 2011)
    qid.setExplicitlyGlobal( false );

    DeclarationPointer dec = findDeclarationImport(ClassDeclarationType, qid);

    if (dec)
    {
        // Check for a name conflict
        DeclarationPointer dec2 = findDeclarationImport(ClassDeclarationType, id.second);

        if (dec2 && dec2->context()->scopeIdentifier() == currentContext()->scopeIdentifier() &&
            dec2->context()->topContext() == currentContext()->topContext() &&
            dec2->identifier().toString() == id.second.toString())
        {
            reportError(i18n("Cannot use '%1' as '%2' because the name is already in use.",
                            dec.data()->identifier().toString(), id.second.toString()),
                        node->identifier, IProblem::Error);
            return;
        }

        AliasDeclaration* decl = openDefinition<AliasDeclaration>(id.second, m_editor->findRange(idNode));
        decl->setAliasedDeclaration(dec.data());
    }
    else
    {
        NamespaceAliasDeclaration* decl = openDefinition<NamespaceAliasDeclaration>(id.second,
                                                                                    m_editor->findRange(idNode));
        decl->setImportIdentifier( qid );
        decl->setPrettyName( id.first );
        decl->setKind(Declaration::NamespaceAlias);
    }
    closeDeclaration();
}

void DeclarationBuilder::updateCurrentType()
{
    DUChainWriteLocker lock(DUChain::lock());
    currentDeclaration()->setAbstractType(currentAbstractType());
}

void DeclarationBuilder::supportBuild(AstNode* node, DUContext* context)
{
    // generally we are the second pass through the doc (see PreDeclarationBuilder)
    // so notify our base about it
    setCompilingContexts(false);
    DeclarationBuilderBase::supportBuild(node, context);
}

void DeclarationBuilder::closeContext()
{
    if (currentContext()->type() == DUContext::Function) {
        Q_ASSERT(currentDeclaration<AbstractFunctionDeclaration>());
        currentDeclaration<AbstractFunctionDeclaration>()->setInternalFunctionContext(currentContext());
    }
    // We don't want the first pass to clean up stuff, since
    // there is lots of stuff we visit/encounter here first.
    // So we clean things up here.
    setCompilingContexts(true);
    DeclarationBuilderBase::closeContext();
    setCompilingContexts(false);
}

void DeclarationBuilder::encounter(Declaration* dec)
{
    // when we are recompiling, it's important to mark decs as encountered
    // and update their comments
    if ( recompiling() && !wasEncountered(dec) ) {
        dec->setComment(comment());
        setEncountered(dec);
    }
}

}
