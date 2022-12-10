/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdremovediagram.h"

#include "basictypes.h"
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

#include <KLocalizedString>
#include <QXmlStreamWriter>

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
        QString xmi;
        QXmlStreamWriter stream(&xmi);
        stream.writeStartElement(QStringLiteral("diagram"));
        scene->saveToXMI(stream);
        stream.writeEndElement();  // diagram
        QString error;
        int line;
        QDomDocument domDoc;
        if (domDoc.setContent(xmi, &error, &line)) {
            // The first child element contains the diagram XMI
            m_element = domDoc.firstChild().firstChild().toElement();  // CHECK was: container
        } else {
            logWarn3("CmdRemoveDiagram(%1): Cannot set content. Error %2 line %3",
                     scene->name(), error, line);
        }
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
