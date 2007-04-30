/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEWCANVAS_H
#define UMLVIEWCANVAS_H

#include <qcanvas.h>

/**
 *@author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView;

class UMLViewCanvas : public QCanvas {
public:
    /**
    *   Constructor
    */
    UMLViewCanvas( UMLView * pView );

    /**
    *   Deconstructor
    */
    virtual ~UMLViewCanvas();

protected:

    /**
    *   Overrides default method.
    */
    virtual void drawBackground( QPainter & painter, const QRect & clip );

    /**
    *   The view the canvas is associated with.
    */
    UMLView * m_pView;
};

#endif
