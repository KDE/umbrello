/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "listpopupmenu.h"

// app includes
#include "activitywidget.h"
#include "associationline.h"
#include "associationwidget.h"
#include "category.h"
#include "classifier.h"
#include "classifierwidget.h"
#include "combinedfragmentwidget.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "folder.h"
#include "forkjoinwidget.h"
#include "layoutgenerator.h"
#include "model_utils.h"
#include "objectnodewidget.h"
#include "objectwidget.h"
#include "notewidget.h"
#include "pinportbase.h"
#include "preconditionwidget.h"
#include "signalwidget.h"
#include "statewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "widget_utils.h"
#include "widgetbase.h"

// kde includes
#include <KLocalizedString>
#include <kactioncollection.h>

DEBUG_REGISTER_DISABLED(ListPopupMenu)

static const bool CHECKABLE = true;

// uncomment to see not handled switch cases
//#define CHECK_SWITCH

class DebugMenu {
public:
    DebugMenu(ListPopupMenu::MenuType _m) : m(_m) {}
    DebugMenu(const QString & _m) : menu(_m) {}
    ListPopupMenu::MenuType m{ListPopupMenu::mt_Undefined};
    QString menu;
};

class ListPopupMenuPrivate {
public:
    QList<DebugMenu> debugActions;
    ~ListPopupMenuPrivate()
    {
        debugActions.clear();
    }
};

#define DEBUG_AddAction(m) d->debugActions.append(DebugMenu(m))
#define DEBUG_StartMenu(m) d->debugActions.append(DebugMenu(m->title() + QStringLiteral(" - start")))
#define DEBUG_EndMenu(m) d->debugActions.append(DebugMenu(m->title() + QStringLiteral(" - end")))

/**
 * Constructs the popup menu
 *
 * @param parent   The parent to ListPopupMenu.
 */
ListPopupMenu::ListPopupMenu(QWidget *parent)
  : QMenu(parent),
    d(new ListPopupMenuPrivate)
{
}

/**
 * Standard destructor.
 */
ListPopupMenu::~ListPopupMenu()
{
    qDeleteAll(m_actions);
    m_actions.clear();
    delete d;
}

QMenu *ListPopupMenu::newMenu(const QString &title, QWidget *widget)
{
    QMenu *menu = new QMenu(title, widget);
    DEBUG_StartMenu(menu);
    return menu;
}

void ListPopupMenu::addMenu(QMenu *menu)
{
    QMenu::addMenu(menu);
    DEBUG_EndMenu(menu);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m   The MenuType for which to insert a menu item.
 */
void ListPopupMenu::insert(MenuType m)
{
    insert(m, this);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The QMenu for which to insert a menu item.
 * @param s      The entry to be inserted from the action collection
 */
void ListPopupMenu::insertFromActionKey(const MenuType m, QMenu *menu, const QString &s)
{
    QAction* action = UMLApp::app()->actionCollection()->action(s);
    insert(m, menu, action->icon(), action->text());
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The QMenu for which to insert a menu item.
 */
void ListPopupMenu::insert(const MenuType m, QMenu* menu)
{
    // Preprocessor macro for List Popup Menu Insert Small Icon
#define LPMISI(IT, TXT) m_actions[m] = menu->addAction(Icon_Utils::SmallIcon(Icon_Utils::IT), TXT)
    // Preprocessor macro for List Popup Menu Insert Bar Icon
#define LPMIBI(IT, TXT) m_actions[m] = menu->addAction(Icon_Utils::BarIcon(Icon_Utils::IT), TXT)
    DEBUG_AddAction(m);
    Q_ASSERT(menu != 0);
    switch (m) {
    case mt_Accept_Signal:              LPMISI(it_Accept_Signal,         i18n("Accept Signal")); break;
    case mt_Accept_Time_Event:          LPMISI(it_Accept_TimeEvent,      i18n("Accept Time Event")); break;
    case mt_Activity:                   LPMISI(it_UseCase,               i18n("Activity...")); break;
    case mt_Activity_Transition:        LPMISI(it_Activity_Transition,   i18n("Activity Transition")); break;
    case mt_Actor:                      LPMISI(it_Actor,                 i18n("Actor")); break;
    //case mt_Actor:                      LPMISI(it_Actor,                 i18n("Actor...")); break;
    case mt_Artifact:                   LPMISI(it_Artifact,              i18n("Artifact")); break;
    //case mt_Artifact:                   LPMISI(it_Artifact,              i18n("Artifact...")); break;
    case mt_Attribute:                  LPMISI(it_Public_Attribute,      i18n("Attribute")); break;
    //case mt_Attribute:                  LPMISI(it_Public_Attribute,      i18n("Attribute...")); break;
    case mt_Branch:                     LPMISI(it_Branch,                i18n("Branch/Merge")); break;
    case mt_Category:                   LPMISI(it_Category,              i18n("Category")); break;
    //case mt_Category:                   LPMISI(it_Category,              i18n("Category...")); break;
    case mt_Change_Font:                LPMISI(it_Change_Font,           i18n("Change Font...")); break;
    case mt_CheckConstraint:            LPMISI(it_Constraint_Check,      i18n("Check Constraint...")); break;
    case mt_Choice:                     LPMISI(it_Choice_Rhomb,          i18n("Choice")); break;
    case mt_Class:                      LPMISI(it_Class,                 i18nc("new class menu item", "Class...")); break;
    case mt_Clone:                      LPMIBI(it_Duplicate,             i18nc("duplicate action", "Duplicate")); break;
    case mt_Collapse_All:               m_actions[m] = menu->addAction(i18n("Collapse All")); break;
    case mt_CombinedState:              LPMISI(it_State,                 i18nc("add new combined state", "Combined state...")); break;
    case mt_Component:                  LPMISI(it_Component,             i18n("Component")); break;
    //case mt_Component:                  LPMISI(it_Component,             i18n("Component...")); break;
    case mt_Component_Diagram:          insertFromActionKey(m, menu, QStringLiteral("new_component_diagram")); break;
    case mt_Component_Folder:           LPMIBI(it_Folder,                i18n("Folder")); break;
    case mt_Copy:                       LPMISI(it_Copy,                  i18n("Copy")); break;
    case mt_Cut:                        LPMISI(it_Cut,                   i18n("Cut")); break;
    case mt_Datatype:                   LPMISI(it_Datatype,              i18n("Datatype...")); break;
    case mt_DeepHistory:                LPMISI(it_History_Deep,          i18n("Deep History")); break;
    case mt_Delete:                     LPMISI(it_Delete,                i18n("Delete")); break;
    case mt_Deployment_Diagram:         insertFromActionKey(m, menu, QStringLiteral("new_deployment_diagram")); break;
    case mt_Deployment_Folder:          LPMIBI(it_Folder,                i18n("Folder")); break;
    case mt_EditCombinedState:          LPMISI(it_State,                 i18n("Edit combined state")); break;
    case mt_End_Activity:               LPMISI(it_EndState,              i18n("End Activity")); break;
    case mt_End_State:                  LPMISI(it_EndState,              i18n("End State")); break;
    case mt_Entity:                     LPMISI(it_Entity,                i18n("Entity")); break;
    //case mt_Entity:                     LPMISI(it_Entity,                i18n("Entity...")); break;
    case mt_EntityAttribute:            LPMISI(it_Entity_Attribute,      i18n("Entity Attribute...")); break;
    case mt_EntityRelationship_Diagram: insertFromActionKey(m, menu, QStringLiteral("new_entityrelationship_diagram")); break;
    case mt_EntityRelationship_Folder:  LPMIBI(it_Folder,                i18n("Folder")); break;
    case mt_Enum:                       LPMISI(it_Enum,                  i18n("Enum...")); break;
    case mt_EnumLiteral:                LPMISI(it_Enum_Literal,          i18n("Enum Literal...")); break;
    case mt_Exception:                  LPMISI(it_Exception,             i18n("Exception")); break;
    case mt_Expand_All:                 m_actions[m] = menu->addAction(i18n("Expand All")); break;
    case mt_Export_Image:               LPMISI(it_Export_Picture,        i18n("Export as Picture...")); break;
    case mt_Externalize_Folder:         m_actions[m] = menu->addAction(i18n("Externalize Folder...")); break;
    case mt_Fill_Color:                 LPMISI(it_Color_Fill,            i18n("Fill Color...")); break;
    case mt_Final_Activity:             LPMISI(it_Activity_Final,        i18n("Final Activity")); break;
    case mt_FlipHorizontal:             m_actions[m] = menu->addAction(i18n("Flip Horizontal")); break;
    case mt_FlipVertical:               m_actions[m] = menu->addAction(i18n("Flip Vertical")); break;
    case mt_FloatText:                  LPMISI(it_Text,                  i18n("Text Line...")); break;
    case mt_ForeignKeyConstraint:       LPMISI(it_Constraint_ForeignKey, i18n("Foreign Key Constraint...")); break;
    case mt_Fork:                       LPMISI(it_Fork_Join,             i18n("Fork")); break;
    case mt_GoToStateDiagram:           LPMISI(it_Remove,                i18n("Go to state diagram")); break;
    case mt_Hide_Destruction_Box:       LPMISI(it_Message_Destroy,       i18n("Hide destruction box")); break;
    case mt_Import_Class:               LPMIBI(it_Import_File,           i18n("Import File(s)...")); break;
    case mt_Import_Project:             LPMIBI(it_Import_Project,        i18n("Import from Directory...")); break;
    case mt_Import_from_File:           LPMISI(it_Import_File,           i18n("from file...")); break;
    case mt_Initial_Activity:           LPMISI(it_InitialState,          i18n("Initial Activity")); break;
    case mt_Initial_State:              LPMISI(it_InitialState,          i18n("Initial State")); break;
    case mt_Instance:                   LPMISI(it_Instance,              i18nc("new instance menu item", "Instance...")); break;
    case mt_InstanceAttribute:          LPMISI(it_Attribute_New,         i18n("New Attribute...")); break;
    case mt_Interface:                  LPMISI(it_Interface,             i18n("Interface")); break;
    case mt_InterfaceComponent:         LPMISI(it_Interface_Provider,    i18n("Interface")); break;
    case mt_InterfaceProvided:          LPMISI(it_Interface_Provider,    i18n("Provided interface")); break;
    case mt_InterfaceRequired:          LPMISI(it_Interface_Requirement, i18n("Required interface")); break;
    case mt_Internalize_Folder:         m_actions[m] = menu->addAction(i18n("Internalize Folder")); break;
    case mt_Junction:                   LPMISI(it_Junction,              i18n("Junction")); break;
    case mt_Line_Color:                 LPMISI(it_Color_Line,            i18n("Line Color...")); break;
    case mt_Logical_Folder:             LPMIBI(it_Folder,                i18n("Folder")); break;
    case mt_MessageAsynchronous:        LPMISI(it_Message_Async,         i18n("Asynchronous Message")); break;
    case mt_MessageCreation:            LPMISI(it_Message_Creation,      i18n("Creation Message")); break;
    case mt_MessageDestroy:             LPMISI(it_Message_Destroy,       i18n("Destroy Message")); break;
    case mt_MessageFound:               LPMISI(it_Message_Found,         i18n("Found Message")); break;
    case mt_MessageLost:                LPMISI(it_Message_Lost,          i18n("Lost Message")); break;
    case mt_MessageSynchronous:         LPMISI(it_Message_Sync,          i18n("Synchronous Message")); break;
    case mt_New_Activity:               LPMISI(it_State_Activity,        i18n("Activity...")); break;
    case mt_New_Attribute:              LPMISI(it_Attribute_New,         i18n("New Attribute...")); break;
    case mt_New_EntityAttribute:        LPMISI(it_Entity_Attribute_New,  i18n("New Entity Attribute...")); break;
    case mt_New_EnumLiteral:            LPMISI(it_Literal_New,           i18n("New Literal...")); break;
    case mt_New_InstanceAttribute:      LPMISI(it_Attribute_New,         i18n("New Attribute...")); break;
    case mt_New_Operation:              LPMISI(it_Operation_Public_New,  i18n("New Operation...")); break;
    case mt_New_Parameter:              LPMISI(it_Parameter_New,         i18n("New Parameter...")); break;
    case mt_New_Template:               LPMISI(it_Template_New,          i18n("New Template...")); break;
    case mt_Node:                       LPMISI(it_Node,                  i18n("Node")); break;
    //case mt_Node:                       LPMISI(it_Node,                  i18n("Node...")); break;
    case mt_Note:                       LPMISI(it_Note,                  i18n("Note...")); break;
    case mt_Object:                     LPMISI(it_Object,                i18n("Object...")); break;
    case mt_Object_Node:                LPMISI(it_Object_Node,           i18n("Object Node")); break;
    case mt_Open_File:                  LPMISI(it_File_Open,             i18n("Open file")); break;
    case mt_Operation:                  LPMISI(it_Public_Method,         i18n("Operation")); break;
    //case mt_Operation:                  LPMISI(it_Public_Method,       i18n("Operation...")); break;
    case mt_Package:                    LPMISI(it_Package,               i18n("Package...")); break;
    case mt_Paste:                      LPMISI(it_Paste,                 i18n("Paste")); break;
    case mt_Pin:                        LPMISI(it_Pin,                   i18n("Pin")); break;
    case mt_Port:                       LPMISI(it_Port,                  i18n("Port")); break;
    //case mt_Port:                       LPMISI(it_Port,                  i18n("Port...")); break;
    case mt_PrePostCondition:           LPMISI(it_Condition_PrePost,     i18n("Pre Post Condition")); break;
    case mt_PrimaryKeyConstraint:       LPMISI(it_Constraint_PrimaryKey, i18n("Primary Key Constraint...")); break;
    case mt_Properties:                 LPMISI(it_Properties,            i18n("Properties")); break;
    case mt_Redo:                       LPMISI(it_Redo,                  i18n("Redo")); break;
    case mt_Region:                     LPMISI(it_Region,                i18n("Region")); break;
    case mt_Remove:                     LPMISI(it_Remove,                i18n("Remove")); break;
    case mt_RemoveStateDiagram:         LPMISI(it_Remove,                i18n("Remove state diagram")); break;
    case mt_Rename:                     LPMISI(it_Rename,                i18n("Rename...")); break;
    case mt_Rename_Object:              insert(m, menu, i18n("Rename Object...")); break;
    case mt_ReturnToCombinedState:      LPMISI(it_Redo,                  i18n("Return to combined state")); break;
    case mt_ReturnToClass:              LPMISI(it_Redo,                  i18n("Return to class")); break;
    case mt_Reset_Label_Positions:      m_actions[m] = menu->addAction(i18n("Reset Label Positions")); break;
    case mt_Resize:                     insert(m, menu, i18n("Resize")); break;
    case mt_SelectStateDiagram:         LPMISI(it_Remove,                i18n("Select state diagram")); break;
    case mt_Send_Signal:                LPMISI(it_Send_Signal,           i18n("Send Signal")); break;
    case mt_ShallowHistory:             LPMISI(it_History_Shallow,       i18n("Shallow History")); break;
    case mt_Show:                       LPMISI(it_Show,                  i18n("Show")); break;
    case mt_Show_Destruction_Box:       LPMISI(it_Message_Destroy,       i18n("Show destruction box")); break;
    case mt_State:                      LPMISI(it_State,                 i18nc("add new state", "State...")); break;
    case mt_StateFork:                  LPMISI(it_Fork_State,            i18n("Fork")); break;
    case mt_StateJoin:                  LPMISI(it_Join,                  i18n("Join")); break;
    case mt_StateTransition:            LPMISI(it_State_Transition,      i18n("State Transition")); break;
    case mt_State_Diagram:              insertFromActionKey(m, menu, QStringLiteral("new_state_diagram")); break;
    case mt_Subsystem:                  LPMISI(it_Subsystem,             i18n("Subsystem")); break;
    //case mt_Subsystem:                  LPMISI(it_Subsystem,             i18n("Subsystem...")); break;
    case mt_Template:                   LPMISI(it_Template_Class,        i18n("Template")); break;
    //case mt_Template:                   LPMISI(it_Template_New,          i18n("Template...")); break;
    case mt_Undo:                       LPMISI(it_Undo,                  i18n("Undo")); break;
    case mt_UniqueConstraint:           LPMISI(it_Constraint_Unique,     i18n("Unique Constraint...")); break;
    case mt_UseCase:                    LPMISI(it_UseCase,               i18n("Use Case")); break;
    //case mt_UseCase:                    LPMISI(it_UseCase,               i18n("Use Case...")); break;
    case mt_UseCase_Diagram:            insertFromActionKey(m, menu, QStringLiteral("new_use_case_diagram")); break;
    case mt_UseCase_Folder:             LPMIBI(it_Folder,                i18n("Folder")); break;

    default:
        logWarn1("ListPopupMenu::insert called on unimplemented MenuType %1", toString(m));
        break;
    }
#undef LPMISI
#undef LPMIBI
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param icon   The icon for this action.
 * @param text   The text for this action.
 */
void ListPopupMenu::insert(const MenuType m, const QIcon & icon, const QString & text)
{
    DEBUG_AddAction(m);
    m_actions[m] = addAction(icon, text);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m           The MenuType for which to insert a menu item.
 * @param text        The text for this action.
 * @param checkable   Sets the action to checkable.
 */
void ListPopupMenu::insert(const MenuType m, const QString & text, const bool checkable)
{
    insert(m, this, text, checkable);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The QMenu for which to insert a menu item.
 * @param icon   The icon for this action.
 * @param text   The text for this action.
 */
void ListPopupMenu::insert(const MenuType m, QMenu* menu, const QIcon & icon, const QString & text)
{
    DEBUG_AddAction(m);
    m_actions[m] = menu->addAction(icon, text);
}

/**
 * Shortcut for the frequently used addAction() calls.
 *
 * @param m      The MenuType for which to insert a menu item.
 * @param menu   The QMenu for which to insert a menu item.
 * @param text   The text for this action.
 * @param checkable   Sets the action to checkable.
 */
void ListPopupMenu::insert(const MenuType m, QMenu* menu, const QString & text, const bool checkable)
{
    DEBUG_AddAction(m);
    m_actions[m] = menu->addAction(text);
    if (checkable) {
        QAction* action = getAction(m);
        if (action)
            action->setCheckable(checkable);
    }
}

/**
 * Shortcut for inserting standard model items (Class, Interface,
 * Datatype, Enum, Package) as well as diagram choices.
 *
 * @param folders   Set this true if folders shall be included as choices.
 * @param diagrams  Set this true if diagram types shall be included as choices.
 * @param packages  Set this true if packages shall be included as choices.
 */
void ListPopupMenu::insertContainerItems(bool folders, bool diagrams, bool packages)
{
    QMenu* menu = newMenu(i18nc("new container menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    insertContainerItems(menu, folders, diagrams, packages);
    addMenu(menu);
}

/**
 * Shortcut for inserting standard model items (Class, Interface,
 * Datatype, Enum, Package) as well as diagram choices.
 *
 * @param menu       Menu to add the menu entries
 * @param folders    Set this true if folders shall be included as choices.
 * @param diagrams   Set this true if diagram types shall be included as choices.
 * @param packages   Set this true if packages shall be included as choices.
 */
void ListPopupMenu::insertContainerItems(QMenu* menu, bool folders, bool diagrams, bool packages)
{
    if (folders)
        insert(mt_Logical_Folder, menu, Icon_Utils::BarIcon(Icon_Utils::it_Folder), i18n("Folder"));
    insert(mt_Class, menu);
    insert(mt_Interface, menu);
    insert(mt_Datatype, menu);
    insert(mt_Enum, menu);
    insert(mt_Instance, menu);
    if (packages)
        insert(mt_Package, menu);
    if (diagrams) {
        insertFromActionKey(mt_Class_Diagram, menu, QStringLiteral("new_class_diagram"));
        insertFromActionKey(mt_Sequence_Diagram, menu, QStringLiteral("new_sequence_diagram"));
        insertFromActionKey(mt_Collaboration_Diagram, menu, QStringLiteral("new_collaboration_diagram"));
        insertFromActionKey(mt_State_Diagram, menu, QStringLiteral("new_state_diagram"));
        insertFromActionKey(mt_Activity_Diagram, menu, QStringLiteral("new_activity_diagram"));
    }
}

/**
 * Inserts a menu item for an association related text
 * (such as name, role, multiplicity etc.)
 *
 * @param label   The menu text.
 * @param mt      The menu type.
 */
void ListPopupMenu::insertAssociationTextItem(const QString &label, MenuType mt)
{
    insert(mt, label);
    insert(mt_Change_Font);
    insert(mt_Reset_Label_Positions);
    insert(mt_Properties);
}

/**
 * Convenience method to extract the ListPopupMenu type from an action.
 * @param action   the action which was called
 * @return menu type enum value
 */
ListPopupMenu::MenuType ListPopupMenu::typeFromAction(QAction *action)
{
    ListPopupMenu *menu = ListPopupMenu::menuFromAction(action);
    if (menu) {
        return menu->getMenuType(action);
    }
    else {
        logError0("typeFromAction: Action's data field does not contain ListPopupMenu pointer!");
        return mt_Undefined;
    }
}

/**
 * Utility: Convert a MenuType value to an ObjectType value.
 */
UMLObject::ObjectType ListPopupMenu::convert_MT_OT(MenuType mt)
{
    UMLObject::ObjectType type =  UMLObject::ot_UMLObject;

    switch (mt) {
    case mt_UseCase:
        type = UMLObject::ot_UseCase;
        break;
    case mt_Actor:
        type = UMLObject::ot_Actor;
        break;
    case mt_Class:
        type = UMLObject::ot_Class;
        break;
    case mt_Datatype:
        type = UMLObject::ot_Datatype;
        break;
    case mt_Attribute:
        type = UMLObject::ot_Attribute;
        break;
    case mt_Interface:
        type = UMLObject::ot_Interface;
        break;
    case mt_Template:
        type = UMLObject::ot_Template;
        break;
    case mt_Enum:
        type = UMLObject::ot_Enum;
        break;
    case mt_EnumLiteral:
        type = UMLObject::ot_EnumLiteral;
        break;
    case mt_EntityAttribute:
        type = UMLObject::ot_EntityAttribute;
        break;
    case mt_Operation:
        type = UMLObject::ot_Operation;
        break;
    case mt_Category:
        type = UMLObject::ot_Category;
        break;
    case mt_InstanceAttribute:
        type = UMLObject::ot_InstanceAttribute;
        break;
    default:
        break;
    }
    return type;
}

/**
 * Returns the data from the given action to the given key.
 */
QVariant ListPopupMenu::dataFromAction(DataType key, QAction* action)
{
    QVariant data = action->data();
    QMap<QString, QVariant> map = data.toMap();
    return map[ListPopupMenu::toString(key)];
}

/**
 * Convenience method to extract the ListPopupMenu pointer stored in QAction
 * objects belonging to ListPopupMenu.
 */
ListPopupMenu* ListPopupMenu::menuFromAction(QAction *action)
{
    if (action) {
        QVariant value = dataFromAction(dt_MenuPointer, action);
        if (value.canConvert<ListPopupMenu*>()) {
            return qvariant_cast<ListPopupMenu*>(value);
        }
    }
    return 0;
}

/**
 * Create the 'new' menu
 * @return menu instance
 */
QMenu *ListPopupMenu::makeNewMenu()
{
    QMenu *menu = newMenu(i18nc("new sub menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    return menu;
}

/**
 * Creates a popup menu for a single category Object
 * @param category The UMLCategory for which the category menu is created
 */
void ListPopupMenu::insertSubMenuCategoryType(UMLCategory* category)
{
    QMenu* menu = newMenu(i18nc("category type sub menu", "Category Type"), this);
    insert(mt_DisjointSpecialisation, menu, i18n("Disjoint(Specialisation)"), CHECKABLE);
    insert(mt_OverlappingSpecialisation, menu, i18n("Overlapping(Specialisation)"), CHECKABLE);
    insert(mt_Union, menu, i18n("Union"), CHECKABLE);
    setActionChecked(mt_DisjointSpecialisation, category->getType()==UMLCategory::ct_Disjoint_Specialisation);
    setActionChecked(mt_OverlappingSpecialisation, category->getType()==UMLCategory::ct_Overlapping_Specialisation);
    setActionChecked(mt_Union, category->getType()==UMLCategory::ct_Union);
    addMenu(menu);
}

/**
 * Get the action from the menu type as index.
 */
QAction* ListPopupMenu::getAction(MenuType idx)
{
    return m_actions.value(idx, 0);
}

// /**
//  * Get the MenuType from the action.
//  */
// ListPopupMenu::MenuType ListPopupMenu::getMenuType(KAction* action)
// {
//     return m_actions.key(action);
// }

/**
 * Get the MenuType from the action.
 */
ListPopupMenu::MenuType ListPopupMenu::getMenuType(QAction* action)
{
    QList<MenuType> keyList = m_actions.keys(action);
    if (keyList.empty() || /* all key-value pairs are unique*/ keyList.count() > 1) {
        return mt_Undefined;
    } else {
        // we return the first (only) value
        return keyList.first();
    }
}

/**
 * Checks the action item.
 *
 * @param idx     The MenuType for which to check the menu item.
 * @param value   The value.
 */
void ListPopupMenu::setActionChecked(MenuType idx, bool value)
{
    QAction* action = getAction(idx);
    if (action && action->isCheckable()) {
        action->setChecked(value);
    }
    else {
        DEBUG() << "called on unknown MenuType " << toString(idx);
    }
}

/**
 * Enables the action item.
 *
 * @param idx     The MenuType for which to enable the menu item.
 * @param value   The value.
 */
void ListPopupMenu::setActionEnabled(MenuType idx, bool value)
{
    QAction* action = getAction(idx);
    if (action) {
        action->setEnabled(value);
    }
    else {
        DEBUG() << "called on unknown MenuType " << toString(idx);
    }
}

/**
 * Sets up actions added to the ListPopupMenu to have their data field set to
 * pointer to this ListPopupMenu object, so that this menu pointer can be
 * retrieved in UMLWidget::slotMenuSelection
 *
 * @note This might seem like an ugly hack, but this is the only solution which
 *       helped in avoiding storage of ListPopupMenu pointer in each UMLWidget.
 */
void ListPopupMenu::setupActionsData()
{
    for(QAction *action : m_actions) {
        QMap<QString, QVariant> map = action->data().toMap();
        map[toString(dt_MenuPointer)] = qVariantFromValue(this);
        action->setData(QVariant(map));
    }

}

/**
 * Convert enum MenuType to string.
 */
QString ListPopupMenu::toString(MenuType menu)
{
    return QLatin1String(ENUM_NAME(ListPopupMenu, MenuType, menu));
}

/**
 * Convert enum DataType to string.
 */
QString ListPopupMenu::toString(DataType data)
{
    return QLatin1String(ENUM_NAME(ListPopupMenu, DataType, data));
}

//QList<DebugMenu> &ListPopupMenu::debugActions()
//{
//    return d->debugActions;
//}

/**
 * dump collected actions
 * @param title optional menu title
 */
void ListPopupMenu::dumpActions(const QString &title)
{
    qDebug().nospace() << title;
    for(DebugMenu e : d->debugActions) {
        if (!e.menu.isEmpty())
            qDebug().nospace() << "  " << e.menu;
        else
            qDebug().nospace() << "    " << toString(e.m);
    }
}
