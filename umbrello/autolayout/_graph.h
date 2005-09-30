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

#ifndef AUTOLAYOUTGRAPH_H
#define AUTOLAYOUTGRAPH_H
#include "baseinclude.h"
namespace Autolayout {

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class Graph{
public:
    virtual ~Graph() {}
    virtual void addNode(const char *name, int width,int heigt)=0;
    virtual void addEdge(const char* nodea,const char*nodeb,int weight=100)=0;
    virtual Node* getNode(const char*)=0;
    virtual bool empty()=0;
};

}

#endif
