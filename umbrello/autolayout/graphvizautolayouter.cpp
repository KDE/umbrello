/*
 *  copyright (C) 2005
 *  Dimitri Ognibene <ognibened @yahoo.it>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "graphvizautolayouter.h"

namespace Autolayout {

GraphvizAutolayouter::GraphvizAutolayouter()
 : Autolayout::AutolayouterAdapter()
{

gg=new GraphvizGraph();
}


}

void Autolayout::GraphvizAutolayouter::setCompressShapes( bool b )
{

gg->setCompressShapes(b);
}

void Autolayout::GraphvizAutolayouter::setCenterDiagram( bool b )
{

gg->setCenterDiagram(b);
}

void Autolayout::GraphvizAutolayouter::setShapeSeparation( int i )
{

gg->setShapeSeparation(i);
}
