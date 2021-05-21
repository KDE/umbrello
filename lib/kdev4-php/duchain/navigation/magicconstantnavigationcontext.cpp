/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "magicconstantnavigationcontext.h"

#include <QtGui/QTextDocument>
#include <KLocalizedString>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>

using namespace KDevelop;

namespace Php {

const int foo = 5;

MagicConstantNavigationContext::MagicConstantNavigationContext(TopDUContextPointer topContext,
                                                                const SimpleCursor& position,
                                                                const QString& constant)
    : AbstractNavigationContext(topContext, 0), m_position(position.line, position.column), m_constant(constant)
{
}


QString MagicConstantNavigationContext::name() const
{
    return m_constant;
}

DUContext* findContext(TopDUContextPointer topCtx, const CursorInRevision& pos, DUContext::ContextType type) {
    DUContext* ctx = topCtx->findContextAt(pos);
    while ( ctx && ctx->type() != type ) {
        ctx = ctx->parentContext();
    }
    if ( !ctx || ctx->type() != type ) {
        return 0;
    } else {
        return ctx;
    }
}

QString MagicConstantNavigationContext::html(bool /*shorten*/)
{
    QString html = "<html><body><p><small><small>";
    html += typeHighlight(i18n("magic constant"));
    html += ' ';
    html += nameHighlight(Qt::escape(m_constant));
    html += "<br/>\n";

    QString value;
    ///TODO: php 5.3: __DIR__, __NAMESPACE__
    if ( m_constant == "__FILE__" ) {
        value = Qt::escape(m_topContext->url().str());
    } else if ( m_constant == "__LINE__" ) {
        value.setNum(m_position.line + 1);
    } else if ( m_constant == "__CLASS__" ) {
        if ( DUContext* ctx = findContext(m_topContext, m_position, DUContext::Class) ) {
            value = codeHighlight(Qt::escape(ctx->localScopeIdentifier().toString()));
        } else {
            value = commentHighlight(i18n("empty (not inside a class)"));
        }
    } else if ( m_constant == "__METHOD__" ) {
        CursorInRevision pos = m_position;
        while ( DUContext* ctx = findContext(m_topContext, pos, DUContext::Other) ) {
            if ( !ctx->parentContext() ) {
                break;
            }
            if ( ctx->parentContext()->type() == DUContext::Class ) {
                value = codeHighlight(Qt::escape(
                            ctx->parentContext()->localScopeIdentifier().toString() + "::"
                            + ctx->localScopeIdentifier().toString()
                        ));
                break;
            }
            // might be a "normal" function inside a method...
            pos = ctx->range().start;
        }
        if ( value.isEmpty() ) {
            value = commentHighlight(i18n("empty (not inside a method)"));
        }
    } else if ( m_constant == "__FUNCTION__" ) {
        CursorInRevision pos = m_position;
        if ( DUContext* ctx = findContext(m_topContext, pos, DUContext::Other) ) {
            if ( ctx->owner() && ctx->owner()->type<FunctionType>() ) {
                value = codeHighlight(Qt::escape(ctx->localScopeIdentifier().toString()));
            }
        }
        if ( value.isEmpty() ) {
            value = commentHighlight(i18n("empty (not inside a function)"));
        }
    } else if ( m_constant == "__NAMESPACE__" ) {
        if ( DUContext* ctx = findContext(m_topContext, m_position, DUContext::Namespace) ) {
            if ( ctx->owner() && ctx->owner()->kind() == Declaration::Namespace ) {
                value = codeHighlight(Qt::escape(ctx->localScopeIdentifier().toString()));
            }
        }
        if ( value.isEmpty() ) {
            value = commentHighlight(i18n("empty (not inside a namespace)"));
        }
    }

    html += i18n("current value: %1", value);

    html += "</small></small></p></body></html>";

    return html;
}


}
