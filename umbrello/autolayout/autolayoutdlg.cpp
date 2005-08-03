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

#include "autolayoutdlg.h"
#include "../associationwidget.h"
#include "../umlwidget.h"
#include "autolayout.h"

AutolayoutDlg::AutolayoutDlg(UMLView* v,QWidget *parent, const char *name)
        :MyDialog1(parent, name)
{
    view=v;
}

void AutolayoutDlg::slotSetAssociationWeight(int i)
{
    associationWeight=i;
}


void AutolayoutDlg::slotSetDependenciesWeight(int i)
{
    dependenciesWeight=i;
}


void AutolayoutDlg::slotSetGeneralizationWeight(int i)
{
    generalizationWeight=i;
}
void AutolayoutDlg::slotSetGenralizationAsEdges(bool b)
{
    genralizationAsEdges=b;
}


void AutolayoutDlg::slotSetDependenciesAsEdges(bool b)
{
    dependenciesAsEdges=b;
}


void AutolayoutDlg::slotSetAssociationAsEdges(bool b)
{
    associationAsEdges=b;
}



void AutolayoutDlg::slotSetCompressShapes(bool b)
{
    compressShapes=b;
}


void AutolayoutDlg::slotSetCenterDiagram(bool b)
{
    centerDiagram=b;
}


void AutolayoutDlg::slotSetClusterizeHierarchies(bool b)

{
    clusterizeHierarchies=b;
}


void AutolayoutDlg::slotSetShapeSeparation(int i)
{
    shapeSeparation=i;
}

void AutolayoutDlg::slotReloadSettings()
{}


void AutolayoutDlg::slotSaveSettings()
{}


void AutolayoutDlg::slotDoAutolayout()
{
    Autolayout::Autolayouter* a= new Autolayout::DotAutolayouter();
    a->setAssociationAsEdges( associationAsEdges);
    a->setAssociationWeight( associationWeight );
    a->setCenterDiagram( centerDiagram);
    a->setDependenciesAsEdges( dependenciesAsEdges);
    a->setClusterizeHierarchies( clusterizeHierarchies);
    a->setCompressShapes( compressShapes);
    a->setDependenciesWeight( dependenciesWeight);
    a->setGeneralizationAsEdges( genralizationAsEdges);
    a->setGeneralizationWeight( generalizationWeight);
    a->setNoteConnectionWeight( 1);
    a->setNoteConnectionsAsEdges(true);
    a->setShapeSeparation( shapeSeparation);
    a->autolayout( view);
    accept();
}



#include "autolayoutdlg.moc"
