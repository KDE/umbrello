/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PACKAGEWIDGET_H
#define PACKAGEWIDGET_H

#include "umlwidget.h"

class UMLPackage;

#define PACKAGE_MARGIN 5

/**
 * Defines a graphical version of the Package.  Most of the
 * functionality will come from the @ref UMLPackage class.
 *
 * @short A graphical version of a Package.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class PackageWidget : public UMLWidget
{
public:
    PackageWidget(UMLScene * scene, UMLPackage * o);
    virtual ~PackageWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    UMLSceneSize minimumSize();

private:
    ListPopupMenu* m_pMenu;  ///< the right mouse button menu
};

#endif
