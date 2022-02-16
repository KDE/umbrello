/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCIATIONWIDGETPOPUPMENU_H
#define ASSOCIATIONWIDGETPOPUPMENU_H

#include "listpopupmenu.h"

class AssociationLine;

/**
 * Constructs the popup menu for an association widget
 */
class AssociationWidgetPopupMenu : public ListPopupMenu
{
public:
    AssociationWidgetPopupMenu(QWidget *parent, Uml::AssociationType::Enum type, AssociationWidget *widget);

protected:
    void insertSubMenuLayout(const AssociationLine& associationLine);
    void insertSubmodelAction();
};

#endif // ASSOCIATIONWIDGETPOPUPMENU_H
