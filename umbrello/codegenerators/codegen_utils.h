/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H

#include <QStringList>

namespace Codegen_Utils {

QStringList cppDatatypes();

const QStringList reservedCppKeywords();

void createCppStereotypes();

QString capitalizeFirstLetter(const QString &string);

}

#endif  // CODEGEN_UTILS_H

