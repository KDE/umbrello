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
#include "graphvizautolayouter.h"

#include <graphviz/graph.h>

namespace Autolayout {

GraphvizAutolayouter::GraphvizAutolayouter()
        : Autolayout::AutolayouterAdapter()
{
    gg = new GraphvizGraph();
}

GraphvizAutolayouter::~GraphvizAutolayouter()
{
    agclose(gg->_agraph);
    delete gg;
}

void GraphvizAutolayouter::setCompressShapes( bool b )
{
    gg->setCompressShapes(b);
}

void GraphvizAutolayouter::setCenterDiagram( bool b )
{
    gg->setCenterDiagram(b);
}

void GraphvizAutolayouter::setShapeSeparation( int i )
{
    gg->setShapeSeparation(i);
}

Autolayout::Canvas * GraphvizAutolayouter::setCanvas( UMLView * view )
{
    Canvas* canvas= AutolayouterAdapter::setCanvas(view);
    gg->setCanvas(canvas);
}

} // end namespace AutoLayout

