/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
 ***************************************************************************/

// own header
#include "uniqueid.h"

// system includes
#include <uuid/uuid.h>

namespace UniqueID {

/**
 * Each model object gets assigned a unique ID.
 */
Uml::IDType m_uniqueID;

Uml::IDType gen() {
    static char buf[40];
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_upper(uuid, buf);
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

