/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef INCLUDENAVIGATIONCONTEXT_H
#define INCLUDENAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractincludenavigationcontext.h>

namespace Php {

class IncludeNavigationContext : public KDevelop::AbstractIncludeNavigationContext {
public:
    IncludeNavigationContext(const KDevelop::IncludeItem& item, KDevelop::TopDUContextPointer topContext);
};

}

#endif // INCLUDENAVIGATIONCONTEXT_H
