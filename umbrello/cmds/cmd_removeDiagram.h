/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_REMOVE_DIAGRAM_H
#define CMD_REMOVE_DIAGRAM_H

#include "basictypes.h"

#include <QDomElement>
#include <QUndoCommand>

class UMLFolder;

namespace Uml
{
    class CmdRemoveDiagram : public QUndoCommand
    {
    public:
        explicit CmdRemoveDiagram(UMLFolder* folder,
                                  Uml::DiagramType::Enum type,
                                  const QString& name,
                                  Uml::ID::Type id);
        ~CmdRemoveDiagram();

        void redo();
        void undo();

    private:
        UMLFolder*      m_folder;
        Uml::DiagramType::Enum m_type;
        QString         m_name;
        Uml::ID::Type   m_sceneId;
        QDomElement     m_element;
    };
}

#endif
