/*
 * KDevelop Php Code Completion Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
