/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "widgetbasepopupmenu.h"

// app includes
#include "activitywidget.h"
#include "category.h"
#include "classifier.h"
#include "combinedfragmentwidget.h"
#include "debug_utils.h"
#include "entitywidget.h"
#include "floatingtextwidget.h"
#include "forkjoinwidget.h"
#include "interfacewidget.h"
#include "notewidget.h"
#include "objectwidget.h"
#include "objectnodewidget.h"
#include "pinportbase.h"
#include "statewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlscene.h"

// kde includes
#include <KLocalizedString>

static const bool CHECKABLE = true;

/**
 * Constructs the popup menu for a scene widget.
 *
 * @param parent   The parent to ListPopupMenu.
 * @param widget   The WidgetBase to represent a menu for.
 * @param multi    True if multiple items are selected.
 * @param uniqueType The type of widget shared by all selected widgets
 */
WidgetBasePopupMenu::WidgetBasePopupMenu(QWidget * parent, WidgetBase * widget, bool multi, WidgetBase::WidgetType uniqueType)
  : ListPopupMenu(parent)
{
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
 * Creates the "Show" submenu in the context menu of one classifier widget
 */
void WidgetBasePopupMenu::makeClassifierShowPopup(ClassifierWidget *c)
{
    WidgetBase::WidgetType type = c->baseType();

    QMenu* show = newMenu(i18n("Show"), this);
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
    setActionChecked(mt_Show_Stereotypes, c->showStereotype() != Uml::ShowStereoType::None);
    addMenu(show);
}

/**
 * Creates the "Show" submenu the context menu of multiple classifier widgets
 */
void WidgetBasePopupMenu::makeMultiClassifierShowPopup(WidgetBase::WidgetType type)
{
    QMenu* show = newMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));

    QMenu* attributes = newMenu(i18n("Attributes"), this);
    if (type == WidgetBase::wt_Class) {
        insert(mt_Show_Attributes_Selection, attributes, i18n("Show"));
        insert(mt_Hide_Attributes_Selection, attributes, i18n("Hide"));
        insert(mt_Show_Attribute_Signature_Selection, attributes, i18n("Show Signatures"));
        insert(mt_Hide_Attribute_Signature_Selection, attributes, i18n("Hide Signatures"));
    }
    show->addMenu(attributes);

    QMenu* operations = newMenu(i18n("Operations"), this);
    insert(mt_Show_Operations_Selection, operations, i18n("Show"));
    insert(mt_Hide_Operations_Selection, operations, i18n("Hide"));
    insert(mt_Show_Operation_Signature_Selection, operations, i18n("Show Signatures"));
    insert(mt_Hide_Operation_Signature_Selection, operations, i18n("Hide Signatures"));
    show->addMenu(operations);

    QMenu* visibility = newMenu(i18n("Visibility"), this);
    insert(mt_Show_Visibility_Selection, visibility, i18n("Show"));
    insert(mt_Hide_Visibility_Selection, visibility, i18n("Hide"));
    insert(mt_Hide_NonPublic_Selection, visibility, i18n("Hide Non-public members"));
    insert(mt_Show_NonPublic_Selection, visibility, i18n("Show Non-public members"));
    show->addMenu(visibility);

    QMenu* packages = newMenu(i18n("Packages"), this);
    insert(mt_Show_Packages_Selection, packages, i18n("Show"));
    insert(mt_Hide_Packages_Selection, packages, i18n("Hide"));
    show->addMenu(packages);

    if (type == WidgetBase::wt_Class) {
        QMenu* stereotypes = newMenu(i18n("Stereotypes"), this);
        insert(mt_Show_Stereotypes_Selection, stereotypes, i18n("Show"));
        insert(mt_Hide_Stereotypes_Selection, stereotypes, i18n("Hide"));
        show->addMenu(stereotypes);
    }
    addMenu(show);
}

/**
 * Inserts the menu actions for a widget
 *
 * @param widget widget to generate the menu for
 */
void WidgetBasePopupMenu::insertSingleSelectionMenu(WidgetBase* widget)
{
    WidgetBase::WidgetType type = widget->baseType();
    switch (type) {
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Category:
       {
         insertSubMenuNew(type);
         insertSubMenuCategoryType(widget->umlObject()->asUMLCategory());
         insertSubMenuColor(widget->useFillColor());
         insertStdItems(true, type);
         insert(mt_Rename);
         insert(mt_Change_Font);
         break;
       }
    case WidgetBase::wt_Class:
        {
            ClassifierWidget* c = widget->asClassifierWidget();
            if (!c)
                break;
            insertSubMenuNew(type);
            makeClassifierShowPopup(c);
            insertSubMenuColor(c->useFillColor());
            addSeparator();
            if (c->linkedDiagram())
                insert(mt_GoToStateDiagram);
            if (UMLApp::app()->document()->views(Uml::DiagramType::State).size() > 0)
                insert(mt_SelectStateDiagram);
            if (c->linkedDiagram())
                insert(mt_RemoveStateDiagram);
            insertStdItems(true, type);
            insert(mt_Rename);
            insert(mt_Change_Font);
            if (c->umlObject() && c->umlObject()->stereotype() == QStringLiteral("class-or-package")) {
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
        break;

    case WidgetBase::wt_Interface:
        {
            InterfaceWidget* c = widget->asInterfaceWidget();
            if (!c)
                break;
            insertSubMenuNew(type);
            makeClassifierShowPopup(c);
            insertSubMenuColor(c->useFillColor());
            insertStdItems(true, type);
            insert(mt_Rename);
            insert(mt_Change_Font);
            insert(mt_DrawAsCircle, i18n("Draw as Circle"), CHECKABLE);
            setActionChecked(mt_DrawAsCircle, c->visualProperty(ClassifierWidget::DrawAsCircle));
            insert(mt_ChangeToClass, i18n("Change into Class"));
            insert(mt_Properties);
        }
        break;

    case WidgetBase::wt_Instance:
        insertSubMenuNew(type);
        insert(mt_InstanceAttribute);
        insert(mt_Rename_Object);
        insert(mt_Rename, i18n("Rename Class..."));
        insertStdItems(true, type);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Enum:
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        insertStdItems(true, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Entity:
        insertSubMenuNew(type);
        insertSubMenuShowEntity(widget->asEntityWidget());
        insertSubMenuColor(widget->useFillColor());
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
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        insertStdItems(false, type);
        insert(mt_Rename);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Port:
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        insertStdItems(false);
        insert(mt_NameAsTooltip, i18n("Name as Tooltip"), true);
        {
            PinPortBase *pW = static_cast<PinPortBase*>(widget);
            FloatingTextWidget *ft = pW->floatingTextWidget();
            if (ft == 0)
                m_actions[mt_NameAsTooltip]->setChecked(true);
        }
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Object:
        //Used for sequence diagram and collaboration diagram widgets
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        if (widget->umlScene() &&
            widget->umlScene()->isSequenceDiagram()) {
            addSeparator();
            MenuType tabUp = mt_Up;
            insert(mt_Up, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Up), i18n("Move Up"));
            insert(mt_Down, Icon_Utils::SmallIcon(Icon_Utils::it_Arrow_Down), i18n("Move Down"));
            if (!(static_cast<ObjectWidget*>(widget))->canTabUp()) {
                setActionEnabled(tabUp, false);
            }
        }
        insertStdItems(true, type);
        insert(mt_Rename_Object);
        insert(mt_Rename, i18n("Rename Class..."));
        if (widget->asObjectWidget() &&
                widget->umlScene() &&
                widget->umlScene()->isSequenceDiagram()) {
            insert(widget->asObjectWidget()->showDestruction() ? mt_Hide_Destruction_Box : mt_Show_Destruction_Box);
        }
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Message:
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        insertStdItems(false, type);
        //insert(mt_Change_Font);
        //insert(mt_Operation, Icon_Utils::SmallIcon(Icon_Utils::it_Operation_New), i18n("New Operation..."));
        //insert(mt_Select_Operation, i18n("Select Operation..."));
        break;

    case WidgetBase::wt_Note:
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear note", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Resize);
        insert(mt_AutoResize, i18n("Auto resize"), CHECKABLE);
        insert(mt_Delete);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Box:
        insertSubMenuNew(type);
        insertStdItems(false, type);
        insert(mt_Line_Color);
        break;

    case WidgetBase::wt_State:
        {
            StateWidget* pState = static_cast< StateWidget *>(widget);
            switch (pState->stateType()) {
            case StateWidget::Combined:
                insert(mt_EditCombinedState);
                addSeparator();
                break;
            default:
                break;
            }

            insertSubMenuNew(type);
            insertSubMenuColor(widget->useFillColor());
            insertStdItems(true, type);
            switch (pState->stateType()) {
            case StateWidget::Combined:
                insert(mt_Change_Font);
                insert(mt_Properties);
                break;
            case StateWidget::Normal:
                insert(mt_Rename, i18n("Change State Name..."));
                insert(mt_Change_Font);
                insert(mt_Properties);
                break;
            case StateWidget::Fork:
            case StateWidget::Join:
                insert(pState->drawVertical() ? mt_FlipHorizontal : mt_FlipVertical);
                break;
            default:
                break;
            }
    }
        break;

    case WidgetBase::wt_ForkJoin:
        insertSubMenuNew(type);
        {
            ForkJoinWidget *pForkJoin = static_cast<ForkJoinWidget*>(widget);
            insert(pForkJoin->orientation() == Qt::Vertical ? mt_FlipHorizontal : mt_FlipVertical);
            insert(mt_Fill_Color);
        }
        break;

    case WidgetBase::wt_Activity:
        insertSubMenuNew(type);
        {
            ActivityWidget* pActivity = static_cast<ActivityWidget *>(widget);
            if(pActivity->activityType() == ActivityWidget::Normal
              || pActivity->activityType() == ActivityWidget::Invok
              || pActivity->activityType() == ActivityWidget::Param) {
                insertSubMenuColor(widget->useFillColor());
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
        insertSubMenuNew(type);
        {
            ObjectNodeWidget* objWidget = static_cast<ObjectNodeWidget *>(widget);
            if (objWidget->objectNodeType() == ObjectNodeWidget::Buffer
              || objWidget->objectNodeType() == ObjectNodeWidget::Data
              || objWidget->objectNodeType() == ObjectNodeWidget::Flow) {
                insertSubMenuColor(widget->useFillColor());
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
        insertSubMenuNew(type);
        insertSubMenuColor(widget->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear precondition", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        if (type == WidgetBase::wt_Pin) {
            insert(mt_NameAsTooltip, i18n("Name as Tooltip"), true);
            PinPortBase *pW = static_cast<PinPortBase*>(widget);
            FloatingTextWidget *ft = pW->floatingTextWidget();
            if (ft == 0)
                m_actions[mt_NameAsTooltip]->setChecked(true);
        }
        insert(mt_Delete);
        insert(mt_Change_Font);
        if (type == WidgetBase::wt_Precondition)
            insert(mt_Properties);
        break;

    case WidgetBase::wt_CombinedFragment:
        insertSubMenuNew(type);
        // for alternative and parallel combined fragments
        if ((static_cast<CombinedFragmentWidget*>(widget))->combinedFragmentType() == CombinedFragmentWidget::Alt ||
            (static_cast<CombinedFragmentWidget*>(widget))->combinedFragmentType() == CombinedFragmentWidget::Par) {
            insert(mt_AddInteractionOperand, i18n("Add Interaction Operand"));
            addSeparator();
        }
        insertSubMenuColor(widget->useFillColor());
        addSeparator();
        insert(mt_Cut);
        insert(mt_Copy);
        insert(mt_Paste);
        insert(mt_Clear, Icon_Utils::SmallIcon(Icon_Utils::it_Clear), i18nc("clear combined fragment", "Clear"));
        addSeparator();
        insert(mt_Rename, i18n("Change Text..."));
        insert(mt_Delete);
        insert(mt_Change_Font);
        insert(mt_Properties);
        break;

    case WidgetBase::wt_Text:
        insertSubMenuNew(type);
        switch((static_cast<FloatingTextWidget*>(widget))->textRole()) {
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
        logWarn1("WidgetBasePopupMenu::insertSingleSelectionMenu unhandled WidgetType %1",
                 WidgetBase::toString(type));
        break;
    }//end switch
}

/**
 * Inserts the menu actions that work on the whole selection of widgets
 */
void WidgetBasePopupMenu::insertMultiSelectionMenu(WidgetBase::WidgetType uniqueType)
{
    insertSubMenuAlign();

    QMenu* color = newMenu(i18nc("color menu", "Color"), this);
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

/**
 * Shortcut for the frequently used insert() calls.
 *
 * @param insertLeadingSeparator   Set this true if the group shall
 *                                 start with a separator.
 * @param type      The WidgetType for which to insert the menu items.
 *                  If no argument is supplied then a Rename item will be
 *                  included.
 */
void WidgetBasePopupMenu::insertStdItems(bool insertLeadingSeparator, WidgetBase::WidgetType type)
{
    if (insertLeadingSeparator)
        addSeparator();
    insert(mt_Cut);
    insert(mt_Copy);
    insert(mt_Paste);
    addSeparator();
    if (type == WidgetBase::wt_UMLWidget)
        insert(mt_Rename);
    else if (Model_Utils::isCloneable(type)) {
        insert(mt_Clone);
        insert(mt_Remove);
    } else
        insert(mt_Delete);
    insert(mt_Resize);
    insert(mt_AutoResize, i18n("Auto resize"), CHECKABLE);
}

/**
 * Add the align actions submenu
 */
void WidgetBasePopupMenu::insertSubMenuAlign()
{
    QMenu* alignment = newMenu(i18nc("align menu", "Align"), this);
    insert(mt_Align_Right, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_Right), i18n("Align Right"));
    insert(mt_Align_Left, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_Left), i18n("Align Left"));
    insert(mt_Align_Top, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_Top), i18n("Align Top"));
    insert(mt_Align_Bottom, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_Bottom), i18n("Align Bottom"));

    insert(mt_Align_VerticalMiddle, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalMiddle), i18n("Align Vertical Middle"));
    insert(mt_Align_HorizontalMiddle, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalMiddle), i18n("Align Horizontal Middle"));
    insert(mt_Align_VerticalDistribute, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalDistribute), i18n("Align Vertical Distribute"));
    insert(mt_Align_HorizontalDistribute, alignment, Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalDistribute), i18n("Align Horizontal Distribute"));

    addMenu(alignment);
}

/**
 * Shortcut for commonly used sub menu initializations.
 *
 * @param fc   The "Use Fill Color" is checked.
 */
void WidgetBasePopupMenu::insertSubMenuColor(bool fc)
{
    QMenu* color = newMenu(i18nc("color menu", "Color"), this);
    insert(mt_Line_Color, color);
    insert(mt_Fill_Color, color);
    insert(mt_Use_Fill_Color, color, i18n("Use Fill Color"), CHECKABLE);
    setActionChecked(mt_Use_Fill_Color, fc);
    addMenu(color);
}

/**
 * Shortcut for commonly used sub menu initializations.
 *
 * @param type   The widget type for which to set up the menu.
 */
void WidgetBasePopupMenu::insertSubMenuNew(WidgetBase::WidgetType type)
{
    QMenu * menu = makeNewMenu();

    switch (type) {
        case WidgetBase::wt_Activity:
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
        case WidgetBase::wt_Actor:
        case WidgetBase::wt_UseCase:
            insert(mt_Actor, menu);
            insert(mt_UseCase, menu);
            break;
        case WidgetBase::wt_Component:
            insert(mt_Subsystem, menu);
            insert(mt_Component, menu);
            if (Settings::optionState().generalState.uml2) {
                insert(mt_Port, menu);
                insert(mt_InterfaceProvided, menu);
                insert(mt_InterfaceRequired, menu);
            } else {
                insert(mt_InterfaceComponent, menu);
            }
            insert(mt_Artifact, menu);
            break;
        case WidgetBase::wt_Class:
            insert(mt_Attribute, menu);
            insert(mt_Operation, menu);
            insert(mt_Template, menu);
            insert(mt_Class, menu);
            insert(mt_Interface, menu);
            insert(mt_Datatype, menu);
            insert(mt_Enum, menu);
            insert(mt_State_Diagram, menu);
            break;
        case WidgetBase::wt_Interface:
            insert(mt_Operation, menu);
            insert(mt_Template, menu);
            insert(mt_Class, menu);
            insert(mt_Interface, menu);
            insert(mt_Datatype, menu);
            insert(mt_Enum, menu);
            break;
        case WidgetBase::wt_Entity:
            insert(mt_EntityAttribute, menu);
            insert(mt_PrimaryKeyConstraint, menu);
            insert(mt_UniqueConstraint, menu);
            insert(mt_ForeignKeyConstraint, menu);
            insert(mt_CheckConstraint, menu);
            break;
        case WidgetBase::wt_Enum:
            insert(mt_EnumLiteral, menu);
            break;
        case WidgetBase::wt_Port:
            insert(mt_InterfaceProvided, menu);
            insert(mt_InterfaceRequired, menu);
            break;
        case WidgetBase::wt_State:
            insert(mt_State, menu);
            insert(mt_End_State, menu);
            insert(mt_StateTransition, menu);
            insert(mt_Junction, menu);
            insert(mt_DeepHistory, menu);
            insert(mt_ShallowHistory, menu);
            insert(mt_Choice, menu);
            insert(mt_StateFork, menu);
            insert(mt_StateJoin, menu);
            insert(mt_CombinedState, menu);
            break;
        default:
            break;
    }
    insert(mt_Note, menu);
    insert(mt_FloatText, menu);

    addMenu(menu);
}

void WidgetBasePopupMenu::insertSubMenuShowEntity(EntityWidget *widget)
{
    QMenu* show = newMenu(i18n("Show"), this);
    show->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Show));

    insert(mt_Show_Attribute_Signature, show, i18n("Attribute Signature"), CHECKABLE);
    setActionChecked(mt_Show_Attribute_Signature, widget->showAttributeSignature());

    insert(mt_Show_Stereotypes, show, i18n("Stereotype"), CHECKABLE);
    setActionChecked(mt_Show_Stereotypes, widget->showStereotype());
    addMenu(show);
}
