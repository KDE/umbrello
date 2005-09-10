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

#ifndef AUTOLAYOUTABLEDIAGRAM_H
#define AUTOLAYOUTABLEDIAGRAM_H
#include <dotneato.h>
#define internal_renderizer
#include "autolayout.h"
/**
@author Dimitri Ognibene <ognibened @yahoo.it>
Umbrello UML Modeller Authors
*/
namespace Autolayout{
class Diagram//: public virtual Graph, public virtual Canvas{
{
private:
    Agraph_t* g;
    Agsym_t* a_width;
    Agsym_t* a_height;
    Agsym_t* a_label;
#ifndef internal_renderizer
    GVC_t* gvc;
#endif
public:
    Diagram(int,int);

    ~Diagram();

    void addNode(const char *name, int width,int heigt);
    void addEdge(const char* nodea,const char*nodeb);
    void autolayout();
    void save();
    Node getNode(const char*);

};


}
#endif
