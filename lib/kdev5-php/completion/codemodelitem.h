/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPCOMPLETIONCODEMODELITEM_H
#define PHPCOMPLETIONCODEMODELITEM_H

#include <language/codecompletion/codecompletionitem.h>
#include <language/duchain/parsingenvironment.h>
#include <serialization/indexedstring.h>

#include "../duchain/completioncodemodel.h"

#include "context.h"

namespace Php
{
struct CompletionCodeModelItem;
class CodeCompletionContext;

class KDEVPHPCOMPLETION_EXPORT CodeModelCompletionItem : public KDevelop::CompletionTreeItem
{
public:
    explicit CodeModelCompletionItem(const KDevelop::ParsingEnvironmentFilePointer &, const CompletionCodeModelItem &item);

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;
    KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const override;
    void execute(KTextEditor::View* View, const KTextEditor::Range& word) override;
    KDevelop::DeclarationPointer declaration() const override;

protected:
    CompletionCodeModelItem m_item;
    KDevelop::ParsingEnvironmentFilePointer m_env;
    mutable KDevelop::DeclarationPointer m_decl;
};

}

#endif
