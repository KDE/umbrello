/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "navigationwidget.h"
#include "declarationnavigationcontext.h"
#include "includenavigationcontext.h"
#include "magicconstantnavigationcontext.h"

namespace Php
{
using namespace KDevelop;

NavigationWidget::NavigationWidget(KDevelop::DeclarationPointer declaration, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix)
        : m_declaration(declaration)
{
    m_topContext = topContext;

    initBrowser(400);

    //The first context is registered so it is kept alive by the shared-pointer mechanism
    m_startContext = NavigationContextPointer(new DeclarationNavigationContext(declaration, m_topContext));
    m_startContext->setPrefixSuffix(htmlPrefix, htmlSuffix);
    setContext(m_startContext);
}

NavigationWidget::NavigationWidget(const IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix, const QString& htmlSuffix)
{
  m_topContext = topContext;

  initBrowser(200);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new IncludeNavigationContext(includeItem, m_topContext));
  m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
  setContext( m_startContext );
}

NavigationWidget::NavigationWidget(TopDUContextPointer topContext, SimpleCursor position, const QString& constant)
    : AbstractNavigationWidget()
{
  m_topContext = topContext;

  initBrowser(200);

  //The first context is registered so it is kept alive by the shared-pointer mechanism
  m_startContext = NavigationContextPointer(new MagicConstantNavigationContext(topContext, position, constant));
  setContext( m_startContext );
}

QString NavigationWidget::shortDescription(KDevelop::Declaration* declaration)
{
    NavigationContextPointer ctx(new DeclarationNavigationContext(DeclarationPointer(declaration), TopDUContextPointer())); ///@todo give correct top-context
    return ctx->html(true);
}

QString NavigationWidget::shortDescription(const IncludeItem& includeItem) {
  NavigationContextPointer ctx(new IncludeNavigationContext(includeItem, TopDUContextPointer())); ///@todo give correct top-context
  return ctx->html(true);
}

}

#include "navigationwidget.moc"
