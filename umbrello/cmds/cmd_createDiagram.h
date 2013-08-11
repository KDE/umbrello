/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
 ***************************************************************************/

#ifndef CMD_CREATE_DIAGRAM_H
#define CMD_CREATE_DIAGRAM_H

#include "basictypes.h"

#include <QUndoCommand>

class UMLDoc;
class UMLView;

namespace Uml
{
    class CmdCreateDiagram : public QUndoCommand
    {
    public:
        explicit CmdCreateDiagram(UMLDoc* doc, Uml::DiagramType::Enum type, const QString& name);
        ~CmdCreateDiagram();

        void redo();
        void undo();

    private:
        QString                 m_name;
        Uml::DiagramType::Enum  m_type;
        UMLDoc*                 m_pUMLDoc;
        UMLView*                m_pUMLView;
    };
}

#endif
