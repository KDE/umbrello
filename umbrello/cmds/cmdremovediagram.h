/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDREMOVEDIAGRAM_H
#define CMDREMOVEDIAGRAM_H

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
