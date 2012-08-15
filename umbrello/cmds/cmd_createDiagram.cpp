/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_createDiagram.h"

#include "model_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

#include <klocale.h>

namespace Uml
{

    CmdCreateDiagram::CmdCreateDiagram(UMLDoc* doc, Uml::DiagramType type, const QString& name)
      : QUndoCommand(),
        m_name(name),
        m_type(type),
        m_pUMLDoc(doc),
        m_pUMLView(0)
    {
        QString msg = i18n("Create diagram %1: %2", DiagramType::toString(type), name);
        setText(msg);
    }

    CmdCreateDiagram::~CmdCreateDiagram()
    {
    }

    void CmdCreateDiagram::redo()
    {
        Uml::ModelType modelType = Model_Utils::convert_DT_MT(m_type);
        UMLFolder* folder = m_pUMLDoc->rootFolder(modelType);
        m_pUMLView = m_pUMLDoc->createDiagram(folder, m_type, m_name);
    }

    void CmdCreateDiagram::undo()
    {
        if (m_pUMLView) {
            m_pUMLDoc->removeDiagram(m_pUMLView->umlScene()->ID());
        }
    }

}
