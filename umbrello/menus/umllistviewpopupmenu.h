/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLLISTVIEWPOPUPMENU_H
#define UMLLISTVIEWPOPUPMENU_H

#include "listpopupmenu.h"
#include "umllistviewitem.h"

class UMLListViewPopupMenu : public ListPopupMenu {
public:
    UMLListViewPopupMenu(QWidget *parent, UMLListViewItem *item);

protected:
    void insertSubMenuNew(UMLListViewItem::ListViewType type, KMenu *menu = 0);
    void insertStdItems(bool insertLeadingSeparator = true);
    void insertSubmodelAction();
};

#endif // UMLLISTVIEWPOPUPMENU_H
