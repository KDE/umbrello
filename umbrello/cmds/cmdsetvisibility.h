/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDSETVISIBILITY_H
#define CMDSETVISIBILITY_H

#include "cmdbaseobjectcommand.h"

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
