/***************************************************************************
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel @uml.sf.net>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "autolayouteradapter.h"


namespace Autolayout
  {

  AutolayouterAdapter::AutolayouterAdapter()
      : Autolayout::Autolayouter()
    {}


  AutolayouterAdapter::~AutolayouterAdapter()
  {}}

void Autolayout::AutolayouterAdapter::addRelationship( AssociationWidget * a )
  {
  int weight;
  switch (a->getAssocType())
    {
      case Uml::at_Generalization:;
      case Uml::at_Realization:
        {
        if (genralizationAsEdges)weight=generalizationWeight;
        else return;
        break;
        }
      case Uml::at_Dependency:
        {
        if (dependenciesAsEdges) weight=dependenciesWeight;
        else return;
        break;
        }
      case Uml::at_Anchor:
        {
        if (anchorsAsEdges) weight=anchorsWeight;
        else return;
        break;
        }
      case Uml::at_Aggregation:;
      case Uml::at_Association:;
      case Uml::at_Containment:;
      case Uml::at_Composition:;
      default: return;
      /*case Uml::at_Association_Self:;
      case Uml::at_Activity:;
      case Uml::at_Relationship:;
      case Uml::at_Coll_Message:;
      case Uml::at_Seq_Message:;
      case Uml::at_Coll_Message_Self:;
      case Uml::at_Seq_Message_Self:;
      case Uml::at_Containment:;
      case Uml::at_Composition:;
      case Uml::at_Realization:;
      case Uml::at_UniAssociation:;

      case Uml::at_State:;
      case Uml::at_Unknown:;
      */
    };
  getGraph()->addEdge(a->getWidgetID(Uml::A).c_str(),a->getWidgetID(Uml::B).c_str(),weight);
  }

void Autolayout::AutolayouterAdapter::setAssociationWeight( int i )
  {
  associationWeight=i;
  }

void Autolayout::AutolayouterAdapter::setDependenciesWeight( int i )
  {
  dependenciesWeight=i;
  }

void Autolayout::AutolayouterAdapter::setGeneralizationWeight( int i )
  {
  generalizationWeight=i;
  }

void Autolayout::AutolayouterAdapter::setGeneralizationAsEdges( bool b )
  {
  genralizationAsEdges=b;
  }

void Autolayout::AutolayouterAdapter::setDependenciesAsEdges( bool b )
  {
  dependenciesAsEdges=b;
  }

void Autolayout::AutolayouterAdapter::setAssociationAsEdges( bool b )
  {
  associationAsEdges=b;
  }

void Autolayout::AutolayouterAdapter::setCompressShapes( bool b )
  {
  compressShapes=b;
  }

void Autolayout::AutolayouterAdapter::setCenterDiagram( bool b )
  {
  centerDiagram=b;
  }

void Autolayout::AutolayouterAdapter::setClusterizeHierarchies( bool b )
  {
  clusterizeHierarchies=b;
  }

void Autolayout::AutolayouterAdapter::setShapeSeparation( int i )
  {
  shapeSeparation=i;
  }

Autolayout::Graph  * Autolayout::AutolayouterAdapter::setGraph( UMLView * view )
  {
  if (! view) return 0;
  Autolayout::Graph * g=getGraph();
  if (g&&g->empty())
    {
    UMLWidgetList list = view->getWidgetList();
    UMLWidget* widget;
    for ( widget = list.first(); widget; widget= list.next() )
      {
      if (widget->getBaseType() == Uml::wt_Class)
        {


        g->addNode(widget->getID().c_str(),widget->getWidth(),
                   widget->getHeight());
        }
      }
    AssociationWidgetList as_list=view->getAssociationList();
    AssociationWidget* assoc;
    AssociationWidgetListIt it(as_list);
    while ( (assoc = it.current()) != 0 )
      {
      ++it;
      addRelationship(assoc);
      }
    }
  return g;
  }

void Autolayout::AutolayouterAdapter::updateView( UMLView* view )
  {
if (! view) return ;
UMLWidgetList list = view->getWidgetList();
  UMLWidget* widget;
  Graph *g=getGraph();
  if (! view||!g)  return ;
  for ( widget = list.first(); widget; widget= list.next() )
    if (widget->getBaseType() == Uml::wt_Class)
      {
      Node* n =g->getNode(widget->getID().c_str());
      //printf("old values widgets %s x,y:%d,%d\n",widget->getID().c_str(),widget->getX(),widget->getY());
      //int x_old=widget->getX();
      //int x_calc=n.getX();
      //int x_calc2=30 +n.getX()-widget->getWidth()/2;
      widget->setX(getCanvas()->getBaseX() +n->getX()-widget->getWidth()/2);
      //int x=widget->getX();
      widget->setY(getCanvas()->getMaxY()/2-(n->getY()+(widget->getHeight()/2)));

      widget->updateWidget();

      }
  }

Autolayout::Canvas * Autolayout::AutolayouterAdapter::setCanvas( UMLView* view )
  {
  return canvas=new Autolayout::SimpleCanvas(view->getCanvasWidth(),view->getCanvasHeight());
  }

void Autolayout::AutolayouterAdapter::setNoteConnectionWeight( int i )
{
noteConnectionWeight=i;
}

void Autolayout::AutolayouterAdapter::setNoteConnectionsAsEdges( bool b )
{
noteConnectionAsEdges=b;
}
