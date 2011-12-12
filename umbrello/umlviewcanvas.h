/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEWCANVAS_H
#define UMLVIEWCANVAS_H

#include "optionstate.h"
#include <q3canvas.h>


/**
 *@author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLView;

class UMLViewCanvas : public Q3Canvas
{
public:
    UMLViewCanvas(UMLView * pView,const Settings::OptionState& option);
    virtual ~UMLViewCanvas();

    void setColors(const QColor& backColor, const QColor& gridColor);
    QColor gridDotColor() const; 

protected:

    virtual void drawBackground(QPainter & painter, const QRect & clip);

    UMLView * m_pView;      ///< The view the canvas is associated with.
    QColor    m_gridColor;  ///< Color for the grid dots.
};

#endif
