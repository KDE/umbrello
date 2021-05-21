/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INCLUDEFILEITEM_H
#define INCLUDEFILEITEM_H

#include <language/codecompletion/abstractincludefilecompletionitem.h>

#include "../duchain/navigation/navigationwidget.h"

namespace Php {

typedef KDevelop::AbstractIncludeFileCompletionItem<Php::NavigationWidget> BaseIncludeFileItem;

class IncludeFileItem : public BaseIncludeFileItem {
public:
    IncludeFileItem(const KDevelop::IncludeItem& include)
        : BaseIncludeFileItem(include) {};

    void execute(KTextEditor::View* View, const KTextEditor::Range& word) override;
};

}

#endif // INCLUDEFILEITEM_H
