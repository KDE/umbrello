/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDSETNAME_H
#define CMDSETNAME_H

#include "cmdbaseobjectcommand.h"

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
