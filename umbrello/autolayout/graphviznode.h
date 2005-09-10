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
#ifndef AUTOLAYOUTGRAPHVIZNODE_H
#define AUTOLAYOUTGRAPHVIZNODE_H

#include "node.h"

class Agnode_t;

namespace Autolayout {

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class GraphvizNode : virtual public Autolayout::Node
{

    GraphvizNode (Agnode_t* n);
    Agnode_t* n;
    virtual ~GraphvizNode();
public:


    int getX();
    int getY();
    friend class GraphvizGraph;
};

}

#endif
