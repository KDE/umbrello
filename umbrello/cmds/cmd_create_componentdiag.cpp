/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_create_componentdiag.h"

#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlscene.h"

#include <klocale.h>

namespace Uml
{

    CmdCreateComponentDiag::CmdCreateComponentDiag(UMLDoc* doc, const QString& name)
      : m_pUMLDoc(doc), m_pUMLView(0), m_Name(name)
    {
        setText(i18n("Create component diagram"));
    }

    CmdCreateComponentDiag::~CmdCreateComponentDiag()
    {
    }

    void CmdCreateComponentDiag::redo()
    {
        UMLFolder* temp = m_pUMLDoc->getRootFolder(Uml::mt_Component);
        m_pUMLView = m_pUMLDoc->createDiagram(temp, Uml::dt_Component);
    }

    void CmdCreateComponentDiag::undo()
    {
        if (m_pUMLView) {
            // [PORT]
            m_pUMLDoc->removeDiagram(m_pUMLView->umlScene()->getID());
        }
    }

}
