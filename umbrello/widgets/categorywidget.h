/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CATEGORYWIDGET_H
#define CATEGORYWIDGET_H
#include "umlwidget.h"

#define UC_MARGIN 5
#define UC_RADIUS 30


class UMLCategory;

/**
 * This class is the graphical version of a UMLCategory.  A CategoryWidget is created
 * by a @ref UMLView.  An CategoryWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * If the Category class that this CategoryWidget is displaying is deleted, the @ref UMLView will
 * make sure that this instance is also deleted.
 *
 * The CategoryWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UMLCategory.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CategoryWidget : public UMLWidget {
public:

    /**
     *  Creates a Category widget.
     *
     *  @param  view            The parent of the widget.
     *  @param  o               The UMLObject to represent.
     */
    CategoryWidget(UMLScene * scene, UMLCategory *o);


    /**
     *  destructor
     */
    virtual ~CategoryWidget();

    /**
    *   Overrides the standard paint event.
    */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
    *   Saves this Category to file.
    */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    // For loading we can use the loadFromXMI() inherited from UMLWidget.

protected:
    /**
     * Overrides method from UMLWidget
     */
    UMLSceneSize minimumSize();

public slots:
    /**
     * Will be called when a menu selection has been made from the
     * popup menu.
     *
     * @param action    The action that has been selected.
     */
    void slotMenuSelection(QAction* action);

};

#endif
