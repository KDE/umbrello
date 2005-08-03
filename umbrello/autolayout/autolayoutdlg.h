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

#ifndef AUTOLAYOUTDLG_H
#define AUTOLAYOUTDLG_H

#include "newautolayoutdialog.h"
#include "../umlview.h"
#include "autolayout.h"

class AutolayoutDlg: public MyDialog1
{
    Q_OBJECT
public:
    AutolayoutDlg(UMLView* v, QWidget *parent = 0, const char *name = 0);
public slots:
    virtual void slotSetAssociationWeight(int i);
public slots:
    virtual void slotSetDependenciesWeight(int i);
public slots:
    virtual void slotSetGeneralizationWeight(int i);
public slots:
    virtual void slotSetGenralizationAsEdges(bool b);
public slots:
    virtual void slotSetDependenciesAsEdges(bool b);
public slots:
    virtual void slotSetAssociationAsEdges(bool b);
public slots:
    virtual void slotSetCompressShapes(bool b);
public slots:
    virtual void slotSetCenterDiagram(bool b);
public slots:
    virtual void slotSetClusterizeHierarchies(bool b);
public slots:
    virtual void slotSetShapeSeparation(int i);
    virtual void slotReloadSettings();
    virtual void slotSaveSettings();
    virtual void slotDoAutolayout();

private:
    UMLView *view;
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

};

#endif
