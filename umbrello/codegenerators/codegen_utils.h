/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <QtCore/QStringList>

namespace Codegen_Utils {

QStringList cppDatatypes();

const QStringList reservedCppKeywords();

void createCppStereotypes();

QString capitalizeFirstLetter(const QString &string);

}

#endif  // CODEGEN_UTILS_H

