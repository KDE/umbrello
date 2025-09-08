/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdbasewidgetcommand.h"

// app includes
#include "umlassociation.h"
#include "associationwidget.h"
#include "umlfolder.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

namespace Uml
{
    CmdBaseWidgetCommand::CmdBaseWidgetCommand(UMLWidget* widget)
      : m_isAssoc(false)
    {
        setWidget(widget);
    }

    CmdBaseWidgetCommand::CmdBaseWidgetCommand(AssociationWidget* widget)
      : m_isAssoc(true)
    {
        setWidget(widget);
    }

    CmdBaseWidgetCommand::~CmdBaseWidgetCommand()
    {
    }

    void CmdBaseWidgetCommand::setWidget(UMLWidget* widget)
    {
        Q_ASSERT(widget);

        m_widget = widget;
        m_assocWidget = nullptr;
        m_widgetId = widget->localID();
        m_scene = widget->umlScene();
        m_sceneId = widget->umlScene()->ID();
    }

    void CmdBaseWidgetCommand::setWidget(AssociationWidget* widget)
    {
        Q_ASSERT(widget);

        m_widget = nullptr;
        m_assocWidget = widget;
        m_widgetId = widget->id();
        m_scene = widget->umlScene();
        m_sceneId = widget->umlScene()->ID();
    }

    UMLScene* CmdBaseWidgetCommand::scene()
    {
        UMLView* umlView = UMLApp::app()->document()->findView(m_sceneId);

        if (umlView)
            return umlView->umlScene();

        Q_ASSERT(m_scene.data());
        return m_scene;
    }

    UMLWidget* CmdBaseWidgetCommand::widget()
    {
        UMLWidget* widget = scene()->findWidget(m_widgetId);
        if (widget)
            return widget;

        Q_ASSERT(m_widget.data());
        return m_widget;
    }

    AssociationWidget* CmdBaseWidgetCommand::assocWidget()
    {
        AssociationWidget *widget = scene()->findAssocWidget(m_widgetId);
        if (widget)
            return widget;

        Q_ASSERT(m_assocWidget.data());
        return m_assocWidget;
    }

    /**
     * Add widget to scene
     *
     * @param widget  Pointer to widget to add
     */
    void CmdBaseWidgetCommand::addWidgetToScene(UMLWidget* widget)
    {
        scene()->addWidgetCmd(widget);
        widget->activate();
    }

    /**
     * Add widget to scene
     *
     * @param widget  Pointer to widget to add
     */
    void CmdBaseWidgetCommand::addWidgetToScene(AssociationWidget* widget)
    {
        widget->clipSize();
        if (scene()->addAssociation(widget, false)) {
            // if view went ok, then append in document
            UMLAssociation *umla = widget->association();
            if (umla) {
                // association with model representation in UMLDoc
                Uml::ModelType::Enum m = Model_Utils::convert_DT_MT(scene()->type());
                UMLDoc *umldoc = UMLApp::app()->document();
                umla->setUMLPackage(umldoc->rootFolder(m));
                umldoc->addAssociation(umla);
                if (umla->getAssocType() == Uml::AssociationType::Containment) {
                    UMLObject *newContainer = widget->widgetForRole(Uml::RoleType::A)->umlObject();
                    UMLObject *objToBeMoved = widget->widgetForRole(Uml::RoleType::B)->umlObject();
                    if (newContainer && objToBeMoved) {
                        Model_Utils::treeViewMoveObjectTo(newContainer, objToBeMoved);
                    }
                }
            }
        }
        widget->activate();
    }

    void CmdBaseWidgetCommand::removeWidgetFromScene(UMLWidget *widget)
    {
        if (widget != nullptr)
            scene()->removeWidgetCmd(widget);
    }

    void CmdBaseWidgetCommand::removeWidgetFromScene(AssociationWidget* widget)
    {
        if (widget != nullptr)
            scene()->removeWidgetCmd(widget);
    }
}
