/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_BASEOBJECTCOMMAND_H
#define CMD_BASEOBJECTCOMMAND_H

#include "basictypes.h"

#include <QUndoCommand>

class UMLObject;

namespace Uml
{
    class CmdBaseObjectCommand : public QUndoCommand
    {
        public:
            explicit CmdBaseObjectCommand(UMLObject *object);
            virtual ~CmdBaseObjectCommand();

        protected:
            Uml::ID::Type m_objectId;

            void setObject(UMLObject* object);
            UMLObject* object();
    };
}

#endif // CMD_BASEOBJECTCOMMAND_H
