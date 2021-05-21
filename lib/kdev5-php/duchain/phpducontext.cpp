/*
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "phpducontext.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/duchainregister.h>
#include <language/duchain/topducontextdata.h>

#include <language/util/includeitem.h>

#include <KIO/Global>

#include "navigation/navigationwidget.h"

namespace Php
{
using namespace KDevelop;

typedef PhpDUContext<TopDUContext> PhpTopDUContext;
REGISTER_DUCHAIN_ITEM_WITH_DATA(PhpTopDUContext, TopDUContextData);

typedef PhpDUContext<DUContext> PhpNormalDUContext;
REGISTER_DUCHAIN_ITEM_WITH_DATA(PhpNormalDUContext, DUContextData);

template<>
QWidget* PhpDUContext<TopDUContext>::createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix,
                                                            const QString& htmlSuffix, KDevelop::AbstractNavigationWidget::DisplayHints hints) const
{
    if (decl == 0) {
        return 0;
    } else if ( decl->kind() == Declaration::Import ) {
        QUrl u( decl->identifier().toString() );
        IncludeItem i;
        i.pathNumber = -1;
        i.name = u.fileName();
        i.isDirectory = false;
        i.basePath = KIO::upUrl(u);

        return new NavigationWidget( i, TopDUContextPointer(topContext), htmlPrefix, htmlSuffix, hints );
    } else {
        return new NavigationWidget(DeclarationPointer(decl), TopDUContextPointer(topContext ? topContext : this->topContext()),
                                    htmlPrefix, htmlSuffix, hints);
    }
}

template<>
QWidget* PhpDUContext<DUContext>::createNavigationWidget(Declaration* decl, TopDUContext* topContext, const QString& htmlPrefix,
                                                         const QString& htmlSuffix, KDevelop::AbstractNavigationWidget::DisplayHints hints) const
{
    if (decl == 0) {
        if (owner())
            return new NavigationWidget(DeclarationPointer(owner()), TopDUContextPointer(topContext ? topContext : this->topContext()),
                                        htmlPrefix, htmlSuffix, hints);
        else
            return 0;
    } else {
        return new NavigationWidget(DeclarationPointer(decl), TopDUContextPointer(topContext ? topContext : this->topContext()),
                                    htmlPrefix, htmlSuffix, hints);
    }
}


}

