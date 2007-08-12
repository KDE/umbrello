/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <qstringlist.h>

namespace Codegen_Utils {

/**
 * Return list of C++ datatypes
 */
QStringList cppDatatypes();

/**
 * Get list of C++ reserved keywords
 */
const QStringList reservedCppKeywords();

/**
 * Add C++ stereotypes
 */
void createCppStereotypes();

/**
 * Return the input string with the first letter capitalized.
 */
QString capitalizeFirstLetter(const QString &string);

}

#endif  // CODEGEN_UTILS_H

