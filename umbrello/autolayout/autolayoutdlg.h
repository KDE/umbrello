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

#ifndef AUTOLAYOUTDLG_H
#define AUTOLAYOUTDLG_H

#include "newautolayoutdialog.h"
#include "../umlview.h"
#include <qobject.h>
#include <kconfig.h>
#include "autolayout.h"

class AutolayoutDlg : public MyDialog1
{
    Q_OBJECT
  public:
    AutolayoutDlg(KConfig* c,UMLView* v, QWidget *parent = 0, const char *name = 0);
  public slots:
    virtual void slotSetAssociationWeight(int i);
    virtual void slotSetDependenciesWeight(int i);
    virtual void slotSetGeneralizationWeight(int i);
    virtual void slotSetGenralizationAsEdges(bool b);
    virtual void slotSetDependenciesAsEdges(bool b);
    virtual void slotSetAssociationAsEdges(bool b);
    virtual void slotSetCompressShapes(bool b);
    virtual void slotSetCenterDiagram(bool b);
    virtual void slotSetClusterizeHierarchies(bool b);
    virtual void slotSetShapeSeparation(int i);
    virtual void slotReloadSettings();
    virtual void slotSaveSettings();
    virtual void slotDoAutolayout();
    void readConfig(KConfig*);
    void writeConfig(KConfig*);
    virtual void slotSelectAlgorithm(const QString&);


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
    KConfig* config;
    QString algname;
    Autolayout::Autolayouter* getAutolayouter();

};

#endif
