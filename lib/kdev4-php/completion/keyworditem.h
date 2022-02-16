/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    Based on Cpp ImplementationHelperItem

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KEYWORDITEM_H
#define KEYWORDITEM_H

#include "item.h"

using namespace KDevelop;

namespace Php
{

class KeywordItem : public NormalDeclarationCompletionItem
{
public:
    /// Use @p customReplacement for additional power on how the keyword gets replaced.
    /// Newlines will be indented to the indentation level of the line we execute the item on.
    /// To increase an indentation level, use %INDENT%. To place the cursor, use %CURSOR%.
    /// Alternatively you can select a word with %SELECT%word%SELECT%
    ///
    /// NOTE: By default (i.e. when this function never gets called) @p keyword will be used as replacement.
    explicit KeywordItem(const QString &keyword,
                KSharedPtr<KDevelop::CodeCompletionContext> context = KSharedPtr<KDevelop::CodeCompletionContext>(),
                const QString &customReplacement = QString())
            : NormalDeclarationCompletionItem(KDevelop::DeclarationPointer(), context, 0),
            m_keyword(keyword), m_replacement(customReplacement) {}

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

private:
    const QString m_keyword;
    QString m_replacement;
};

}

#endif // KEYWORDITEM_H
