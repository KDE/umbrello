/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "basictypes.h"

namespace UniqueID {

    Uml::ID::Type gen();

    /////////// auxiliary functions ////////////////////////////////////
    // Only required by code that does special operations on unique IDs.
    // NB Try to avoid these functions if possible because their
    // implementation and/or programming interface may change.

    void init();

    void set(Uml::ID::Type id);
    Uml::ID::Type get();

}  // end namespace UniqueID

#endif

