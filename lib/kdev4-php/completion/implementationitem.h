/*
 * KDevelop Php Code Completion Support
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
 * Basec on Cpp ImplementationHelperItem
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

    explicit ImplementationItem(HelperType type, KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<KDevelop::CodeCompletionContext> context = KSharedPtr<KDevelop::CodeCompletionContext>(), int _inheritanceDepth = 0)
            : NormalDeclarationCompletionItem(decl, context, _inheritanceDepth), m_type(type) {}

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

    HelperType m_type;
};

}

#endif // IMPLEMENTATIONITEM_H
