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

#ifndef AUTOLAYOUTAUTOLAYOUTERADAPTER_H
#define AUTOLAYOUTAUTOLAYOUTERADAPTER_H
//#include "autolayout.h"
#include "baseinclude.h"
#include "autolayouter.h"
#include "../umlnamespace.h"


namespace Autolayout
{

/**
@author Dimitri Ognibene <ognibened @yahoo.it>
*/
class AutolayouterAdapter : virtual public Autolayout::Autolayouter
{
public:
    AutolayouterAdapter();

    virtual ~AutolayouterAdapter();
    virtual void setAssociationWeight(int i);

    virtual void setDependenciesWeight(int i);

    virtual void setGeneralizationWeight(int i);

    virtual void setGeneralizationAsEdges(bool b);

    virtual void setDependenciesAsEdges(bool b);

    virtual void setAssociationAsEdges(bool b);

    virtual void setCompressShapes(bool b);

    virtual void setCenterDiagram(bool b);

    virtual void setClusterizeHierarchies(bool b);

    virtual void setShapeSeparation(int i);
    virtual void setNoteConnectionsAsEdges(bool b);
    virtual void setNoteConnectionWeight(int i);


protected:
    virtual void run()=0;
    virtual void updateView(UMLView* view);
    virtual Canvas* getCanvas(){return canvas;};
    virtual Graph* getGraph()=0;
    virtual Graph* setGraph(UMLView* view);
    virtual void addRelationship(AssociationWidget* a);
    virtual Canvas* setCanvas(UMLView* view);

    int associationWeight;
    int dependenciesWeight;
    int generalizationWeight;
    bool genralizationAsEdges;
    bool dependenciesAsEdges;
    bool associationAsEdges;
    bool compressShapes;
    bool centerDiagram;
    bool clusterizeHierarchies;
    int shapeSeparation;
    int noteConnectionWeight;
    bool noteConnectionAsEdges;
    bool anchorsAsEdges;
    int  anchorsWeight;
    Canvas* canvas;
private:
    //Graph* graph;


};

}

#endif
