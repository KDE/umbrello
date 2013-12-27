/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CREATE_UMLOBJECT_H
#define CMD_CREATE_UMLOBJECT_H

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
