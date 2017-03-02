/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2017                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdrenameumlinstancetype.h"

//app includes
#include "instance.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{
    CmdRenameUMLInstanceType::CmdRenameUMLInstanceType(UMLInstance* o, UMLClassifier *type)
      : m_obj(o),
        m_type(type)
    {
        setText(i18n("Rename type: %1 to %2", o->classifier()->name(), type->name()));
        m_oldType = o->classifier();
    }

    CmdRenameUMLInstanceType::~CmdRenameUMLInstanceType()
    {
    }

    void CmdRenameUMLInstanceType::redo()
    {
        m_obj->setClassifierCmd(m_type);
    }

    void CmdRenameUMLInstanceType::undo()
    {
        m_obj->setClassifierCmd(m_oldType);
    }
}
