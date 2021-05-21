/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PHPCODECOMPLETIONCONTEXT_H
#define PHPCODECOMPLETIONCONTEXT_H

#include <ksharedptr.h>

#include <language/codecompletion/codecompletioncontext.h>
#include <language/duchain/types/abstracttype.h>

#include "phpcompletionexport.h"
#include "item.h"
#include "expressionevaluationresult.h"

namespace KTextEditor
{
class View;
class Cursor;
}

namespace KDevelop
{
class DUContext;
class ClassDeclaration;

class CompletionTreeItem;
typedef KSharedPtr<CompletionTreeItem> CompletionTreeItemPointer;

class SimpleCursor;
}

namespace Php
{

class TokenAccess;

/**
 * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
 * */
class KDEVPHPCOMPLETION_EXPORT CodeCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    typedef KSharedPtr<CodeCompletionContext> Ptr;

    /**
     * To be used from the Worker. For parent/child contexts, use the private ctor that takes a TokenAccess.
     * That way we don't have to reparse the text over and over again.
     *
     * @param context The context in which code completion was requested.
     * @param text The text before @p position. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start.
     * @param followingText When @p position is inside a word, followingText will contain the text that follows.
     * @param position The position where code completion was requested.
     * @param depth Simple recursion counter.
     *
     * @warning The du-chain must be unlocked when this is called.
     * */
    CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text,
                          const QString& followingText, const KDevelop::CursorInRevision& position,
                          int depth = 0);
    ~CodeCompletionContext();

    ///Computes the full set of completion items, using the information retrieved earlier.
    ///Should only be called on the first context, parent contexts are included in the computations.
    ///@param Abort is checked regularly, and if it is false, the computation is aborted.
    virtual QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true);

    enum MemberAccessOperation {
        NoMemberAccess,  ///With NoMemberAccess, a global completion should be done
        MemberAccess,      ///klass->
        FunctionCallAccess,  ///"function(". Will never appear as initial access-operation, but as parentContext() access-operation.
        StaticMemberAccess, ///klass::
        NewClassChoose, /// after the "new" keyword any non-abstract classes (not interfaces) should be shown
        ClassExtendsChoose, /// after "class XYZ extends" any non-final classes should be shown
        InterfaceChoose, /// after the "implements" keyword or after "interface XYZ extends" any interfaces should be shown
        InstanceOfChoose, /// after the "instanceof" operator, any class-type should be shown
        ExceptionChoose, /// after keywords "catch" and "throw new" only classes which extend Exception should be shown
        ExceptionInstanceChoose, /// after the "throw" keyword instancec of the exception class should be shown
        ClassMemberChoose, /// in class context show list of overloadable or implementable methods
        /// and typical keywords for classes, i.e. access modifiers, static etc.
        FileChoose, /// autocompletion for files
        NamespaceChoose, /// autocompletion after namespace keyword
        BackslashAccess /// autocompletion after backslash token
    };

    ///@return the used access-operation
    MemberAccessOperation memberAccessOperation() const;

    ExpressionEvaluationResult memberAccessContainer() const;

    /**
     * Returns the internal context of memberAccessContainer, if any.
     *
     * When memberAccessOperation is StaticMemberChoose, this returns all
     * fitting namespace-contexts.
     * */
    QList<KDevelop::DUContext*> memberAccessContainers() const;

    /**
     * When memberAccessOperation is FunctionCallAccess,
     * this returns all functions available for matching, together with the argument-number that should be matched.
     * */
    const QList<KDevelop::AbstractFunctionDeclaration*>& functions() const;

    virtual CodeCompletionContext* parentContext();

protected:
    virtual QList<QSet<KDevelop::IndexedString> > completionFiles();
    inline bool isValidCompletionItem(KDevelop::Declaration* dec);

private:
    /**
     * Internal ctor to use when you want to create parent contexts.
     *
     * NOTE: Since you pass the TokenAccess, it's not save to use
     *       it afterwards. Probably you don't want to do that anyway.
     *       Hence always return after creating the parent.
     */
    CodeCompletionContext(KDevelop::DUContextPointer context, const KDevelop::CursorInRevision& position,
                            TokenAccess& lastToken, const int depth);

    /**
     * Evaluate expression for the given @p lastToken.
     */
    void evaluateExpression(TokenAccess& lastToken);

    MemberAccessOperation m_memberAccessOperation;
    ExpressionEvaluationResult m_expressionResult;
    QString m_expression;
    bool m_parentAccess;
    /// If we do file completion after dirname(__FILE__) or in PHP 5.3 after __DIR__
    /// relative URLS have to start with a /
    bool m_isFileCompletionAfterDirname;
    /**
     * a list of indexes of identifiers which must not be added as completion items
     * examples:
     * class test implements foo, ...
     * => identifiers test and foo must not be proposed for completion
     **/
    QList<uint> m_forbiddenIdentifiers;
    /// filled only during BackslashAccess and NamespaceChoose
    KDevelop::QualifiedIdentifier m_namespace;

    void forbidIdentifier(const QString &identifier);
    void forbidIdentifier(KDevelop::ClassDeclaration* identifier);
};
}

#endif
