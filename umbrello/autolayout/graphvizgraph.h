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

#ifndef AUTOLAYOUTGRAPHVIZGRAPH_H
#define AUTOLAYOUTGRAPHVIZGRAPH_H

#include "baseinclude.h"
#include "_graph.h"
#include "graphviznode.h"
#include <deque>
#include <graphviz/types.h>

namespace Autolayout
{

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class GraphvizGraph : virtual public Autolayout::Graph
{
public:
    GraphvizGraph();

    virtual ~GraphvizGraph();

    virtual Node* getNode(const char* arg1);
    virtual bool empty();
    virtual void addEdge(const char* nodea, const char* nodeb, int weight=10);
    virtual void addNode(const char* name, int width, int heigt);
    void setCompressShapes(bool b);
    void setCenterDiagram(bool b);
    void setShapeSeparation(int i);
    void setCanvas(Canvas* );
    Agraph_t* _agraph;
    Agsym_t* a_width;
    Agsym_t* a_height;
    Agsym_t* a_label;
    Agsym_t* a_weight;
    std::deque<Node*> nodelist;
    GVC_t* gvc;
    bool empty_flag;
    friend class GraphvizAutolayouter;
};

}

#endif
