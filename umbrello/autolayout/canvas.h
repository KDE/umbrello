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

#ifndef AUTOLAYOUTCANVAS_H
#define AUTOLAYOUTCANVAS_H

namespace Autolayout {

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class Canvas{
public:
    virtual int getMaxX()=0;
    virtual int getMaxY()=0;
    virtual int getBaseX()=0;
    virtual int getBaseY()=0;
    virtual ~Canvas() {}
};

}

#endif
