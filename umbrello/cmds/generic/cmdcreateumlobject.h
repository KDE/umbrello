/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCREATEUMLOBJECT_H
#define CMDCREATEUMLOBJECT_H

#include "umlobject.h"
#include <QDomElement>
#include <QUndoCommand>

class UMLPackage;

namespace Uml
{
    class CmdCreateUMLObject : public QUndoCommand
    {
    public:
        explicit CmdCreateUMLObject(UMLObject* o);
        ~CmdCreateUMLObject();

        void redo();
        void undo();

    private:
        UMLObject             *m_obj;
        UMLPackage            *m_package;
        UMLObject::ObjectType  m_type;
        QString                m_name;
        bool                   m_skipSignal;
    };
}

#endif
