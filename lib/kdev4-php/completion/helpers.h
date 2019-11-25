/*
 * KDevelop Php Code Completion Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2008 Niko Sams <niko.sams@gmail.com>
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

#ifndef COMPLETIONHELPERS_H
#define COMPLETIONHELPERS_H

#include <language/duchain/types/abstracttype.h>
#include "phpcompletionexport.h"

#include <QtCore/QStringList>

class QString;
class QVariant;
template<class T>
class QList;

namespace KTextEditor {
    class Document;
}

namespace Php
{
class NormalDeclarationCompletionItem;

/**
 * @param phpTypeHinting set to true if you only want PHP-valid typehinting (i.e. only array and classes)
 */
void KDEVPHPCOMPLETION_EXPORT createArgumentList(const NormalDeclarationCompletionItem& item, QString& ret, QList<QVariant>* highlighting, bool phpTypeHinting = false);

/// get a list of tokens which define a method at the end of the given text
/// NOTE: result will contain "function" if it's the last token at the end of the text
QStringList getMethodTokens(QString text);

/**
 * Get a string representing an indentation.
 *
 * @param document The current document.
 * @returns a QString containing the indentation to be used.
 */
const QString KDEVPHPCOMPLETION_EXPORT indentString(KTextEditor::Document *document);

/**
 * Get the indentation of a given line.
 *
 * You usually want to use it with something like the following:
 *
 * \code
 * const QString indentation = getIndentation( document->line(replaceRange.start().line()) );
 * \endcode
 */
QString KDEVPHPCOMPLETION_EXPORT getIndentation( const QString &line );
}

#endif

