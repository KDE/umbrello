/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
 * @param object   The WidgetBase to represent a menu for.
 * @param multi    True if multiple items are selected.
 * @param uniqueType The type of widget shared by all selected widgets
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
        addSeparator();
        insertSubMenuLayout(widget->associationLine());
        addSeparator();
        insert(mt_Delete);
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
void AssociationWidgetPopupMenu::insertSubMenuLayout(AssociationLine *associationLine)
{
    KMenu* layout = new KMenu(i18nc("Layout menu", "Layout"), this);
    insert(mt_LayoutPolyline, layout, i18n("Polyline"), true);
    insert(mt_LayoutDirect, layout, i18n("Direct"), true);
    insert(mt_LayoutSpline, layout, i18n("Spline"), true);
    insert(mt_LayoutOrthogonal, layout, i18n("Orthogonal"), true);
    switch(associationLine->layout()) {
    case AssociationLine::Direct:
        m_actions[mt_LayoutDirect]->setChecked(true);
        break;
    case AssociationLine::Orthogonal:
        m_actions[mt_LayoutOrthogonal]->setChecked(true);
        break;
    case AssociationLine::Spline:
        m_actions[mt_LayoutSpline]->setChecked(true);
        break;
    case AssociationLine::Polyline:
    default:
        m_actions[mt_LayoutPolyline]->setChecked(true);
        break;
    }
    addMenu(layout);
}
