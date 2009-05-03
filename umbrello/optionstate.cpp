/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "optionstate.h"
#include <kglobal.h>

namespace Settings {

    /*
     * Impt: This ensures creation of OptionState object after
     * QApplication there by avoiding nasty font rendering issues
     * which occurs due to creation of QFont objects before
     * QApplication object is created.
     */
    K_GLOBAL_STATIC(OptionState, opState)

    OptionState& getOptionState()
    {
        return *opState;
    }

    void setOptionState(const OptionState& optstate)
    {
        *opState = optstate;
    }

}  // namespace Settings

