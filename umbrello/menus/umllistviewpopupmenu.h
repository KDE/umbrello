/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
