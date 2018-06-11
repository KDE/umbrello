/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
