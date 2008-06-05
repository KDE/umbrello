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

#include "optionstate.h"

namespace Settings{

    struct OptionStateHoster
    {
        static OptionState *pd_optionState;

        OptionState& getOptionState() {
            if(!pd_optionState) {
                pd_optionState = new OptionState;
            }
            return *pd_optionState;
        }

        void setOptionState(const OptionState& optstate) {
            if(!pd_optionState) {
                pd_optionState = new OptionState;
            }
            *pd_optionState = optstate;
        }
    };

    OptionStateHoster hoster;

    Settings::OptionState* Settings::OptionStateHoster::pd_optionState = NULL;

    OptionState& getOptionState()
    {
        return hoster.getOptionState();
    }

    void setOptionState(const OptionState& optstate)
    {
        hoster.setOptionState(optstate);
    }

}  // namespace Settings

