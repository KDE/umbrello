/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDBASEOBJECTCOMMAND_H
#define CMDBASEOBJECTCOMMAND_H

#include "basictypes.h"

#include <QPointer>
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
            QPointer<UMLObject> m_object;

            void setObject(UMLObject* object);
            UMLObject* object();
    };
}

#endif // CMDBASEOBJECTCOMMAND_H
