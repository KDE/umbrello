/*
 *  copyright (C) 2005
 *  Umbrello UML Modeller Authors <uml-devel @uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUTOLAYOUTSIMPLECANVAS_H
#define AUTOLAYOUTSIMPLECANVAS_H

#include "canvas.h"
#include <qrect.h>
namespace Autolayout {

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class SimpleCanvas: public Canvas
{
public:
    SimpleCanvas(int i, int j):max_x(i),max_y(j){}
    virtual ~SimpleCanvas() {}
    virtual int getMaxX(){return max_x;}
    virtual int getMaxY(){return max_y;}
    virtual int getBaseX(){return 0;}
    virtual int getBaseY(){return 0;}
    int max_x,max_y;
};

}

#endif
