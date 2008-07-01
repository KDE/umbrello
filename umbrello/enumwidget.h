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

#ifndef ENUMWIDGET_H
#define ENUMWIDGET_H

#include "umlwidget.h"

class UMLScene;

#define ENUM_MARGIN 5

/**
 * Defines a graphical version of the enum.  Most of the functionality
 * will come from the @ref NewUMLRectWidget class from which class inherits from.
 *
 * @short A graphical version of an enum.
 * @author Jonathan Riddell
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class EnumWidget : public NewUMLRectWidget {
public:

    /**
     * Constructs an EnumWidget.
     *
     * @param view              The parent of this EnumWidget.
     * @param o         The UMLObject this will be representing.
     */
    EnumWidget(UMLScene* view, UMLObject* o);

    /**
     * Standard deconstructor.
     */
    ~EnumWidget();

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Returns the status of whether to show Package.
     *
     * @return  True if package is shown.
     */
    bool getShowPackage() const;

    /**
     * Toggles the status of whether to show package.
     */
    void toggleShowPackage();

    /**
     * Set the status of whether to show Package.
     *
     * @param _status             True if package shall be shown.
     */
    void setShowPackage(bool _status);

    /**
     * Draws the enum as a rectangle with a box underneith with a list of literals
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

    /**
     * Saves to the "enumwidget" XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads from an "enumwidget" XMI element.
     */
    bool loadFromXMI(QDomElement& qElement);

protected:
    /**
     * Overrides method from NewUMLRectWidget.
     */
    QSizeF calculateSize();

    bool m_bShowPackage;

private:

public slots:
    /**
     * Will be called when a menu selection has been made from the
     * popup menu.
     *
     * @param action       The action that has been selected.
     */
    void slotMenuSelection(QAction* action);
};

#endif
