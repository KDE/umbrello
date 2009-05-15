/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "uniqueid.h"
#include <krandom.h>

namespace UniqueID {


/**
 * Each model object gets assigned a unique ID.
 */
Uml::IDType m_uniqueID;

/**
 * MAIN FUNCTION: Return a new unique ID.
 */
Uml::IDType gen()
{
    m_uniqueID = std::string(KRandom::randomString(12).toLatin1());
    return m_uniqueID;
}

/**
 * Reinitialize the unique ID counter.
 * Should not normally be required because the ID counter is
 * initialized by default anyway.
 */
void init()
{
    m_uniqueID = Uml::id_Reserved;
}

/**
 * Return the last generated unique ID without generating a new one.
 */
Uml::IDType get()
{
    return m_uniqueID;
}

/**
 * Explicitly set a new ID value.
 */
void set(Uml::IDType id)
{
    m_uniqueID = id;
}

}  // end namespace UniqueID

