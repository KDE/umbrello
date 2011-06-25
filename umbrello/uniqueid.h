/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "basictypes.h"

namespace UniqueID {

    Uml::IDType gen();

    /////////// auxiliary functions ////////////////////////////////////
    // Only required by code that does special operations on unique IDs.
    // NB Try to avoid these functions if possible because their
    // implementation and/or programming interface may change.

    void init();

    void set(Uml::IDType id);
    Uml::IDType get();

}  // end namespace UniqueID

#endif

