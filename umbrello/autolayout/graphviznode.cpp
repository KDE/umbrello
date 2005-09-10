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
#include "graphviznode.h"

#include <stdio.h>
#include <graphviz/types.h>
#include <graphviz/graph.h>

namespace Autolayout {



GraphvizNode::~GraphvizNode()
{
}


int GraphvizNode::getX()
{
    point p = ND_coord_i(n);
    return p.x;
}

int GraphvizNode::getY()
{
    point p = ND_coord_i(n);
    return p.y;
}

}

Autolayout::GraphvizNode::GraphvizNode( Agnode_t * node )
{
    n=node;
}
