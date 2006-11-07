/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstateother.h"

// kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>

// app includes
#include "activitywidget.h"
#include "boxwidget.h"
#include "dialog_utils.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "notewidget.h"
#include "object_factory.h"
#include "statewidget.h"
#include "uml.h"
#include "umlview.h"
#include "umldoc.h"

using namespace Uml;

ToolBarStateOther::ToolBarStateOther(UMLView *umlView) : ToolBarStatePool(umlView) {
}

ToolBarStateOther::~ToolBarStateOther() {
}

void ToolBarStateOther::setCurrentElement() {
}

void ToolBarStateOther::mouseReleaseEmpty() {
    if (m_pMouseEvent->button() == Qt::LeftButton) {
        if (!newWidget()) {
            // Is UMLObject?

            m_pUMLView->setCreateObject(true);
            Object_Factory::createUMLObject(getObjectType());
        }

        m_pUMLView->resizeCanvasToItems();
    }
}

Uml::Object_Type ToolBarStateOther::getObjectType() {
    Object_Type ot;

    switch(getButton()) {
        case WorkToolBar::tbb_Actor:        ot = ot_Actor;          break;
        case WorkToolBar::tbb_UseCase:      ot = ot_UseCase;        break;
        case WorkToolBar::tbb_Class:        ot = ot_Class;          break;
        case WorkToolBar::tbb_Object:       ot = ot_Class;          break;  // Object is a class.
        case WorkToolBar::tbb_Package:      ot = ot_Package;        break;
        case WorkToolBar::tbb_Component:    ot = ot_Component;      break;
        case WorkToolBar::tbb_Node:         ot = ot_Node;           break;
        case WorkToolBar::tbb_Artifact:     ot = ot_Artifact;       break;
        case WorkToolBar::tbb_Interface:    ot = ot_Interface;      break;
        case WorkToolBar::tbb_Enum:         ot = ot_Enum;           break;
        case WorkToolBar::tbb_Entity:       ot = ot_Entity;         break;
        case WorkToolBar::tbb_Datatype:     ot = ot_Datatype;       break;

        default:                            ot = ot_UMLObject;      break;
    }

    return ot;
}

// TODO: The name is a bit confusing.
bool ToolBarStateOther::newWidget() {
    UMLWidget* umlWidget = NULL;

    switch (getButton()) {
        case WorkToolBar::tbb_Note:
            umlWidget = new NoteWidget(m_pUMLView);
            break;

        case WorkToolBar::tbb_Box:
            umlWidget = new BoxWidget(m_pUMLView);
            break;

        case WorkToolBar::tbb_Text:
            umlWidget = new FloatingTextWidget(m_pUMLView, tr_Floating, "");
            break;

        // Activity buttons
        case WorkToolBar::tbb_Initial_Activity:
            umlWidget = new ActivityWidget(m_pUMLView, ActivityWidget::Initial);
            break;

        case WorkToolBar::tbb_Activity:
            umlWidget = new ActivityWidget(m_pUMLView, ActivityWidget::Normal);
            break;

        case WorkToolBar::tbb_End_Activity:
            umlWidget = new ActivityWidget(m_pUMLView, ActivityWidget::End);
            break;

        case WorkToolBar::tbb_Branch:
            umlWidget = new ActivityWidget(m_pUMLView, ActivityWidget::Branch);
            break;

        case WorkToolBar::tbb_Fork:
        case WorkToolBar::tbb_StateFork:
            umlWidget = new ForkJoinWidget(m_pUMLView);
            break;

        case WorkToolBar::tbb_Initial_State:
            umlWidget = new StateWidget(m_pUMLView, StateWidget::Initial);
            break;

        case WorkToolBar::tbb_State:
            umlWidget = new StateWidget(m_pUMLView, StateWidget::Normal);
            break;

        case WorkToolBar::tbb_End_State:
            umlWidget = new StateWidget(m_pUMLView, StateWidget::End);
            break;

        default:
            break;
    }

    // Return false if we didn't find a suitable widget.
    if (umlWidget == NULL) {
        return false;
    }

    // Special treatment for some buttons
    if (getButton() == WorkToolBar::tbb_Activity) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter Activity Name"),
            i18n("Enter the name of the new activity:"), i18n("new activity"));
    } else if (getButton() == WorkToolBar::tbb_State) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter State Name"),
            i18n("Enter the name of the new state:"), i18n("new state"));
    } else if (getButton() == WorkToolBar::tbb_Text) {
        // It is pretty invisible otherwise.
        FloatingTextWidget* ft = (FloatingTextWidget*) umlWidget;
        ft->changeTextDlg();
    }

    // Create the widget. Some setup functions can remove the widget.
    if (umlWidget != NULL) {
        m_pUMLView->setupNewWidget(umlWidget);
    }

    return true;
}

#include "toolbarstateother.moc"
