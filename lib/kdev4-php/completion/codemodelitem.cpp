/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codemodelitem.h"

#include <KTextEditor/Document>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <navigation/navigationwidget.h>
#include <language/codecompletion/codecompletionhelper.h>
#include <language/duchain/abstractfunctiondeclaration.h>

using namespace KDevelop;

namespace Php
{

CodeModelCompletionItem::CodeModelCompletionItem(const ParsingEnvironmentFilePointer &env, const CompletionCodeModelItem &item)
    : CompletionTreeItem(), m_item(item), m_env(env)
{
}

QVariant CodeModelCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{

    DUChainReadLocker lock(DUChain::lock(), 500);
    if (!lock.locked()) {
        kDebug() << "Failed to lock the du-chain in time";
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case KDevelop::CodeCompletionModel::Name:
            return QVariant(m_item.prettyName.str());
        case KDevelop::CodeCompletionModel::Prefix:
            return QString("class");
        }
        break;
    case Qt::DecorationRole:
        if( index.column() == CodeCompletionModel::Icon ) {
            CodeCompletionModel::CompletionProperties p = completionProperties();
            lock.unlock();
            return DUChainUtils::iconForProperties(p);
        }
        break;
    case CodeCompletionModel::IsExpandable:
        return QVariant(true);
    case CodeCompletionModel::ExpandingWidget: {
        if (!declaration()) return QVariant();
        QWidget *nav = new NavigationWidget(declaration(), model->currentTopContext());
        model->addNavigationWidget(this, nav);

        QVariant v;
        v.setValue<QWidget*>(nav);
        return v;
    }
    }
    return QVariant();
}

CodeCompletionModel::CompletionProperties CodeModelCompletionItem::completionProperties() const
{
    CodeCompletionModel::CompletionProperties ret = CodeCompletionModel::Class;
    return ret;
}

void CodeModelCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    document->replaceText(word, m_item.prettyName.str());

    if (declaration() && dynamic_cast<AbstractFunctionDeclaration*>(declaration().data())) {
        //Do some intelligent stuff for functions with the parens:
        KTextEditor::Cursor pos = word.start();
        pos.setColumn(pos.column() + m_item.prettyName.length());
        insertFunctionParenText(document, pos, declaration());
    }
}

DeclarationPointer CodeModelCompletionItem::declaration() const
{
    if (!m_decl) {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        QList<Declaration*> decls = m_env->topContext()->findDeclarations(m_item.id);
        if (decls.isEmpty()) return DeclarationPointer();
        m_decl = decls.first();
    }
    return m_decl;
}


}

