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
#include "associationwidget.h"
#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

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
    m_isListView = false;
    m_TriggerObject.m_Widget = widget;
    m_TriggerObjectType = tot_Widget;

    ListPopupMenu::MenuType menuType = ListPopupMenu::mt_Association_Selected;
    if ((type == Uml::AssociationType::Anchor) /*||
          widget->onAssocClassLine(event->scenePos())*/) {
        menuType = ListPopupMenu::mt_Anchor;
    } else if (widget->isCollaboration()) {
        menuType = ListPopupMenu::mt_Collaboration_Message;
    } else if (!widget->association()) {
        menuType = ListPopupMenu::mt_AttributeAssociation;
    } else if (AssocRules::allowRole(type)) {
        menuType = ListPopupMenu::mt_FullAssociation;
    }
    DEBUG(DBG_SRC) << "menue type = " << ListPopupMenu::toString(menuType)
                 << " / association = " << Uml::AssociationType::toString(type);
    setupMenu(menuType);
    setActionChecked(mt_AutoResize, widget->autoResize());
    setupActionsData();
}


/**
 * Inserts a menu item for an association
 *
 * @param label   The menu text.
 * @param mt      The menu type.
 */
void AssociationWidgetPopupMenu::insertAssociationItem(MenuType mt)
{
    switch(mt) {
    case mt_Collaboration_Message:
        // insert(mt_Cut);
        // insert(mt_Copy);
        // insert(mt_Paste);
        // addSeparator();
        break;
    default:
        break;
    }

    if (m_TriggerObjectType == tot_Widget
        && m_TriggerObject.m_Widget->isAssociationWidget()) {
        AssociationWidget *w = m_TriggerObject.m_Widget->asAssociationWidget();
        if (w->isPointAddable())
            insert(mt_Add_Point, Icon_Utils::SmallIcon(Icon_Utils::it_Add_Point), i18n("Add Point"));
        if (w->isPointRemovable())
            insert(mt_Delete_Point, Icon_Utils::SmallIcon(Icon_Utils::it_Delete_Point), i18n("Delete Point"));
        addSeparator();
        insertSubMenuLayout(w->associationLine());
    }
    addSeparator();
    insert(mt_Delete);

    switch(mt) {
    case mt_AttributeAssociation:
    case mt_FullAssociation:
        insert(mt_Rename_Name, i18n("Change Association Name..."));
        insert(mt_Rename_RoleAName, i18n("Change Role A Name..."));
        insert(mt_Rename_RoleBName, i18n("Change Role B Name..."));
        insert(mt_Change_Font);
        insert(mt_Reset_Label_Positions);
        break;

    case mt_Collaboration_Message:
        insert(mt_Change_Font);
        insert(mt_New_Operation);
        insert(mt_Select_Operation, i18n("Select Operation..."));
        break;
    default:
        break;
    }

    insert(mt_Line_Color);
    insert(mt_Properties);
}

void AssociationWidgetPopupMenu::setupMenu(MenuType type)
{
    switch (type) {
    case mt_Association_Selected:
    case mt_AttributeAssociation:
    case mt_FullAssociation:
    case mt_Collaboration_Message:
        insertAssociationItem(type);
        break;
    default:
        break;
    }
}
