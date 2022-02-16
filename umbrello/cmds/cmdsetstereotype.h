/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDSETSTEREOTYPE_H
#define CMDSETSTEREOTYPE_H

#include "cmdbaseobjectcommand.h"

namespace Uml
{
    class CmdSetStereotype : public CmdBaseObjectCommand
    {
        public:
            CmdSetStereotype(UMLObject * obj, const QString& stereo);
            ~CmdSetStereotype();

            void redo();
            void undo();

        private:
            QString    m_stereo;
            QString    m_oldStereo;
    };
}

#endif
