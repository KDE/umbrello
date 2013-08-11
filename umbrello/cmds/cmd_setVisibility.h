/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
 ***************************************************************************/

#ifndef CMD_SETVISIBILITY_H
#define CMD_SETVISIBILITY_H

#include <QUndoCommand>

#include "basictypes.h"

class UMLObject;

namespace Uml
{
    class CmdSetVisibility : public QUndoCommand
    {
        public:
            CmdSetVisibility(UMLObject* obj, Uml::Visibility::Enum visibility);
            ~CmdSetVisibility();

            void redo();
            void undo();

        private:
            Uml::Visibility::Enum  m_oldVisibility;
            Uml::Visibility::Enum  m_visibility;
            UMLObject*             m_umlObject;
    };
}

#endif
