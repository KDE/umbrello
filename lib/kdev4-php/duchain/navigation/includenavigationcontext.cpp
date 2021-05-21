/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "includenavigationcontext.h"

#include <language/duchain/parsingenvironment.h>

namespace Php {

IncludeNavigationContext::IncludeNavigationContext(const KDevelop::IncludeItem& item, KDevelop::TopDUContextPointer topContext)
    : AbstractIncludeNavigationContext(item, topContext, KDevelop::PhpParsingEnvironment)
{
}

}
