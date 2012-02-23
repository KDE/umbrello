/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_create_entityrelationdiag.h"

#include "umldoc.h"
#include "umlview.h"

#include <klocale.h>

namespace Uml
{

    CmdCreateEntityRelationDiag::CmdCreateEntityRelationDiag(UMLDoc* doc, const QString& name)
      : QUndoCommand(),
        m_pUMLDoc(doc),
        m_pUMLView(0)
    {
        setText(i18n("Create entity relationship diagram : %1", name));
    }

    CmdCreateEntityRelationDiag::~CmdCreateEntityRelationDiag()
    {
    }

    void CmdCreateEntityRelationDiag::redo()
    {
        UMLFolder* temp = m_pUMLDoc->rootFolder(Uml::ModelType::EntityRelationship);
        m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::DiagramType::EntityRelationship);
    }

    void CmdCreateEntityRelationDiag::undo()
    {
        if (m_pUMLView) {
            m_pUMLDoc->removeDiagram(m_pUMLView->getID());
        }
    }

}
