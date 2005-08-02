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
#ifndef AUTOLAYOUTGRAPHVIZNODE_H
#define AUTOLAYOUTGRAPHVIZNODE_H

#include <node.h>
#include <dotneato.h>
namespace Autolayout {

/**
@author 
*/
class GraphvizNode : virtual public Autolayout::Node
{

    GraphvizNode (Agnode_t* n);
    Agnode_t* n;
    ~GraphvizNode();
public:


    int getX();
    int getY();
friend class GraphvizGraph;
};

}

#endif
