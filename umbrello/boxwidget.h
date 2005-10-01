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

#ifndef BOXWIDGET_H
#define BOXWIDGET_H
//qt includes
#include <qpainter.h>
//app includes
#include "umlwidget.h"

// fwd decl.
class UMLView;

/**
 * Displays a rectangular box.
 * These widgets are diagram specific.  They will still need a unique id
 * from the @ref UMLDoc class for deletion and other purposes.
 *
 * @short Displays a box.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class BoxWidget : public UMLWidget {
public:

    /**
     * Constructs a BoxWidget.
     *
     * @param view              The parent to this widget.
     * @param id                The ID to assign (-1 will prompt a new ID.)
     */
    BoxWidget(UMLView * view, Uml::IDType id = Uml::id_None);

    /**
     * destructor
     */
    virtual ~BoxWidget();

    /**
     * Apply possible constraints to the given candidate width and height.
     */
    virtual void constrain(int& width, int& height);

    /**
     * Overrides the standard function.
     * Changes the cursor if in the lower right corner.
     */
    void mousePressEvent(QMouseEvent* me);

    /**
     * Overrides the standard function.
     * Resizes the widget if necessary.
     */
    void mouseMoveEvent(QMouseEvent* me);

    /**
     * Overrides the standard operation.
     * Stops the resize event and resets the mouse cursor.
     */
    void mouseReleaseEvent(QMouseEvent* me);

    /**
     * Draws a rectangle.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Saves the widget to the <boxwidget> XMI element.
     * Note: For loading from XMI, the inherited parent method is used.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
private:
    /**
     * Initializes key variables for the class.
     */
    void init();

};

#endif
