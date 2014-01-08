/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_SETVISIBILITY_H
#define CMD_SETVISIBILITY_H

#include "cmd_baseObjectCommand.h"

namespace Uml
{
    class CmdSetVisibility : public CmdBaseObjectCommand
    {
        public:
            CmdSetVisibility(UMLObject* obj, Uml::Visibility::Enum visibility);
            ~CmdSetVisibility();

            void redo();
            void undo();

        private:
            Uml::Visibility::Enum  m_oldVisibility;
            Uml::Visibility::Enum  m_visibility;
    };
}

#endif
