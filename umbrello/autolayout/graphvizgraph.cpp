/***************************************************************************
 *  copyright (C) 2005
 *  Umbrello UML Modeller Authors <uml-devel @uml.sf.net>
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "graphvizgraph.h"

#include <graphviz/graph.h>
#include <kdebug.h>

#define DPI 96

char* _strcpy(const char* name)
{
    char *a;
    strlen(name);
    a=new char[strlen(name)+1];
    a=strcpy(a,name);
    return a;
}
namespace Autolayout
{

/**
 *
 * @return
 */
GraphvizGraph::GraphvizGraph()
        : Autolayout::Graph()
{
    aginit();
    empty_flag=true;
    _agraph = agopen("graph",AGDIGRAPH);
    a_width= agnodeattr(_agraph, "width", "");;
    a_height= agnodeattr(_agraph, "height", "");
    a_label = agnodeattr(_agraph, "label", "");
    a_weight= agedgeattr(_agraph,"weight","");
    agnodeattr(_agraph, "fixedsize", "true");
    agnodeattr(_agraph, "margin", "0.01,0.01");
    agnodeattr(_agraph, "shape", "box");
    agraphattr(_agraph, "dpi", "DPI/0");


}


GraphvizGraph::~GraphvizGraph()
{
    nodelist.clear();
    agclose(_agraph);
    /* Free graph structures */
    //#ifndef internal_renderizer

    /* Clean up output file and errors */
    // dotneato_terminate(gvc);
    //dotneato_eof(gvc);

}




void GraphvizGraph::addEdge(const char* nodea, const char* nodeb, int weight)
{
    char *a=_strcpy(nodea);
    char *b=_strcpy(nodeb);
    char *weight_str;
    asprintf(&weight_str,"%d",weight);
    Agedge_t* e= agedge(_agraph,agnode(_agraph,a),agnode(_agraph,b));
    delete[](a);
    delete[](b);
    agxset(e,a_weight->index,weight_str);


}

void GraphvizGraph::addNode(const char* name, int width, int height)
{
    char *a =_strcpy(name);
    char *w_str,*h_str;
    Agnode_t* node =agnode(_agraph,a);
    delete[](a);
    agxset(node, a_label->index, "a");
    asprintf(&h_str,"%f",((float)height)/DPI);
    asprintf(&w_str,"%f",((float)width)/DPI);
    agxset(node, a_height->index,h_str);// sprintf("%d",height));
    free (h_str);
    agxset(node, a_width->index, w_str);
    free (w_str);
    empty_flag = false;
}


void Autolayout::GraphvizGraph::setCompressShapes( bool b )
{
    if (empty())
    {
        if (b) agraphattr(_agraph,"ratio","compress");
        else agraphattr(_agraph,"ratio","");
    }
}

void Autolayout::GraphvizGraph::setCenterDiagram( bool b )
{
    if (empty())
    {
        if (b)  agraphattr(_agraph,"center","true");
        else  agraphattr(_agraph,"center","false");
    }
}

void Autolayout::GraphvizGraph::setShapeSeparation( int i )
{
    char* a;
    asprintf(&a,"%f",((float) i)/10.0);
    agraphattr(_agraph,"nodesep",a);
    free (a);
}

bool Autolayout::GraphvizGraph::empty( )
{
    return empty_flag;
}

Autolayout::Node* Autolayout::GraphvizGraph::getNode( const char * arg1 )
{
    char *a = _strcpy(arg1);
    Autolayout::GraphvizNode* b=
        new Autolayout::GraphvizNode(agnode(_agraph,a));
    delete[](a);
    nodelist.push_back(b);
    return b;
}

void GraphvizGraph::setCanvas( Autolayout::Canvas * canvas)
{
    char buf[100];
    sprintf(buf,"%f,%f",((float)canvas->getMaxX()/DPI),((float)canvas->getMaxY()/DPI));
    kDebug() << "size: " << buf << endl;
    agraphattr(_agraph, "size", buf);
    agraphattr(_agraph, "page", buf);
}

}  // end namespace Autolayout

