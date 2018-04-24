/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "widgetbasepopupmenu.h"

// app includes
#include "activitywidget.h"
#include "classifier.h"
#include "combinedfragmentwidget.h"
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "notewidget.h"
#include "objectwidget.h"
#include "objectnodewidget.h"
#include "pinportbase.h"
#include "statewidget.h"
#include "uml.h"
#include "umllistview.h"
#include "umlscene.h"

// kde includes
#include <KLocalizedString>

static const bool CHECKABLE = true;

/**
 * Constructs the popup menu for a scene widget.
 *
 * @param parent   The parent to ListPopupMenu.
 * @param object   The WidgetBase to represent a menu for.
 * @param multi    True if multiple items are selected.
 * @param uniqueType The type of widget shared by all selected widgets
 */
WidgetBasePopupMenu::WidgetBasePopupMenu(QWidget * parent, WidgetBase * widget, bool multi, WidgetBase::WidgetType uniqueType)
  : ListPopupMenu(parent)
{
    m_isListView = false;
    m_TriggerObject.m_Widget = widget;
    m_TriggerObjectType = tot_Widget;

    if (!widget)
        return;

    if (multi) {
        insertMultiSelectionMenu(uniqueType);
    } else {
        insertSingleSelectionMenu(widget);
    }

    bool bCutState = UMLApp::app()->isCutCopyState();
    setActionEnabled(mt_Cut, bCutState);
    setActionEnabled(mt_Copy, bCutState);
    bool pasteAvailable = false;
    if (widget->isNoteWidget() &&
            UMLApp::app()->listView()->startedCopy()) {
        NoteWidget::s_pCurrentNote = widget->asNoteWidget();
        pasteAvailable = true;
    }
    setActionEnabled(mt_Paste, pasteAvailable);
    setActionChecked(mt_AutoResize, widget->autoResize());
    setupActionsData();
}

/**
 * return menu type for related Widget type
 * @param type widget type
 * @return menu type
 */
ListPopupMenu::MenuType WidgetBasePopupMenu::toMenuType(WidgetBase::WidgetType type)
{
    switch (type) {
      case WidgetBase::wt_Activity:         return ListPopupMenu::mt_Activity;
      case WidgetBase::wt_Actor:            return ListPopupMenu::mt_Actor;
      case WidgetBase::wt_Artifact:         return ListPopupMenu::mt_Artifact;
    //case WidgetBase::wt_Association:      return ListPopupMenu::mt_Association;
    //case WidgetBase::wt_Box:              return ListPopupMenu::mt_Box;
      case WidgetBase::wt_Category:         return ListPopupMenu::mt_Category;
      case WidgetBase::wt_Class:            return ListPopupMenu::mt_Class;
    //case WidgetBase::wt_CombinedFragment: return ListPopupMenu::mt_CombinedFragment;
      case WidgetBase::wt_Component:        return ListPopupMenu::mt_Component;
      case WidgetBase::wt_Datatype:         return ListPopupMenu::mt_Datatype;
      case WidgetBase::wt_Entity:           return ListPopupMenu::mt_Entity;
      case WidgetBase::wt_Enum:             return ListPopupMenu::mt_Enum;
    //case WidgetBase::wt_FloatingDashLine: return ListPopupMenu::mt_FloatingDashLine;
    //case WidgetBase::wt_ForkJoin:         return ListPopupMenu::mt_ForkJoin;
      case WidgetBase::wt_Instance:         return ListPopupMenu::mt_Instance;
      case WidgetBase::wt_Interface:        return ListPopupMenu::mt_Interface;
    //case WidgetBase::wt_Message:          return ListPopupMenu::mt_Message;
      case WidgetBase::wt_Node:             return ListPopupMenu::mt_Node;
      case WidgetBase::wt_Note:             return ListPopupMenu::mt_Note;
      case WidgetBase::wt_Object:           return ListPopupMenu::mt_Object;
    //case WidgetBase::wt_ObjectNode:       return ListPopupMenu::mt_ObjectNode;
      case WidgetBase::wt_Package:          return ListPopupMenu::mt_Package;
    //case WidgetBase::wt_Pin:              return ListPopupMenu::mt_Pin;
      case WidgetBase::wt_Port:             return ListPopupMenu::mt_Port;
    //case WidgetBase::wt_Precondition:     return ListPopupMenu::mt_Precondition;
    //case WidgetBase::wt_Region:           return ListPopupMenu::mt_Region;
    //case WidgetBase::wt_Signal:           return ListPopupMenu::mt_Signal;
      case WidgetBase::wt_State:            return ListPopupMenu::mt_State;
    //case WidgetBase::wt_Text:             return ListPopupMenu::mt_Text;
    //case WidgetBase::wt_UMLWidget:        return ListPopupMenu::mt_UMLWidget;
      case WidgetBase::wt_UseCase:          return ListPopupMenu::mt_UseCase;
    default:
        uWarning() << "unknown widget type " << type;
    }
    return ListPopupMenu::mt_Undefined;
}

/**
 * Creates a popup menu for a single class or interface widgets.
 */
void WidgetBasePopupMenu::makeClassifierPopup(ClassifierWidget *c)
{
    WidgetBase::WidgetType type = c->baseType();
    KMenu* menu = new KMenu(i18nc("new classifier menu", "New"), this);
    menu->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    if (type == WidgetBase::wt_Class)
        insert(mt_Attribute, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Attribute), i18n("Attribute..."));
    insert(mt_Operation, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method), i18n("Operation..."));
    insert(mt_Template, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n("Template..."));
    insert(mt_Class, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Class), i18nc("new class menu item", "Class..."));
    insert(mt_Interface, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Interface), i18n("Interface..."));
    insert(mt_Datatype, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Datatype), i18n("Datatype..."));
    insert(mt_Enum, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Enum), i18n("Enum..."));
    insert(mt_New_Connected_Note, menu, Icon_Utils::SmallIcon(Icon_Utils::it_Note), i18n("Note..."));
    addMenu(menu);

    makeClassifierShowPopup(c);

    insertSubMenuColor(c->useFillColor());
    insertStdItems(true, type);
    insert(mt_Rename);
    insert(mt_Change_Font);
    if (type == WidgetBase::wt_Interface) {
        insert(mt_DrawAsCircle, i18n("Draw as Circle"), CHECKABLE);
        setActionChecked(mt_DrawAsCircle, c->visualProperty(ClassifierWidget::DrawAsCircle));
        insert(mt_ChangeToClass, i18n("Change into Class"));
    } else if (type == WidgetBase::wt_Class && c->umlObject() && c->umlObject()->stereotype() == QLatin1String("class-or-package")) {
            insert(mt_ChangeToClass, i18n("Change into Class"));
            insert(mt_ChangeToPackage, i18n("Change into Package"));
    } else {
        insert(mt_Refactoring, Icon_Utils::SmallIcon(Icon_Utils::it_Refactor), i18n("Refactor"));
        insert(mt_ViewCode, Icon_Utils::SmallIcon(Icon_Utils::it_View_Code), i18n("View Code"));
        UMLClassifier *umlc = c->classifier();
        if (umlc->isAbstract() && umlc->getAttributeList().size() == 0)
            insert(mt_ChangeToInterface, i18n("Change into Interface"));
    }
    insert(mt_Properties);
}

/**
 * Creates the "Show" submenu in the context menu of one classifier widget
 */
void WidgetBasePopupMenu::makeClassifierShowPopup(ClassifierWidget *c)
{
    WidgetBase::WidgetType type = c->baseType();

    KMenu* show = new KMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));
#ifdef ENABLE_WIDGET_SHOW_DOC
    insert(mt_Show_Documentation, show, i18n("Documentation"), CHECKABLE);
    setActionChecked(mt_Show_Documentation, c->visualProperty(ClassifierWidget::ShowDocumentation));
#endif
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Attributes, show, i18n("Attributes"), CHECKABLE);
        setActionChecked(mt_Show_Attributes, c->visualProperty(ClassifierWidget::ShowAttributes));
    }
    insert(mt_Show_Operations, show, i18n("Operations"), CHECKABLE);
    setActionChecked(mt_Show_Operations, c->visualProperty(ClassifierWidget::ShowOperations));
    insert(mt_Show_Public_Only, show, i18n("Public Only"), CHECKABLE);
    setActionChecked(mt_Show_Public_Only, c->visualProperty(ClassifierWidget::ShowPublicOnly));
    insert(mt_Visibility, show, i18n("Visibility"), CHECKABLE);
    setActionChecked(mt_Visibility, c->visualProperty(ClassifierWidget::ShowVisibility));
    insert(mt_Show_Operation_Signature, show, i18n("Operation Signature"), CHECKABLE);
    bool sig = (c->operationSignature() == Uml::SignatureType::SigNoVis ||
                c->operationSignature() == Uml::SignatureType::ShowSig);
    setActionChecked(mt_Show_Operation_Signature, sig);
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Attribute_Signature, show, i18n("Attribute Signature"), CHECKABLE);
        sig = (c->attributeSignature() == Uml::SignatureType::SigNoVis ||
               c->attributeSignature() == Uml::SignatureType::ShowSig);
        setActionChecked(mt_Show_Attribute_Signature, sig);
    }
    insert(mt_Show_Packages, show, i18n("Package"), CHECKABLE);
    setActionChecked(mt_Show_Packages, c->visualProperty(ClassifierWidget::ShowPackage));
    insert(mt_Show_Stereotypes, show, i18n("Stereotype"), CHECKABLE);
    setActionChecked(mt_Show_Stereotypes, c->visualProperty(ClassifierWidget::ShowStereotype));
    addMenu(show);
}

/**
 * Creates the "Show" submenu the context menu of multiple classifier widgets
 */
void WidgetBasePopupMenu::makeMultiClassifierShowPopup(WidgetBase::WidgetType type)
{
    KMenu* show = new KMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));

    KMenu* attributes = new KMenu(i18n("Attributes"), this);
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Attributes_Selection, attributes, i18n("Show"));
        insert(mt_Hide_Attributes_Selection, attributes, i18n("Hide"));
        insert(mt_Show_Attribute_Signature_Selection, attributes, i18n("Show Signatures"));
        insert(mt_Hide_Attribute_Signature_Selection, attributes, i18n("Hide Signatures"));
    }
    show->addMenu(attributes);

    KMenu* operations = new KMenu(i18n("Operations"), this);
    insert(mt_Show_Operations_Selection, operations, i18n("Show"));
    insert(mt_Hide_Operations_Selection, operations, i18n("Hide"));
    insert(mt_Show_Operation_Signature_Selection, operations, i18n("Show Signatures"));
    insert(mt_Hide_Operation_Signature_Selection, operations, i18n("Hide Signatures"));
    show->addMenu(operations);

    KMenu* visibility = new KMenu(i18n("Visibility"), this);
    insert(mt_Show_Visibility_Selection, visibility, i18n("Show"));
    insert(mt_Hide_Visibility_Selection, visibility, i18n("Hide"));
    insert(mt_Hide_NonPublic_Selection, visibility, i18n("Hide Non-public members"));
    insert(mt_Show_NonPublic_Selection, visibility, i18n("Show Non-public members"));
    show->addMenu(visibility);

    KMenu* packages = new KMenu(i18n("Packages"), this);
    insert(mt_Show_Packages_Selection, packages, i18n("Show"));
    insert(mt_Hide_Packages_Selection, packages, i18n("Hide"));
    show->addMenu(packages);

    if (type == WidgetBase::wt_Class) {
        KMenu* stereotypes = new KMenu(i18n("Stereotypes"), this);
        insert(mt_Show_Stereotypes_Selection, stereotypes, i18n("Show"));
        insert(mt_Hide_Stereotypes_Selection, stereotypes, i18n("Hide"));
        show->addMenu(stereotypes);
    }
    addMenu(show);
}

/**
 * Inserts the menu actions for a widget
 *
 * @param WidgetBase* object
 */
void WidgetBasePopupMenu::insertSingleSelectionMenu(WidgetBase* object)
{
    WidgetBase::WidgetType type = object->baseType();
    if (!object->umlObject())
        insert(mt_New_Connected_Note, i18n("New connected note"));
    switch (type) {
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Category:
       {
         KMenu* m = makeCategoryTypeMenu(object->umlObject()->asUMLCategory());
         m->setTitle(i18n("Category Type"));
         addMenu(m);
         insertSubMenuColor(object->useFillColor());
         insertStdItems(true, type);
         insert(mt_Rename);
         insert(mt_Change_Font);
         break;
       }
    case WidgetBase::wt_Class:
    case WidgetBase::wt_Interface:
        {
            ClassifierWidget* classifier = object->asClassifierWidget();
            if (classifier)
                makeClassifierPopup(classifier);
        }
        break;

    case WidgetBase::wt_Instance:
        insert(mt_InstanceAttribute);
        insert(mt_Rename_Object);
        insert(mt_Rename, i18n("Rename Class..."));
        insertStdItems(true, type);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Enum:
        insertSubMenuNew(mt_Enum);
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Entity:
        insertSubMenuNew(mt_Entity);
        insertSubMenuShowEntity(object->asEntityWidget());
        insertSubMenuColor(object->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Datatype:
    case WidgetBase::wt_Package:
    case WidgetBase::wt_Component:
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(false, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Port:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(false);
        insert(mt_NameAsTooltip, i18n("Name as Tooltip"), CHECKABLE);
        {
            PinPortBase *pW = static_cast<PinPortBase*>(object);
            FloatingTextWidget *ft = pW->floatingTextWidget();
            if (ft == 0)
                m_actions[mt_NameAsTooltip]->setChecked(true);
        }
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Object:
        //Used for sequence diagram and collaboration diagram widgets
        insertSubMenuNew(mt_Object);
        insertSubMenuColor(object->useFillColor());
        if (object->umlScene() &&
            object->umlScene()->type() == Uml::DiagramType::Sequence) {
            addSeparator();
            MenuType tabUp = mt_Up;
            insert(mt_Up, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Up), i18n("Move Up"));
            insert(mt_Down, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Down), i18n("Move Down"));
            if (!(static_cast<ObjectWidget*>(object))->canTabUp()) {
                setActionEnabled(tabUp, false);
            }
        }
        insertStdItems(true, type);
        insert(mt_Rename_Object);
        insert(mt_Rename, i18n("Rename Class..."));
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Message:
        insertSubMenuColor(object->useFillColor());
        insertStdItems(false, type);
        //insert(mt_Change_Font);
        //insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
        //insert(mt_Select_Operation, i18n("Select Operation..."));
        break;

    case WidgetBase::wt_Note:
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear note", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Box:
        insertStdItems(false, type);
        insert(mt_Line_Color);
        break;

    case WidgetBase::wt_State:
        {
            StateWidget* pState = static_cast< StateWidget *>(object);
            if (pState->stateType() == StateWidget::Normal) {
                insertSubMenuNew(mt_New_Activity);
            }
            insertSubMenuColor(object->useFillColor());
            insertStdItems(false, type);
            switch (pState->stateType()) {
            case StateWidget::Normal:
                insert(mt_Rename, i18n("Change State Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
                break;
            case StateWidget::Fork:
            case StateWidget::Join:
                if (pState->drawVertical())
                    insert(mt_Flip, i18n("Flip Horizontal"));
                else
                    insert(mt_Flip, i18n("Flip Vertical"));
                break;
            default:
                break;
            }
    }
        break;

    case WidgetBase::wt_ForkJoin:
        {
            ForkJoinWidget *pForkJoin = static_cast<ForkJoinWidget*>(object);
            if (pForkJoin->orientation() == Qt::Vertical) {
                insert(mt_Flip, i18n("Flip Horizontal"));
            }
            else {
                insert(mt_Flip, i18n("Flip Vertical"));
            }
            m_actions[mt_Fill_Color] = addAction(Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill),
                                                 i18n("Fill Color..."));
        }
        break;

    case WidgetBase::wt_Activity:
        {
            ActivityWidget* pActivity = static_cast<ActivityWidget *>(object);
            if(pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insertSubMenuColor(object->useFillColor());
            }
            insertStdItems(false, type);
            if(pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insert(mt_Rename, i18n("Change Activity Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case WidgetBase::wt_ObjectNode:
        {
            ObjectNodeWidget* objWidget = static_cast<ObjectNodeWidget *>(object);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
              || objWidget->objectNodeType() == ObjectNodeWidget::Data
              || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insertSubMenuColor(object->useFillColor());
            }
            insertStdItems(false, type);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
                || objWidget->objectNodeType() == ObjectNodeWidget::Data
                || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insert(mt_Rename, i18n("Change Object Node Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
            }
        }
        break;

    case WidgetBase::wt_Pin:
    case WidgetBase::wt_Signal:
    case WidgetBase::wt_FloatingDashLine:
    case WidgetBase::wt_Precondition:
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear precondition", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        if (type == WidgetBase::wt_Pin) {
            insert(mt_NameAsTooltip, i18n("Name as Tooltip"), CHECKABLE);
            PinPortBase *pW = static_cast<PinPortBase*>(object);
            FloatingTextWidget *ft = pW->floatingTextWidget();
            if (ft == 0)
                m_actions[mt_NameAsTooltip]->setChecked(true);
        }
        insert(mt_Delete);
        insert(mt_Change_Font);
        break;

    case WidgetBase::wt_CombinedFragment:
        // for alternative and parallel combined fragments
        if ((static_cast<CombinedFragmentWidget*>(object))->combinedFragmentType() == CombinedFragmentWidget::Alt ||
            (static_cast<CombinedFragmentWidget*>(object))->combinedFragmentType() == CombinedFragmentWidget::Par) {
            insert(mt_AddInteractionOperand, i18n("Add Interaction Operand"));
            addSeparator();
        }
        insertSubMenuColor(object->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear combined fragment", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        break;

    case WidgetBase::wt_Text:
        switch((static_cast<FloatingTextWidget*>(object))->textRole()) {
        case Uml::TextRole::MultiB:
            insertAssociationTextItem(i18n("Change Multiplicity..."), mt_Rename_MultiB);
            break;
        case Uml::TextRole::MultiA:
            insertAssociationTextItem(i18n("Change Multiplicity..."), mt_Rename_MultiA);
            break;
        case Uml::TextRole::Name:
            insertAssociationTextItem(i18n("Change Name"), mt_Rename_Name);
            break;
        case Uml::TextRole::RoleAName:
            insertAssociationTextItem(i18n("Change Role A Name..."), mt_Rename_RoleAName);
            break;
        case Uml::TextRole::RoleBName:
            insertAssociationTextItem(i18n("Change Role B Name..."), mt_Rename_RoleBName);
            break;
        case Uml::TextRole::ChangeA:
        case Uml::TextRole::ChangeB:
            insert(mt_Change_Font);
            insert(mt_Reset_Label_Positions);
            insert(mt_Properties);
            break;

        case Uml::TextRole::Coll_Message_Self:
        case Uml::TextRole::Coll_Message:
        case Uml::TextRole::Seq_Message_Self:
        case Uml::TextRole::Seq_Message:
            insert(mt_Change_Font);
            insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
            insert(mt_Select_Operation, i18n("Select Operation..."));
            break;

        case Uml::TextRole::Floating:
        default:
            insertStdItems(false, type);
            insert(mt_Rename, i18n("Change Text..."));
            insert(mt_Change_Font);
            break;
        }
        break;
    default:
        uWarning() << "unhandled WidgetType " << WidgetBase::toString(type);
        break;
    }//end switch
}

/**
 * Inserts the menu actions that work on the whole selection of widgets
 */
void WidgetBasePopupMenu::insertMultiSelectionMenu(WidgetBase::WidgetType uniqueType)
{
    insertSubMenuAlign();

    KMenu* color = new KMenu(i18nc("color menu", "Color"), this);
    insert(mt_Line_Color_Selection, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Line), i18n("Line Color..."));
    insert(mt_Fill_Color_Selection, color, Icon_Utils::SmallIcon(Icon_Utils::it_Color_Fill), i18n("Fill Color..."));
    insert(mt_Set_Use_Fill_Color_Selection, color, i18n("Use Fill Color"));
    insert(mt_Unset_Use_Fill_Color_Selection, color, i18n("No Fill Color"));

    // Add menu actions specific to classifiers
    if (uniqueType == WidgetBase::wt_Class ||
        uniqueType == WidgetBase::wt_Interface) {
        makeMultiClassifierShowPopup(uniqueType);
    }

    addMenu(color);

    addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);

    addSeparator();
    insert(mt_Clone);
    insert(mt_Delete);
    insert(mt_Resize);

    addSeparator();
    insert(mt_Change_Font_Selection, Icon_Utils::SmallIcon(Icon_Utils::it_Change_Font), i18n("Change Font..."));
}

