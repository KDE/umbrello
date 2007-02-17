/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlnamespace.h"
#include "qregexp.h"

namespace Uml {

bool tagEq (const QString& inTag, const QString& inPattern) {
    QString tag = inTag;
    QString pattern = inPattern;
    tag.remove( QRegExp("^\\w+:") );  // remove leading "UML:" or other
    int patSections = pattern.contains( '.' ) + 1;
    QString tagEnd = tag.section( '.', -patSections );
    return (tagEnd.lower() == pattern.lower());
}

QString Visibility::toString(Value value, bool mnemonic) {
    switch (value) {
    case Protected:
        return (mnemonic ? "#" : "protected");
        break;
    case Private:
        return (mnemonic ? "-" : "private");
        break;
    case Implementation:
        return (mnemonic ? "~" : "implementation");
        break;
    case Public:
    default:
        return (mnemonic ? "+" : "public");
        break;
    }
}

Visibility Visibility::fromString(const QString& vis) {
    if (vis == "public" || vis == "+")
        return Visibility(Public);
    else if (vis == "protected" || vis == "#")
        return Visibility(Protected);
    else if (vis == "private" || vis == "-")
        return Visibility(Private);
    else if (vis == "~")
        return Visibility(Implementation);
    else if (vis == "signals")
        return Visibility(Protected);
    else if (vis == "class")
        return Visibility(Private);
    else
        return Visibility(Public);
}

Visibility::Visibility(): _v(Public) {
}

Visibility::Visibility(Value v): _v(v) {
}

QString Visibility::toString(bool mnemonic) const {
    return toString(_v, mnemonic);
}

Visibility::operator Visibility::Value() const {
    return _v;
}

}  // end namespace Uml

