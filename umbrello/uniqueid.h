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

#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "umlnamespace.h"

namespace UniqueID {

    /**
     * MAIN FUNCTION: Return a new unique ID.
     */
    Uml::IDType gen();


    /////////// auxiliary functions ////////////////////////////////////
    // Only required by code that does special operations on unique IDs.
    // NB Try to avoid these functions if possible because their
    // implementation and/or programming interface may change.

    /**
     * Reinitialize the unique ID counter.
     * Should not normally be required because the ID counter is
     * initialized by default anyway.
     */
    void init();

    /**
     * Return the last generated unique ID without generating a new one.
     */
    Uml::IDType get();

    /**
     * Explicitly set a new ID value.
     */
    void set(Uml::IDType id);

}  // end namespace UniqueID

#endif

