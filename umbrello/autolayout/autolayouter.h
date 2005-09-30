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

#ifndef AUTOLAYOUTAUTOLAYOUTER_H
#define AUTOLAYOUTAUTOLAYOUTER_H
#include "baseinclude.h"

namespace Autolayout {

/**
This is the super class of any class which encapsulate an autolayout algorithm


@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class Autolayouter {
public:
    virtual ~Autolayouter() {}

    virtual void setNoteConnectionWeight(int i)=0;
    virtual void setNoteConnectionsAsEdges(bool b)=0;
    virtual void setAssociationWeight(int i)=0;

    virtual void setDependenciesWeight(int i)=0;

    virtual void setGeneralizationWeight(int i)=0;

    virtual void setGeneralizationAsEdges(bool b)=0;

    virtual void setDependenciesAsEdges(bool b)=0;

    virtual void setAssociationAsEdges(bool b)=0;

    virtual void setCompressShapes(bool b)=0;

    virtual void setCenterDiagram(bool b)=0;

    virtual void setClusterizeHierarchies(bool b)=0;

    virtual void setShapeSeparation(int i)=0;

    virtual void autolayout(UMLView* v);
protected:
    virtual void run()=0;
    virtual void updateView(UMLView*)=0;
    virtual Autolayout::Canvas* getCanvas()=0;
    virtual Autolayout::Graph* getGraph()=0;
    virtual Autolayout::Graph* setGraph(UMLView* view)=0;
    virtual Autolayout::Canvas* setCanvas(UMLView* view)=0;


};

}

#endif
