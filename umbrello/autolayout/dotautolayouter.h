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

#ifndef AUTOLAYOUTDOTAUTOLAYOUTER_H
#define AUTOLAYOUTDOTAUTOLAYOUTER_H
#include "baseinclude.h"
#include "graphvizautolayouter.h"

namespace Autolayout {

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class DotAutolayouter : virtual public Autolayout::GraphvizAutolayouter
{
public:
    DotAutolayouter();

    virtual ~DotAutolayouter();

    virtual void run();

};

}

#endif
