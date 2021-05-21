/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

