/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstateother.h"

// app includes
#include "activitywidget.h"
#include "boxwidget.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "regionwidget.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "notewidget.h"
#include "object_factory.h"
#include "preconditionwidget.h"
#include "combinedfragmentwidget.h"
#include "statewidget.h"
#include "signalwidget.h"
#include "uml.h"
#include "umlview.h"
#include "umldoc.h"
#include "objectwidget.h"
#include "objectnodewidget.h"
#include "pinwidget.h"
#include "umlscene.h"
#include "model_utils.h"  // for ENUM_NAMES only

// kde includes
#include <klocale.h>
#include <kinputdialog.h>

#include <QtGui/QMouseEvent>

using namespace Uml;

/**
 * Creates a new ToolBarStateOther.
 * @param umlView The UMLView to use.
 */
ToolBarStateOther::ToolBarStateOther(UMLScene *umlScene)
  : ToolBarStatePool(umlScene)
{
}

/**
 * Destroys this ToolBarStateOther.
 */
ToolBarStateOther::~ToolBarStateOther()
{
}

/**
 * Overridden from base class to ignore associations and widgets and treat
 * them as empty spaces to create widgets on it.
 * Sets nothing.
 */
void ToolBarStateOther::setCurrentElement()
{
}

/**
 * Called when the release event happened on an empty space.
 * Associations, widgets and actual empty spaces are all treated as empty
 * spaces. It creates a new widget if the left button was released.
 * The widget to create depends on the type of the toolbar button selected.
 * If the widget is the visual representation of an UMLObject, the object
 * factory handles its creation. Otherwise, the widget is created using
 * newWidget().
 * The UMLView is resized to fit on all the items.
 */
void ToolBarStateOther::mouseReleaseEmpty()
{
    if (m_pMouseEvent->button() == Qt::LeftButton) {
        if (!newWidget()) {
            // Is UMLObject?

            m_pUMLScene->setCreateObject(true);
            Object_Factory::createUMLObject(getObjectType());
        }

        m_pUMLScene->resizeCanvasToItems();
    }
}

/**
 * Returns the object type of this tool.
 * @return The object type of this tool.
 */
UMLObject::ObjectType ToolBarStateOther::getObjectType()
{
    UMLObject::ObjectType ot;

    switch(getButton()) {
        case WorkToolBar::tbb_Actor:        ot = UMLObject::ot_Actor;          break;
        case WorkToolBar::tbb_UseCase:      ot = UMLObject::ot_UseCase;        break;
        case WorkToolBar::tbb_Class:        ot = UMLObject::ot_Class;          break;
        case WorkToolBar::tbb_Object:       ot = UMLObject::ot_Class;          break;  // Object is a class.
        case WorkToolBar::tbb_Package:      ot = UMLObject::ot_Package;        break;
        case WorkToolBar::tbb_Component:    ot = UMLObject::ot_Component;      break;
        case WorkToolBar::tbb_Node:         ot = UMLObject::ot_Node;           break;
        case WorkToolBar::tbb_Artifact:     ot = UMLObject::ot_Artifact;       break;
        case WorkToolBar::tbb_Interface:    ot = UMLObject::ot_Interface;      break;
        case WorkToolBar::tbb_Enum:         ot = UMLObject::ot_Enum;           break;
        case WorkToolBar::tbb_Entity:       ot = UMLObject::ot_Entity;         break;
        case WorkToolBar::tbb_Datatype:     ot = UMLObject::ot_Datatype;       break;
        case WorkToolBar::tbb_Category:     ot = UMLObject::ot_Category;       break;

        default:                            ot = UMLObject::ot_UMLObject;      break;
    }

    return ot;
}

/**
 * Creates and adds a new widget to the UMLView (if widgets of that type
 * don't have an associated UMLObject).
 * If the type of the widget doesn't use an UMLObject (for example, a note
 * or a box), it creates the widget, adds it to the view and returns true.
 * Otherwise, it returns false.
 *
 * @return True if the widget was created, false otherwise.
 * @todo Rename to something more clear. The name is a bit confusing.
 */
bool ToolBarStateOther::newWidget()
{
    UMLWidget* umlWidget = 0;

    switch (getButton()) {
        case WorkToolBar::tbb_Note:
            umlWidget = new NoteWidget(m_pUMLScene, NoteWidget::Normal);
            break;

        case WorkToolBar::tbb_Box:
            umlWidget = new BoxWidget(m_pUMLScene);
            break;

        case WorkToolBar::tbb_Text:
            umlWidget = new FloatingTextWidget(m_pUMLScene, Uml::TextRole::Floating, "");
            break;

        // Activity buttons
        case WorkToolBar::tbb_Initial_Activity:
            umlWidget = new ActivityWidget(m_pUMLScene, ActivityWidget::Initial);
            break;

        case WorkToolBar::tbb_Activity:
            umlWidget = new ActivityWidget(m_pUMLScene, ActivityWidget::Normal);
            break;

        case WorkToolBar::tbb_End_Activity:
            umlWidget = new ActivityWidget(m_pUMLScene, ActivityWidget::End);
            break;

        case WorkToolBar::tbb_Final_Activity:
            umlWidget = new ActivityWidget(m_pUMLScene, ActivityWidget::Final);
            break;

        case WorkToolBar::tbb_Branch:
            umlWidget = new ActivityWidget(m_pUMLScene, ActivityWidget::Branch);
            break;

        case WorkToolBar::tbb_Fork:
            umlWidget = new ForkJoinWidget(m_pUMLScene);
            break;

        case WorkToolBar::tbb_Initial_State:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Initial);
            break;

        case WorkToolBar::tbb_State:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Normal);
            break;

        case WorkToolBar::tbb_End_State:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::End);
            break;

        case WorkToolBar::tbb_StateFork:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Fork);
            break;

        case WorkToolBar::tbb_StateJoin:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Join);
            break;

        case WorkToolBar::tbb_Junction:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Junction);
            break;

        case WorkToolBar::tbb_DeepHistory:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::DeepHistory);
            break;

        case WorkToolBar::tbb_ShallowHistory:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::ShallowHistory);
            break;

        case WorkToolBar::tbb_Choice:
            umlWidget = new StateWidget(m_pUMLScene, StateWidget::Choice);
            break;

        case WorkToolBar::tbb_Send_Signal:
            umlWidget = new SignalWidget(m_pUMLScene, SignalWidget::Send);
            break;

        case WorkToolBar::tbb_Accept_Signal:
            umlWidget = new SignalWidget(m_pUMLScene, SignalWidget::Accept);
            break;

        case WorkToolBar::tbb_Accept_Time_Event:
            umlWidget = new SignalWidget(m_pUMLScene, SignalWidget::Time);
            break;

        case WorkToolBar::tbb_Region:
            umlWidget = new RegionWidget(m_pUMLScene);
            break;

        case WorkToolBar::tbb_Seq_Combined_Fragment:
            umlWidget = new CombinedFragmentWidget(m_pUMLScene);
            break;

        case WorkToolBar::tbb_Object_Node:
            umlWidget = new ObjectNodeWidget(m_pUMLScene, ObjectNodeWidget::Data);
            break;

        case WorkToolBar::tbb_PrePostCondition:
            umlWidget = new NoteWidget(m_pUMLScene, NoteWidget::Normal);
            break;

        default:
            break;
    }

    // Return false if we didn't find a suitable widget.
    if (umlWidget == 0) {
        return false;
    }

    // Special treatment for some buttons
    switch (getButton()) {
    case WorkToolBar::tbb_Activity:
        {
            Dialog_Utils::askNameForWidget(
                umlWidget, i18n("Enter Activity Name"),
                i18n("Enter the name of the new activity:"), i18n("new activity"));
        }
        break;
    case WorkToolBar::tbb_Accept_Signal:
    case WorkToolBar::tbb_Send_Signal:
        {
            Dialog_Utils::askNameForWidget(
                umlWidget, i18n("Enter Signal Name"),
                i18n("Enter Signal"), i18n("new Signal"));
        }
        break;
    case WorkToolBar::tbb_Accept_Time_Event:
        {
            Dialog_Utils::askNameForWidget(
                umlWidget, i18n("Enter Time Event Name"),
                i18n("Enter Time Event"), i18n("new time event"));
        }
        break;
    case WorkToolBar::tbb_Seq_Combined_Fragment:
        {
            dynamic_cast<CombinedFragmentWidget*>(umlWidget)->askNameForWidgetType(
                umlWidget, i18n("Enter Combined Fragment Name"),
                i18n("Enter the Combined Fragment"), i18n("new Combined Fragment"));
        }
        break;
    case WorkToolBar::tbb_State:
        {
            Dialog_Utils::askNameForWidget(
                umlWidget, i18n("Enter State Name"),
                i18n("Enter the name of the new state:"), i18n("new state"));
        }
        break;
    case WorkToolBar::tbb_Text:
        {
            // It is pretty invisible otherwise.
            FloatingTextWidget* ft = (FloatingTextWidget*) umlWidget;
            ft->showChangeTextDialog();
        }
        break;
    case WorkToolBar::tbb_Object_Node:
        {
            dynamic_cast<ObjectNodeWidget*>(umlWidget)->askForObjectNodeType(umlWidget);
        }
        break;
    case WorkToolBar::tbb_PrePostCondition:
        {
            dynamic_cast<NoteWidget*>(umlWidget)->askForNoteType(umlWidget);
        }
        break;
    default:
        uWarning() << "Unknown ToolBar_Buttons: " << QLatin1String(ENUM_NAME(WorkToolBar, WorkToolBar::ToolBar_Buttons, getButton()));
        break;
    }

    // Create the widget. Some setup functions can remove the widget.
    if (umlWidget != 0) {
        m_pUMLScene->setupNewWidget(umlWidget);
    }

    return true;
}

#include "toolbarstateother.moc"
