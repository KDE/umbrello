/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "uniqueid.h"

// system includes
#include <kapplication.h>

namespace UniqueID {

/**
 * Each model object gets assigned a unique ID.
 */
Uml::IDType m_uniqueID;

Uml::IDType gen() {
    static char buf[20];
    int length = 12;
    int i = 0;
    // Source: KDE4 kdelibs/kdecore/krandom.cpp KRandom::randomString()
    while (length--) {
        int r = kapp->random() % 62;
        r += 48;
        if (r > 57)
            r += 7;
        if (r > 90)
            r += 6;
        buf[i++] = char(r);
    }
    buf[i] = '\0';
    m_uniqueID = std::string(buf);
    return m_uniqueID;
}

void init() {
    m_uniqueID = Uml::id_Reserved;
}

Uml::IDType get() {
    return m_uniqueID;
}

void set(Uml::IDType id) {
    m_uniqueID = id;
}

}  // end namespace UniqueID

