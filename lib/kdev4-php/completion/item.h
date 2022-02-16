/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPCOMPLETIONITEM_H
#define PHPCOMPLETIONITEM_H

#include <ksharedptr.h>
#include <ktexteditor/codecompletionmodel.h>

#include <language/duchain/duchainpointer.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include "context.h"

namespace KTextEditor
{
class CodeCompletionModel;
class Document;
class Range;
class Cursor;
}

class QModelIndex;

namespace Php
{
class CodeCompletionContext;

//A completion item used for completion of normal declarations while normal code-completion
class KDEVPHPCOMPLETION_EXPORT NormalDeclarationCompletionItem : public KDevelop::NormalDeclarationCompletionItem
{
public:
    explicit NormalDeclarationCompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<KDevelop::CodeCompletionContext> context = KSharedPtr<KDevelop::CodeCompletionContext>(), int _inheritanceDepth = 0)
            : KDevelop::NormalDeclarationCompletionItem(decl, context, _inheritanceDepth) {
    }


    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

protected:
    virtual QString declarationName() const;
    virtual void executed(KTextEditor::Document* document, const KTextEditor::Range& word);
    virtual QWidget* createExpandingWidget(const KDevelop::CodeCompletionModel* model) const;
    virtual bool createsExpandingWidget() const;

    KSharedPtr<CodeCompletionContext> completionContext() const;
};

}

#endif
