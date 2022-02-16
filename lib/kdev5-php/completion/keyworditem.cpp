/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    Based on Cpp ImplementationHelperItem

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keyworditem.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <KLocalizedString>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/codecompletion/codecompletionmodel.h>

#include "../duchain/declarations/classmethoddeclaration.h"

#include "helpers.h"

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

void KeywordItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    KTextEditor::Document *document = view->document();
    if ( !m_replacement.isEmpty() ) {
        QString replacement = m_replacement;
        replacement = replacement.replace('\n', '\n' + getIndentation(document->line(word.start().line())));
        replacement = replacement.replace(QLatin1String("%INDENT%"), indentString(document));

        int cursorPos = replacement.indexOf(QStringLiteral("%CURSOR%"));
        int selectionEnd = -1;
        if ( cursorPos != -1 ) {
            replacement.remove(QStringLiteral("%CURSOR%"));
        } else {
            cursorPos = replacement.indexOf(QStringLiteral("%SELECT%"));
            if ( cursorPos != -1 ) {
                replacement.remove(QStringLiteral("%SELECT%"));
                selectionEnd = replacement.indexOf(QStringLiteral("%ENDSELECT%"), cursorPos + 1);
                if ( selectionEnd == -1 ) {
                    selectionEnd = replacement.length();
                }
                replacement.remove(QStringLiteral("%ENDSELECT%"));
            }
        }

        document->replaceText(word, replacement);

        if ( cursorPos != -1 ) {
            if (view) {
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
