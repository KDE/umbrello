/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

    // FIXME: this is not true
    m_isListView = true;
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
            insertSubMenuNew(type);
            addSeparator();
            insert(mt_Undo, Icon_Utils::SmallIcon(Icon_Utils::it_Undo), i18n("Undo"));
            insert(mt_Redo, Icon_Utils::SmallIcon(Icon_Utils::it_Redo), i18n("Redo"));
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

    if (IS_DEBUG_ENABLED(DBG_SRC))
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
        uWarning() << "Could not add autolayout entries because graphviz installation has not been found.";
    }
}

void UMLScenePopupMenu::insertSubMenuNew(Uml::DiagramType::Enum type, KMenu *menu)
{
    if (!menu) {
        menu = makeNewMenu();
    }
    switch(type) {
        case Uml::DiagramType::UseCase:
            insert(mt_Actor, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Actor), i18n("Actor..."));
            insert(mt_UseCase, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18n("Use Case..."));
            break;
        case Uml::DiagramType::Class:
            insert(mt_Import_from_File, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Import_File), i18n("from file..."));
            insert(mt_Class, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Class), i18nc("new class menu item", "Class..."));
            insert(mt_Interface, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Interface), i18n("Interface..."));
            insert(mt_Datatype, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Datatype), i18n("Datatype..."));
            insert(mt_Enum, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Enum), i18n("Enum..."));
            insert(mt_Package, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Package), i18n("Package..."));
            break;
         case Uml::DiagramType::Object:
            insert(mt_Instance, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Instance), i18nc("new instance menu item", "Instance..."));
        break;
        case Uml::DiagramType::State:
            insert(mt_Initial_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_InitialState), i18n("Initial State"));
            insert(mt_End_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_EndState), i18n("End State"));
            insert(mt_State, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18nc("add new state", "State..."));
            insert(mt_Junction, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Junction), i18n("Junction"));
            insert(mt_DeepHistory, menu, Icon_Utils::SmallIcon(Icon_Utils::it_History_Deep), i18n("Deep History"));
            insert(mt_ShallowHistory, menu, Icon_Utils::SmallIcon(Icon_Utils::it_History_Shallow), i18n("Shallow History"));
            insert(mt_Choice, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Choice_Rhomb), i18n("Choice"));
            insert(mt_StateFork, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Fork_State), i18n("Fork"));
            insert(mt_StateJoin, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Join), i18n("Join"));
            break;
        case Uml::DiagramType::Activity:
            insert(mt_Initial_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_InitialState), i18n("Initial Activity"));
            insert(mt_End_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_EndState), i18n("End Activity"));
            insert(mt_Activity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_UseCase), i18n("Activity..."));
            insert(mt_Branch, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Branch), i18n("Branch/Merge"));
            break;
        case Uml::DiagramType::Component:
            insert(mt_Subsystem, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Subsystem), i18n("Subsystem..."));
            insert(mt_Component, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Component), i18n("Component..."));
            if (Settings::optionState().generalState.uml2)
                insert(mt_Port, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Port), i18n("Port..."));
            insert(mt_Artifact, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Artifact), i18n("Artifact..."));
            break;
        case Uml::DiagramType::Deployment:
            insert(mt_Node, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Node), i18n("Node..."));
            break;
        case Uml::DiagramType::EntityRelationship:
            insert(mt_Entity, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Entity), i18n("Entity..."));
            insert(mt_Category, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Category), i18n("Category..."));
            break;
        case Uml::DiagramType::Sequence:
            insert(mt_Import_from_File, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Import_File), i18n("from file..."));
            insert(mt_Object, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Object), i18n("Object..."));
            break;
        case Uml::DiagramType::Collaboration:
            insert(mt_Object, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Object), i18n("Object..."));
            break;
        default:
            delete menu;
            return;
    }
    addMenu(menu);
}
