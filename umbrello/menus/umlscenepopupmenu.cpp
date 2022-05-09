/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlscenepopupmenu.h"

// app includes
#include "debug_utils.h"
#include "layoutgenerator.h"
#include "uml.h"
#include "umlscene.h"

// kde includes
#include <KLocalizedString>

UMLScenePopupMenu::UMLScenePopupMenu(QWidget *parent, UMLScene *scene)
  : ListPopupMenu(parent),
    m_scene(scene)
{
    const bool CHECKABLE = true;

    Uml::DiagramType::Enum type = scene->type();
    switch(type) {
        case Uml::DiagramType::UseCase:
        case Uml::DiagramType::Class:
        case Uml::DiagramType::Object:
        case Uml::DiagramType::State:
        case Uml::DiagramType::Activity:
        case Uml::DiagramType::Component:
        case Uml::DiagramType::Deployment:
        case Uml::DiagramType::EntityRelationship:
        case Uml::DiagramType::Sequence:
        case Uml::DiagramType::Collaboration:
            if (type == Uml::DiagramType::State && scene->widgetLink()) {
                if (scene->widgetLink()->isStateWidget()) {
                    insert(mt_ReturnToCombinedState);
                    addSeparator();
                } else if (scene->widgetLink()->isClassWidget()) {
                    insert(mt_ReturnToClass);
                    addSeparator();
                }
            }
            insertSubMenuNew(type);
            addSeparator();
            insert(mt_Undo);
            insert(mt_Redo);
            addSeparator();
            insert(mt_Cut);
            insert(mt_Copy);
            insert(mt_Paste);
            addSeparator();
            insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18n("Clear Diagram"));
            insert(mt_Export_Image);
            addSeparator();
            insertLayoutItems();
            insert(mt_SnapToGrid, i18n("Snap to Grid"), CHECKABLE);
            setActionChecked(mt_SnapToGrid, scene->snapToGrid());
            insert(mt_SnapComponentSizeToGrid, i18n("Snap Component Size to Grid"), CHECKABLE);
            setActionChecked(mt_SnapComponentSizeToGrid, scene->snapComponentSizeToGrid());
            insert(mt_ShowSnapGrid, i18n("Show Grid"), CHECKABLE);
            setActionChecked(mt_ShowSnapGrid, scene->isSnapGridVisible());
            insert(mt_ShowDocumentationIndicator, i18n("Show Documentation Indicator"), CHECKABLE);
            setActionChecked(mt_ShowDocumentationIndicator, scene->isShowDocumentationIndicator());
            insert(mt_Properties);
            break;
#ifndef CHECK_SWITCH
        default:
            break;
#endif
    }

    bool bCutState = UMLApp::app()->isCutCopyState();
    setActionEnabled(mt_Undo, UMLApp::app()->isUndoActionEnabled());
    setActionEnabled(mt_Redo, UMLApp::app()->isRedoActionEnabled());
    setActionEnabled(mt_Cut, bCutState);
    setActionEnabled(mt_Copy, bCutState);
    setActionEnabled(mt_Paste, UMLApp::app()->isPasteState());

    setupActionsData();

    if (IS_DEBUG_ENABLED())
        dumpActions(Uml::DiagramType::toString(type));
}

void UMLScenePopupMenu::insertLayoutItems()
{
    QList<MenuType> types;
    types << mt_Apply_Layout  << mt_Apply_Layout1
          << mt_Apply_Layout2 << mt_Apply_Layout3
          << mt_Apply_Layout4 << mt_Apply_Layout5
          << mt_Apply_Layout6 << mt_Apply_Layout7
          << mt_Apply_Layout8 << mt_Apply_Layout9;
    LayoutGenerator generator;
    if (generator.isEnabled()) {
        QHash<QString, QString> configFiles;
        if (LayoutGenerator::availableConfigFiles(m_scene, configFiles)) {
            int i = 0;
            foreach(const QString &key, configFiles.keys()) { // krazy:exclude=foreach
                if (i >= types.size())
                    break;
                if (key == QLatin1String("export") &&
                        !Settings::optionState().autoLayoutState.showExportLayout)
                    continue;
                insert(types[i], QPixmap(), i18n("apply '%1'", configFiles[key]));
                QAction* action = getAction(types[i]);
                QMap<QString, QVariant> map = action->data().toMap();
                map[toString(dt_ApplyLayout)] = QVariant(key);
                action->setData(QVariant(map));
                i++;
            }
            addSeparator();
        }
    }
    else {
        logWarn0("UMLScenePopupMenu::insertLayoutItems could not add autolayout entries "
                 "because graphviz was not found.");
    }
}

void UMLScenePopupMenu::insertSubMenuNew(Uml::DiagramType::Enum type)
{
    KMenu * menu = makeNewMenu();

    switch(type) {
        case Uml::DiagramType::UseCase:
            insert(mt_Actor, menu);
            insert(mt_UseCase, menu);
            break;
        case Uml::DiagramType::Class:
            insert(mt_Import_from_File, menu);
            insert(mt_Class, menu);
            insert(mt_Interface, menu);
            insert(mt_Datatype, menu);
            insert(mt_Enum, menu);
            insert(mt_Package, menu);
            break;
        case Uml::DiagramType::Object:
            insert(mt_Instance, menu);
            break;
        case Uml::DiagramType::State:
            insert(mt_Initial_State, menu);
            insert(mt_State, menu);
            insert(mt_End_State, menu);
            insert(mt_Junction, menu);
            insert(mt_DeepHistory, menu);
            insert(mt_ShallowHistory, menu);
            insert(mt_Choice, menu);
            insert(mt_StateFork, menu);
            insert(mt_StateJoin, menu);
            insert(mt_CombinedState, menu);
            break;
        case Uml::DiagramType::Activity:
            insert(mt_Initial_Activity, menu);
            insert(mt_Activity, menu);
            insert(mt_End_Activity, menu);
            insert(mt_Final_Activity, menu);
            insert(mt_Branch, menu);
            insert(mt_Fork, menu);
            insert(mt_Invoke_Activity, menu);
            insert(mt_Param_Activity, menu);
            insert(mt_Activity_Transition, menu);
            insert(mt_Exception, menu);
            insert(mt_PrePostCondition, menu);
            insert(mt_Send_Signal, menu);
            insert(mt_Accept_Signal, menu);
            insert(mt_Accept_Time_Event, menu);
            insert(mt_Region, menu);
            insert(mt_Pin, menu);
            insert(mt_Object_Node, menu);
            break;
        case Uml::DiagramType::Component:
            insert(mt_Subsystem, menu);
            insert(mt_Component, menu);
            insert(mt_InterfaceComponent, menu);
            insert(mt_Artifact, menu);
            break;
        case Uml::DiagramType::Deployment:
            insert(mt_Node, menu);
            break;
        case Uml::DiagramType::EntityRelationship:
            insert(mt_Entity, menu);
            insert(mt_Category, menu);
            break;
        case Uml::DiagramType::Sequence:
            insert(mt_Import_from_File, menu);
            insert(mt_Object, menu);
            if (m_scene->onWidgetLine(m_scene->pos())) {
                insert(mt_MessageCreation, menu);
                insert(mt_MessageDestroy, menu);
                insert(mt_MessageSynchronous, menu);
                insert(mt_MessageAsynchronous, menu);
                insert(mt_MessageLost, menu);
            } else if (m_scene->widgetOnDiagram(WidgetBase::wt_Object)){
                insert(mt_MessageFound, menu);
            }
            break;
        case Uml::DiagramType::Collaboration:
            insert(mt_Object, menu);
            break;
        default:
            delete menu;
            return;
    }
    insert(mt_Note, menu);
    insert(mt_FloatText, menu);
    addMenu(menu);
}
