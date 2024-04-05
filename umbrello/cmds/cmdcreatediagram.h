/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCREATEDIAGRAM_H
#define CMDCREATEDIAGRAM_H

#include "basictypes.h"

#include <QUndoCommand>

class UMLDoc;
class UMLView;
class UMLFolder;

namespace Uml
{
    class CmdCreateDiagram : public QUndoCommand
    {
    public:
        explicit CmdCreateDiagram(UMLDoc* doc, Uml::DiagramType::Enum type, const QString& name, UMLFolder *parent = nullptr);
        ~CmdCreateDiagram();

        void redo();
        void undo();
        UMLView *view() { return m_pUMLView; }

    private:
        QString                 m_name;
        Uml::DiagramType::Enum  m_type;
        UMLDoc*                 m_pUMLDoc;
        UMLView*                m_pUMLView;
        Uml::ID::Type           m_sceneId;
        UMLFolder*              m_parent;
    };
}

#endif
