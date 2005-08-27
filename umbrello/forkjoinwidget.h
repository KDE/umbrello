/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#ifndef FORKJOINWIDGET_H
#define FORKJOINWIDGET_H
//qt includes
#include <qpainter.h>
//app includes
#include "umlwidget.h"

// fwd decl.
class UMLView;

/**
 * @short Displays a fork/join plate in a state diagram.
 * @author Oliver Kellogg  <okellogg@users.sourceforge.net>
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ForkJoinWidget : public UMLWidget {
public:

    /**
     * Constructs a ForkJoinWidget.
     *
     * @param view          The parent to this widget.
     * @param drawVertical  Whether to draw the plate horizontally or vertically.
     * @param id            The ID to assign (-1 will prompt a new ID.)
     */
    ForkJoinWidget(UMLView * view, bool drawVertical = false, Uml::IDType id = Uml::id_None);

    /**
     * destructor
     */
    virtual ~ForkJoinWidget();

    /**
     * Overrides the function from UMLWidget.
     */
    void calculateSize();

    /**
     * Set whether to draw the plate vertically.
     */
    void setDrawVertical(bool to);
    /**
     * Get whether to draw the plate vertically.
     */
    bool getDrawVertical() const;

    /**
     * Overrides the function from UMLWidget.
     *
     * @param sel  The command to be executed.
     */
    void slotMenuSelection(int sel);

    /**
     * Draws a slim solid black rectangle.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Saves the widget to the <forkjoinwidget> XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Loads the widget from the <forkjoinwidget> XMI element.
     */
    bool loadFromXMI(QDomElement & qElement);

protected:
    /**
     * Reimplements method from UMLWidget. Disables selection adornments and resize.
     */
    void drawSelected(QPainter * p, int offsetX, int offsetY, bool resizeable = false);

private:
    /**
     * Initializes key variables for the class.
     */
    void init();

    bool m_drawVertical;   ///< whether to draw the plate horizontally or vertically
};

#endif
