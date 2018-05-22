/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGETPOPUPMENU_H
#define ASSOCIATIONWIDGETPOPUPMENU_H

#include "listpopupmenu.h"

/**
 * Constructs the popup menu for a WidgetBase with customized menu type.
 */
class AssociationWidgetPopupMenu : public ListPopupMenu
{
public:
    AssociationWidgetPopupMenu(QWidget *parent, Uml::AssociationType::Enum type, AssociationWidget *widget);

protected:
    void insertAssociationItem(MenuType mt);
    void setupMenu(MenuType type);
    void insertSubMenuLayout(AssociationLine *associationLine);
    void insertSubmodelAction();
};

#endif // ASSOCIATIONWIDGETPOPUPMENU_H
