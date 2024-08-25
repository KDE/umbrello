/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "uniqueid.h"
#include <krandom.h>

namespace UniqueID {


/**
 * Each model object gets assigned a unique ID.
 */
Uml::ID::Type m_uniqueID;

/**
 * MAIN FUNCTION: Return a new unique ID.
 */
Uml::ID::Type gen()
{
    m_uniqueID = 'u' + KRandom::randomString(12).toStdString();
    return m_uniqueID;
}

/**
 * Reinitialize the unique ID counter.
 * Should not normally be required because the ID counter is
 * initialized by default anyway.
 */
void init()
{
    m_uniqueID = Uml::ID::Reserved;
}

/**
 * Return the last generated unique ID without generating a new one.
 */
Uml::ID::Type get()
{
    return m_uniqueID;
}

/**
 * Explicitly set a new ID value.
 */
void set(Uml::ID::Type id)
{
    m_uniqueID = id;
}

}  // end namespace UniqueID

