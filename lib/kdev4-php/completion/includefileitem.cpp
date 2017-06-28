/*
 * KDevelop Php Code Completion Support
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "includefileitem.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KTextEditor;

namespace Php {

void IncludeFileItem::execute(Document* document, const Range& _word)
{
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
        QRegExp regexp("(?:include|require)(?:_once)?(\\s*)(\\(?)(\\s*)(?:dirname\\s*\\(\\s*__FILE__\\s*\\)\\s*\\.\\s*)?([\"'])", Qt::CaseInsensitive);

        if ( regexp.lastIndexIn(textBefore) != -1 ) {
            closeChar = regexp.cap(4)[0];

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
                word.end().setColumn( word.end().column() + pos );
            }
        }
    }

    document->replaceText(word, newText);

    // when we complete a directory, move the cursor behind it so we can continue with auto-completion
    if ( includeItem.isDirectory ) {
        if ( View* view = document->activeView() ) {
            view->setCursorPosition(Cursor(_word.start().line(), _word.start().column() + includeItem.name.size() + 1));
        }
    }
}

}
