/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *  copyright (C) 2003-2004                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#ifndef RESIZABLEWIDGET_H
#define RESIZABLEWIDGET_H

#include "umlwidget.h"

/**
 * Intermediate abstract base class which adds resizing to UMLWidget.
 *
 * @short UMLWidget with resizability
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ResizableWidget : public UMLWidget {
public:

    /**
     * Constructs a ResizableWidget.
     *
     * @param view      The parent to this widget.
     * @param o         The UMLObject this will be representing.
     */
    ResizableWidget(UMLView * view, UMLObject *o);

    /**
     * Constructs a ResizableWidget.
     *
     * @param view      The parent to this widget.
     * @param id        The ID to assign (-1 will prompt a new ID.)
     */
    ResizableWidget(UMLView * view, Uml::IDType id = Uml::id_None);

    /**
     * destructor
     */
    virtual ~ResizableWidget();

    /**
     * Compute the minimum possible width and height.
     * The default implementation returns width=20, height=20.
     *
     * @param width  return value, computed width
     * @param height return value, computed height
     */
    virtual void calcMinWidthAndHeight(int& width, int& height);

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

protected:
    /**
     * Apply possible constraints to the given candidate width and height.
     * The default implementation calls calcMinWidthAndHeight() and
     * assigns the returned values if they are greater than the
     * input values.
     *
     * @param width  input value, may be modified by the constraint
     * @param height input value, may be modified by the constraint
     */
    virtual void constrain(int& width, int& height);

    bool m_bResizing;
    int  m_nOldH, m_nOldW;
private:
    void init();
};

#endif
