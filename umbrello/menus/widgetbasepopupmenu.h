/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
    void insertSubMenuNew(WidgetBase::WidgetType type, KMenu *menu=0);
    void insertSubMenuShowEntity(EntityWidget *widget);
    KMenu *makeCategoryTypeMenu(UMLCategory *category);
};

#endif // WIDGETBASEPOPUPMENU_H
