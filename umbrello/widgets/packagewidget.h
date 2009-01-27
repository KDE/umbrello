/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PACKAGEWIDGET_H
#define PACKAGEWIDGET_H

#include "umlwidget.h"

class UMLPackage;

#define PACKAGE_MARGIN 5

/**
 * Defines a graphical version of the Package.  Most of the functionality
 * will come from the @ref UMLPackage class.
 *
 * @short A graphical version of a Package.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class PackageWidget : public UMLWidget {
public:

    /**
     * Constructs a PackageWidget.
     *
     * @param view              The parent of this PackageWidget.
     * @param o         The UMLObject this will be representing.
     */
    PackageWidget(UMLView * view, UMLPackage * o);

    /**
     * destructor
     */
    virtual ~PackageWidget();

    /**
     * Overrides standard method.
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves to the "packagewidget" XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Overrides method from UMLWidget
     */
    QSize calculateSize();

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * The right mouse button menu.
     */
    ListPopupMenu* m_pMenu;
};

#endif
