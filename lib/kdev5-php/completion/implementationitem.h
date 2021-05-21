/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    Basec on Cpp ImplementationHelperItem

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef IMPLEMENTATIONITEM_H
#define IMPLEMENTATIONITEM_H

#include "item.h"

using namespace KDevelop;

namespace Php
{

class ImplementationItem : public NormalDeclarationCompletionItem
{
public:
    enum HelperType {
        Override,
        Implement,
        OverrideVar
    };

    explicit ImplementationItem(HelperType type, KDevelop::DeclarationPointer decl = {},
                                QExplicitlySharedDataPointer<Php::CodeCompletionContext> context = {}, int _inheritanceDepth = 0)
            : NormalDeclarationCompletionItem(decl, context, _inheritanceDepth), m_type(type) {}

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override;

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;

    HelperType m_type;
};

}

#endif // IMPLEMENTATIONITEM_H
