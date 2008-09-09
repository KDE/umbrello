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


using namespace Uml;

ToolBarStateOther::ToolBarStateOther(UMLScene *umlScene) : ToolBarStatePool(umlScene)
{
}

ToolBarStateOther::~ToolBarStateOther() {
}

/**
 * Sets nothing.
 * Overridden from base class to ignore associations and widgets and treat
 * them as empty spaces to create widgets on it.
 */
void ToolBarStateOther::setCurrentElement() {
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
 *
 * @return The object type of this tool.
 */
Uml::Object_Type ToolBarStateOther::getObjectType()
{
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
        case WorkToolBar::tbb_Category:     ot = ot_Category;       break;

        default:                            ot = ot_UMLObject;      break;
    }

    return ot;
}

// TODO: The name is a bit confusing.

/**
 * Creates and adds a new widget to the UMLView (if widgets of that type
 * don't have an associated UMLObject).
 * If the type of the widget doesn't use an UMLObject (for example, a note
 * or a box), it creates the widget, adds it to the view and returns true.
 * Otherwise, it returns false.
 *
 * @return True if the widget was created, false otherwise.
 * @todo rename to something more clear
 */
bool ToolBarStateOther::newWidget()
{
    NewUMLRectWidget* umlWidget = NULL;

    switch (getButton()) {
        case WorkToolBar::tbb_Note:
            umlWidget = new NoteWidget(NoteWidget::Normal);
            break;

        case WorkToolBar::tbb_Box:
            umlWidget = new BoxWidget();
            break;

        case WorkToolBar::tbb_Text:
            umlWidget = new FloatingTextWidget(tr_Floating);
            break;

        // Activity buttons
        case WorkToolBar::tbb_Initial_Activity:
            umlWidget = new ActivityWidget(ActivityWidget::Initial);
            break;

        case WorkToolBar::tbb_Activity:
            umlWidget = new ActivityWidget(ActivityWidget::Normal);
            break;

        case WorkToolBar::tbb_End_Activity:
            umlWidget = new ActivityWidget(ActivityWidget::End);
            break;

        case WorkToolBar::tbb_Final_Activity:
            umlWidget = new ActivityWidget(ActivityWidget::Final);
            break;

        case WorkToolBar::tbb_Branch:
            umlWidget = new ActivityWidget(ActivityWidget::Branch);
            break;

        case WorkToolBar::tbb_Fork:
        case WorkToolBar::tbb_StateFork:
            umlWidget = new ForkJoinWidget();
            break;

        case WorkToolBar::tbb_Initial_State:
            umlWidget = new StateWidget(StateWidget::Initial);
            break;

        case WorkToolBar::tbb_State:
            umlWidget = new StateWidget(StateWidget::Normal);
            break;

        case WorkToolBar::tbb_End_State:
            umlWidget = new StateWidget(StateWidget::End);
            break;

        case WorkToolBar::tbb_Send_Signal:
            umlWidget = new SignalWidget(SignalWidget::Send);
            break;

        case WorkToolBar::tbb_Accept_Signal:
            umlWidget = new SignalWidget(SignalWidget::Accept);
            break;

        case WorkToolBar::tbb_Accept_Time_Event:
            umlWidget = new SignalWidget(SignalWidget::Time);
            break;

        case WorkToolBar::tbb_Region:
            umlWidget = new RegionWidget();
            break;

        case WorkToolBar::tbb_Seq_Combined_Fragment:
            umlWidget = new CombinedFragmentWidget();
            break;

        case WorkToolBar::tbb_Object_Node:
            umlWidget = new ObjectNodeWidget(ObjectNodeWidget::Data);
            break;

        case WorkToolBar::tbb_PrePostCondition:
            umlWidget = new NoteWidget(NoteWidget::Normal);
            break;

        default:
            break;
    }

    // Return false if we didn't find a suitable widget.
    if (umlWidget == NULL) {
        return false;
    }

    // add the widget.
    m_pUMLScene->addItem(umlWidget);

    // Special treatment for some buttons
    if (getButton() == WorkToolBar::tbb_Activity) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter Activity Name"),
            i18n("Enter the name of the new activity:"), i18n("new activity"));
    } else if (getButton() == WorkToolBar::tbb_Accept_Signal
            || getButton() == WorkToolBar::tbb_Send_Signal) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter Signal Name"),
            i18n("Enter Signal"), i18n("new Signal"));
    } else if (getButton() == WorkToolBar::tbb_Accept_Time_Event) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter Time Event Name"),
            i18n("Enter Time Event"), i18n("new time event"));
    } else if (getButton() == WorkToolBar::tbb_Seq_Combined_Fragment) {
        dynamic_cast<CombinedFragmentWidget*>(umlWidget)->askNameForWidgetType(
            umlWidget, i18n("Enter Combined Fragment Name"),
            i18n("Enter the Combined Fragment"), i18n("new Combined Fragment"));
    } else if (getButton() == WorkToolBar::tbb_State) {
        Dialog_Utils::askNameForWidget(
            umlWidget, i18n("Enter State Name"),
            i18n("Enter the name of the new state:"), i18n("new state"));
    } else if (getButton() == WorkToolBar::tbb_Text) {
        // It is pretty invisible otherwise.
        FloatingTextWidget* ft = (FloatingTextWidget*) umlWidget;
        ft->showChangeTextDialog();
    } else if (getButton() == WorkToolBar::tbb_Object_Node) {
         dynamic_cast<ObjectNodeWidget*>(umlWidget)->askForObjectNodeType(umlWidget);
    } else if (getButton() == WorkToolBar::tbb_PrePostCondition) {
         dynamic_cast<NoteWidget*>(umlWidget)->askForNoteType(umlWidget);
    }

    // Create the widget. Some setup functions can remove the widget.
    if (umlWidget != NULL) {
        m_pUMLScene->setupNewWidget(umlWidget);
    }

    return true;
}

#include "toolbarstateother.moc"
