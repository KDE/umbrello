/*
 *  copyright (C) 2003-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
     * Returns the status of whether to show StereoType.
     *
     * @return  True if stereotype is shown.
     */
    bool getShowStereotype();

    /**
     * Set the status of whether to show StereoType.
     *
     * @param _show             True if stereotype shall be shown.
     */
    void setShowStereotype(bool _status);

    /**
     * Overrides standard method.
     */
    void draw(QPainter& p, int offsetX, int offsetY);

    /**
     * Saves to the <packagewidget> XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads from a <packagewidget> XMI element.
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    // Data loaded/saved

    /**
     * Should it show the <<stereotype>> of the package, currently ignored
     * (stereotype is shown if it isn't empty).
     */
    bool m_bShowStereotype;

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Automatically calculates the size of the object.
     */
    void calculateSize();

    /**
     * The right mouse button menu.
     */
    ListPopupMenu* m_pMenu;
};

#endif
