/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2018 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLSCENEPOPUPMENU_H
#define UMLSCENEPOPUPMENU_H

#include "listpopupmenu.h"

/**
 * @brief The UMLScenePopupMenu class
 */
class UMLScenePopupMenu : public ListPopupMenu
{
public:
    UMLScenePopupMenu(QWidget *parent, UMLScene *scene);

protected:
    UMLScene *m_scene;
    void insertLayoutItems();
    void insertSubMenuNew(Uml::DiagramType::Enum type, KMenu *menu = 0);
};

#endif // UMLSCENEPOPUPMENU_H
