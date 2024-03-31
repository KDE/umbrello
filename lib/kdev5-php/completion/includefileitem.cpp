/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "includefileitem.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KTextEditor;

namespace Php {

void IncludeFileItem::execute(View* view, const Range& _word)
{
    KTextEditor::Document *document = view->document();
    Range word(_word);

    QString newText;

    if ( includeItem.isDirectory ) {
        newText = includeItem.name + '/';
    } else {
        newText = includeItem.name;
    }

    // Add suffix
    QChar closeChar;
    {
        const QString textBefore = document->text(Range(Cursor(0, 0), _word.start()));
        QRegularExpression regexp("(?:include|require)(?:_once)?(\\s*)(\\(?)(\\s*)(?:dirname\\s*\\(\\s*__FILE__\\s*\\)\\s*\\.\\s*)?([\"'])", Qt::CaseInsensitive);

        if ( regexp.lastIndexIn(textBefore) != -1 ) {
            closeChar = regexp.cap(4).at(0);

            newText.append(closeChar);
            if ( !regexp.cap(2).isEmpty() ) {
                newText.append(regexp.cap(3));
                newText.append(')');
            }
            newText.append(';');
        }
    }

    // Adapt range and replace existing stuff
    {
        const QString textAfter = document->text(Range(_word.end(), document->documentEnd()));
        if ( !textAfter.isEmpty() ) {
            int pos = 0;
            for (; pos < textAfter.length(); ++pos ) {
                if ( textAfter[pos].isSpace() ) {
                    break;
                } else if ( textAfter[pos] == closeChar ) {
                    // remove close char
                    ++pos;
                    // find semicolon (if existing)
                    for (int i = pos; i < textAfter.length(); ++i ) {
                        if ( textAfter[i] == ';' ) {
                            // remove semicolon
                            pos = i + 1;
                            break;
                        } else if ( !textAfter[i].isSpace() && textAfter[i] != ')' ) {
                            break;
                        }
                    }
                    break;
                }
            }
            if ( pos > 0 ) {
                word.setEnd(word.end() + Cursor(0, pos));
            }
        }
    }

    document->replaceText(word, newText);

    // when we complete a directory, move the cursor behind it so we can continue with auto-completion
    if ( includeItem.isDirectory ) {
        if (view) {
            view->setCursorPosition(Cursor(_word.start().line(), _word.start().column() + includeItem.name.size() + 1));
        }
    }
}

}
