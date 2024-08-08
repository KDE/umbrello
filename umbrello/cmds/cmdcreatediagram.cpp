/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdcreatediagram.h"

#include "model_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

#include <KLocalizedString>

namespace Uml
{

    CmdCreateDiagram::CmdCreateDiagram(UMLDoc* doc, Uml::DiagramType::Enum type, const QString& name, UMLFolder *parent)
      : QUndoCommand(),
        m_name(name),
        m_type(type),
        m_pUMLDoc(doc),
        m_pUMLView(nullptr),
        m_parent(parent)
    {
        QString msg = i18n("Create diagram %1: %2", DiagramType::toString(type), name);
        setText(msg);

        m_sceneId = Uml::ID::None;
    }

    CmdCreateDiagram::~CmdCreateDiagram()
    {
    }

    void CmdCreateDiagram::redo()
    {
        if (!m_pUMLDoc->findView(m_type, m_name, true)) {
            Uml::ModelType::Enum modelType = Model_Utils::convert_DT_MT(m_type);
            UMLFolder* folder = m_parent ? m_parent : m_pUMLDoc->rootFolder(modelType);
            m_pUMLView = m_pUMLDoc->createDiagram(folder, m_type, m_name, m_sceneId);
        }

        // Remember the scene-ID, it might be auto generated. The ID must
        // not change after undo/redo because other commands may try to
        // lookup the diagram later.
        m_sceneId = m_pUMLView->umlScene()->ID();
    }

    void CmdCreateDiagram::undo()
    {
        if (m_pUMLView) {
            m_pUMLDoc->removeDiagramCmd(m_sceneId);
        }
    }

}
