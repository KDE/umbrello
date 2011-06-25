/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_create_deploydiag.h"

#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

#include <klocale.h>

namespace Uml
{

    CmdCreateDeployDiag::CmdCreateDeployDiag(UMLDoc* doc, const QString& name)
      : m_pUMLDoc(doc), m_pUMLView(0), m_Name(name)
    {
        setText(i18n("Create deployment diagram : %1", name));
    }

    CmdCreateDeployDiag::~CmdCreateDeployDiag()
    {
    }

    void CmdCreateDeployDiag::redo()
    {
        UMLFolder* temp = m_pUMLDoc->rootFolder(Uml::ModelType::Deployment);
        m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::DiagramType::Deployment);
    }

    void CmdCreateDeployDiag::undo()
    {
        if (m_pUMLView) {
            m_pUMLDoc->removeDiagram(m_pUMLView->getID());
        }
    }

}
