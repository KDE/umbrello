/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_SETNAME_H
#define CMD_SETNAME_H

#include "cmd_baseObjectCommand.h"

namespace Uml
{
    class CmdSetName : public CmdBaseObjectCommand
    {
        public:
            CmdSetName(UMLObject* obj, const QString& name);
            ~CmdSetName();

            void redo();
            void undo();

        private:
            QString m_oldname;
            QString m_name;
    };
}

#endif
