/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    Based on Cpp ImplementationHelperItem

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keyworditem.h"

#include "helpers.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/codecompletion/codecompletionmodel.h>
#include "declarations/classmethoddeclaration.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kicon.h>
#include <klocalizedstring.h>

using namespace KDevelop;

namespace Php
{

QVariant KeywordItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    switch (role) {
    case CodeCompletionModel::IsExpandable:
        return QVariant(false);
    case Qt::DisplayRole:
        if (index.column() == KTextEditor::CodeCompletionModel::Name) {
            return QVariant(m_keyword);
        } else {
            return QVariant("");
        }
        break;
    case KTextEditor::CodeCompletionModel::ItemSelected:
        return QVariant("");
    case KTextEditor::CodeCompletionModel::InheritanceDepth:
        return QVariant(0);
    default:
        //pass
        break;
    }

    return NormalDeclarationCompletionItem::data(index, role, model);
}

void KeywordItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    if ( !m_replacement.isEmpty() ) {
        QString replacement = m_replacement;
        replacement = replacement.replace('\n', '\n' + getIndentation(document->line(word.start().line())));
        replacement = replacement.replace("%INDENT%", indentString(document));

        int cursorPos = replacement.indexOf("%CURSOR%");
        int selectionEnd = -1;
        if ( cursorPos != -1 ) {
            replacement.remove("%CURSOR%");
        } else {
            cursorPos = replacement.indexOf("%SELECT%");
            if ( cursorPos != -1 ) {
                replacement.remove("%SELECT%");
                selectionEnd = replacement.indexOf("%ENDSELECT%", cursorPos + 1);
                if ( selectionEnd == -1 ) {
                    selectionEnd = replacement.length();
                }
                replacement.remove("%ENDSELECT%");
            }
        }

        document->replaceText(word, replacement);

        if ( cursorPos != -1 ) {
            if ( KTextEditor::View* view = document->activeView() ) {
                replacement = replacement.left(cursorPos);
                KTextEditor::Cursor newPos(
                    word.start().line() + replacement.count('\n'),
                    word.start().column() + replacement.length() - replacement.lastIndexOf('\n') - 1
                );
                view->setCursorPosition(newPos);
                if ( selectionEnd != -1 ) {
                    ///TODO: maybe we want to support multi-line selections in the future?
                    view->setSelection(
                        KTextEditor::Range(
                            newPos,
                            KTextEditor::Cursor(
                                newPos.line(),
                                newPos.column() + selectionEnd - cursorPos
                            )
                        )
                    );
                }
            }
        }
    } else {
        document->replaceText(word, m_keyword + ' ');
    }
}

}
