/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "dialogspopupmenu.h"

// app includes
#include "debug_utils.h"
#include "uml.h"  // Only needed for log{Warn,Error}

// kde includes
#include <KLocalizedString>

DialogsPopupMenu::DialogsPopupMenu(QWidget *parent, TriggerType type)
  : ListPopupMenu(parent)
{
    switch(type) {
    case tt_New_Parameter:
        insert(mt_New_Parameter);
        break;

    case tt_New_Operation:
        insert(mt_New_Operation);
        break;

    case tt_New_Attribute:
        insert(mt_New_Attribute);
        break;

    case tt_New_InstanceAttribute:
        insert(mt_New_InstanceAttribute);
        break;

    case tt_New_Template:
        insert(mt_New_Template);
        break;

    case tt_New_EnumLiteral:
        insert(mt_New_EnumLiteral);
        break;

    case tt_New_EntityAttribute:
        insert(mt_New_EntityAttribute);
        break;

    case tt_New_Activity:
        insertSubMenuNew(type);
        break;

    case tt_Activity_Selected:
        insertSubMenuNew(type);
        insert(mt_Rename);
        insert(mt_Delete);
        break;

    case tt_Parameter_Selected:
        insert(mt_New_Parameter);
        insert(mt_Rename);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_Operation_Selected:
        insert(mt_New_Operation);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_Attribute_Selected:
        insert(mt_New_Attribute);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_InstanceAttribute_Selected:
        insert(mt_New_InstanceAttribute);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_Template_Selected:
        insert(mt_New_Attribute, Icon_Utils::SmallIcon(Icon_Utils::it_Template_New), i18n("New Template..."));
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_EnumLiteral_Selected:
        insert(mt_New_EnumLiteral);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    case tt_EntityAttribute_Selected:
        insert(mt_New_EntityAttribute);
        insert(mt_Delete);
        insert(mt_Properties);
        break;

    default:
        logWarn1("DialogsPopupMenu: unknown menu type %1", type);
        break;
    }//end switch
    setupActionsData();
}

/**
 * Shortcut for commonly used sub menu initializations.
 *
 * @param type   The MenuType for which to set up the menu.
 */
void DialogsPopupMenu::insertSubMenuNew(TriggerType type)
{
    KMenu * menu = makeNewMenu();
    switch (type) {
        case tt_New_Activity:
        case tt_Activity_Selected:
            break;
        default:
            break;
    }
    addMenu(menu);
}

/**
 * Convert enum MenuType to string.
 */
QString DialogsPopupMenu::toString(TriggerType type)
{
    return QLatin1String(ENUM_NAME(DialogsPopupMenu, TriggerType, type));
}

QDebug operator<<(QDebug out, DialogsPopupMenu::TriggerType type)
{
    out.nospace() << "ListPopupMenu::TriggerType: " << DialogsPopupMenu::toString(type);
    return out.space();
}
