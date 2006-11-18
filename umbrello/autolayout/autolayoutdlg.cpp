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

#include "autolayoutdlg.h"
#include "../associationwidget.h"
#include "../umlwidget.h"
#include "autolayout.h"
#include "newautolayoutdialog.h"
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <kdebug.h>


AutolayoutDlg::AutolayoutDlg(KConfig* c,UMLView* v,QWidget *parent, const char *name)
        :MyDialog1(parent, name)
{
    view=v;
    readConfig(c);
    config=c;
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
{
    readConfig(config);
}


void AutolayoutDlg::slotSaveSettings()
{
    writeConfig(config);
}


void AutolayoutDlg::slotDoAutolayout()
{

    Autolayout::Autolayouter* a=getAutolayouter();;

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
    delete a;
    a=0;
    accept();
}

void AutolayoutDlg::readConfig( KConfig * conf)
{
    conf->setGroup("AutolayoutDlg");
    associationEdgesCB->setChecked((bool)(conf->readBoolEntry( "associationAsEdges",false)));
    centerDiagramCB->setChecked((bool)(conf->readBoolEntry( "centerDiagram",true)));
    dependenciesEdgesCB->setChecked((bool)(conf->readBoolEntry( "dependenciesAsEdges",false)));
    clusterizeHierarchiesCB->setChecked((bool)(conf->readBoolEntry( "clusterizeHierarchies",false)));
    compressShapesCB->setChecked((bool)(conf->readBoolEntry( "compressShapes",true)));
    dependenciedEdgesSL->setValue((int)(conf->readNumEntry( "dependenciesWeight",0)));
    generalizationCB->setChecked((bool)(conf->readBoolEntry( "genralizationAsEdges",true)));
    generalizationEdgessSL->setValue((int)(conf->readNumEntry( "generalizationWeight",1)));
    associationEdgesSL->setValue((int)(conf->readNumEntry( "associationWeight",0)));
    shapeSeparationSB->setValue((int)(conf->readNumEntry( "shapeSeparation",0)));
    algorithmCOB->setCurrentItem((int)(conf->readNumEntry( "algorithm",0)));
}

void AutolayoutDlg::writeConfig( KConfig * conf)
{
    //  conf=kapp->config();
    conf->setGroup("AutolayoutDlg");
    conf->writeEntry( "associationAsEdges",associationEdgesCB->isChecked());
    conf->writeEntry( "centerDiagram", centerDiagramCB->isChecked());
    conf->writeEntry("dependenciesAsEdges",dependenciesEdgesCB->isChecked());
    conf->writeEntry("clusterizeHierarchies",clusterizeHierarchiesCB->isChecked());
    conf->writeEntry("dependenciesWeight", dependenciedEdgesSL->value());
    conf->writeEntry("genralizationAsEdges",generalizationCB->isChecked());

    conf->writeEntry("generalizationWeight",generalizationEdgessSL->value());
    conf->writeEntry("associationWeight",associationEdgesSL->value());
    conf->writeEntry("shapeSeparation",shapeSeparationSB->value());
    //conf->writeEntry("al


    //algorithmCOB->setCurrentItem(conf->readNumEntry( "algorithm",0));

}

void AutolayoutDlg::slotSelectAlgorithm( const QString& _algname)
{
    algname=_algname;

}

Autolayout::Autolayouter * AutolayoutDlg::getAutolayouter( )
{
    const QString text = algorithmCOB->currentText();
    kDebug() << "Autolayout Algorithm " << algname << " found " << text << endl;
    if (text == "dot")
        return new Autolayout::DotAutolayouter();
    if (text == "circo")
        return new Autolayout::CircoAutolayouter();
    if (text ==  "neato")
        return new Autolayout::NeatoAutolayouter();
    kError() << "Autolayout Algorithm not found" << endl;
    return new Autolayout::DotAutolayouter();
}



#include "autolayoutdlg.moc"
