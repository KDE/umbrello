/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "associationwidgetpopupmenu.h"

// app includes
#include "assocrules.h"
#include "associationline.h"
#include "associationwidget.h"
#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

enum class LocalTriggerType { AnchorSelected, AssociationSelected, CollaborationMessage, AttributeAssociation, FullAssociation };
/**
 * Constructs the popup menu for a scene widget.
 *
 * @param parent   The parent to ListPopupMenu.
 * @param type     The type of widget shared by all selected widgets.
 * @param widget   The AssociationWidget to represent a menu for.
 */
AssociationWidgetPopupMenu::AssociationWidgetPopupMenu(QWidget *parent, Uml::AssociationType::Enum type, AssociationWidget *widget)
  : ListPopupMenu(parent)
{
    LocalTriggerType triggerType = LocalTriggerType::AssociationSelected;
    if (type == Uml::AssociationType::Anchor) {
        triggerType = LocalTriggerType::AnchorSelected;
    } else if (widget->isCollaboration()) {
        triggerType = LocalTriggerType::CollaborationMessage;
    } else if (!widget->association()) {
        triggerType = LocalTriggerType::AttributeAssociation;
    } else if (AssocRules::allowRole(type)) {
        triggerType = LocalTriggerType::FullAssociation;
    }

    switch (triggerType) {
    case LocalTriggerType::AnchorSelected:
        insert(mt_Delete, Icon_Utils::SmallIcon(Icon_Utils::it_Delete), i18n("Delete Anchor"));
        break;
    case LocalTriggerType::AssociationSelected:
    case LocalTriggerType::AttributeAssociation:
    case LocalTriggerType::FullAssociation:
    case LocalTriggerType::CollaborationMessage:
        if (widget->isPointAddable())
            insert(mt_Add_Point, Icon_Utils::SmallIcon(Icon_Utils::it_Add_Point), i18n("Add Point"));
        if (widget->isPointRemovable())
            insert(mt_Delete_Point, Icon_Utils::SmallIcon(Icon_Utils::it_Delete_Point), i18n("Delete Point"));
        if (!widget->isAutoLayouted())
            insert(mt_Auto_Layout_Spline, i18n("Choose Spline points automatically"));
        if (widget->isLayoutChangeable()) {
            addSeparator();
            insertSubMenuLayout(widget->associationLine());
        }
        addSeparator();
        insert(mt_Delete);
        break;
    default:
        break;
    }

    switch(triggerType) {
    case LocalTriggerType::AttributeAssociation:
    case LocalTriggerType::FullAssociation:
        insert(mt_Rename_Name, i18n("Change Association Name..."));
        insert(mt_Rename_RoleAName, i18n("Change Role A Name..."));
        insert(mt_Rename_RoleBName, i18n("Change Role B Name..."));
        insert(mt_Change_Font);
        insert(mt_Reset_Label_Positions);
        break;
    case LocalTriggerType::CollaborationMessage:
        insert(mt_Change_Font);
        insert(mt_New_Operation);
        insert(mt_Select_Operation, i18n("Select Operation..."));
        break;
    default:
        break;
    }

    insert(mt_Line_Color);
    insert(mt_Properties);

    setActionChecked(mt_AutoResize, widget->autoResize());
    setupActionsData();
}

/**
 * Inserts a sub menu for association layouts.
 */
void AssociationWidgetPopupMenu::insertSubMenuLayout(const AssociationLine& associationLine)
{
    KMenu* layout = newMenu(i18nc("Layout menu", "Layout"), this);
    insert(mt_LayoutPolyline, layout, i18n("Polyline"), true);
    insert(mt_LayoutDirect, layout, i18n("Direct"), true);
    insert(mt_LayoutSpline, layout, i18n("Spline"), true);
    insert(mt_LayoutOrthogonal, layout, i18n("Orthogonal"), true);
    switch (associationLine.layout()) {
    case Uml::LayoutType::Direct:
        m_actions[mt_LayoutDirect]->setChecked(true);
        break;
    case Uml::LayoutType::Orthogonal:
        m_actions[mt_LayoutOrthogonal]->setChecked(true);
        break;
    case Uml::LayoutType::Spline:
        m_actions[mt_LayoutSpline]->setChecked(true);
        break;
    case Uml::LayoutType::Polyline:
    default:
        m_actions[mt_LayoutPolyline]->setChecked(true);
        break;
    }
    addMenu(layout);
}
