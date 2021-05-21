/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIALOGSPOPUPMENU_H
#define DIALOGSPOPUPMENU_H

#include "listpopupmenu.h"

class DialogsPopupMenu : public ListPopupMenu
{
    Q_OBJECT
    Q_ENUMS(TriggerType)
public:
    enum TriggerType  ///< This type hosts all possible menu types.
    {
        tt_Min = -1,
        tt_Activity_Selected,
        tt_Association_Selected,
        tt_Attribute_Selected,
        tt_EntityAttribute_Selected,
        tt_EnumLiteral_Selected,
        tt_InstanceAttribute_Selected,
        tt_New_Activity,
        tt_New_Attribute,
        tt_New_EntityAttribute,
        tt_New_EnumLiteral,
        tt_New_InstanceAttribute,
        tt_New_Operation,
        tt_New_Parameter,
        tt_New_Template,
        tt_Operation_Selected,
        tt_Parameter_Selected,
        tt_Template_Selected,
        tt_Undefined,
        tt_Max
    };

    DialogsPopupMenu(QWidget *parent, TriggerType type);
    void insertSubMenuNew(TriggerType type, KMenu *menu = 0);
    static QString toString(TriggerType type);
};

QDebug operator<<(QDebug out, DialogsPopupMenu::TriggerType type);

#endif // DIALOGSPOPUPMENU_H
