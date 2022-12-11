/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef WIDGETBASEPOPUPMENU_H
#define WIDGETBASEPOPUPMENU_H

#include "listpopupmenu.h"

/**
 * Constructs the popup menu for a WidgetBase with customized menu type.
 */
class WidgetBasePopupMenu : public ListPopupMenu
{
public:
    WidgetBasePopupMenu(QWidget * parent, WidgetBase * widget, bool multi, WidgetBase::WidgetType uniqueType = WidgetBase::wt_UMLWidget);

protected:
    void makeMultiClassifierShowPopup(WidgetBase::WidgetType type);
    void makeClassifierShowPopup(ClassifierWidget *c);
    void insertSingleSelectionMenu(WidgetBase* widget);
    void insertMultiSelectionMenu(WidgetBase::WidgetType uniqueType);
    void insertStdItems(bool insertLeadingSeparator = true, WidgetBase::WidgetType type = WidgetBase::wt_UMLWidget);
    void insertSubMenuAlign();
    void insertSubMenuColor(bool fc);
    void insertSubMenuNew(WidgetBase::WidgetType type);
    void insertSubMenuShowEntity(EntityWidget *widget);
    QMenu *makeCategoryTypeMenu(UMLCategory *category);
};

#endif // WIDGETBASEPOPUPMENU_H
