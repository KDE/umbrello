/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_removeDiagram.h"

#include "basictypes.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

#include <klocale.h>

namespace Uml
{

    CmdRemoveDiagram::CmdRemoveDiagram(UMLFolder* folder,
                                       Uml::DiagramType::Enum type,
                                       const QString& name,
                                       Uml::ID::Type id)
      : QUndoCommand(),
        m_folder(folder),
        m_type(type),
        m_name(name),
        m_sceneId(id)
    {
        UMLScene* scene = UMLApp::app()->document()->findView(id)->umlScene();

        QString msg = i18n("Remove diagram %1", scene->name());
        setText(msg);

        // Save diagram XMI for undo
        QDomDocument doc;
        QDomElement container = doc.createElement("diagram");
        scene->saveToXMI(doc, container);

        // The first child element contains the diagram XMI
        m_element = container.firstChild().toElement();
    }

    CmdRemoveDiagram::~CmdRemoveDiagram()
    {
    }

    void CmdRemoveDiagram::redo()
    {
        UMLApp::app()->document()->removeDiagramCmd(m_sceneId);
    }

    void CmdRemoveDiagram::undo()
    {
        UMLDoc* doc = UMLApp::app()->document();
        UMLView* view = doc->createDiagram(m_folder, m_type, m_name, m_sceneId);

        view->umlScene()->loadFromXMI(m_element);
    }

}
