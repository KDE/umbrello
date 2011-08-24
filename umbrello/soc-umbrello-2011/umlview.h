/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef QGV_UMLVIEW_H
#define QGV_UMLVIEW_H

#include <QtGui/QGraphicsView>
#include "optionstate.h"
#include "assocrules.h"
#include "umlwidget.h"
#include <listpopupmenu.h>


class UMLFolder;
class UMLDoc;
class UMLObject;



namespace QGV
{
  class UMLScene;
  class Diagram;
  class UMLWidget;
 
  class UMLView : public ::QGraphicsView { 
  
    
  Q_OBJECT    
  public:
    UMLView( UMLFolder *Parentfolder );
    virtual ~UMLView();
    
    UMLFolder *folder() const;
    Diagram *diagram();
    void setFolder(UMLFolder *folder);
    void setDiagram(Diagram *diagram);    
       
    void setZoom(qreal zoom);
    qreal zoom() const;  
    qreal currentZoom();
    void zoomIn();
    void zoomOut();
    
    UMLScene *scene() const;
    
    void setDoc(UMLDoc *doc);
    UMLDoc *doc() const;
    
    void setCenter(const QPointF& centerPoint);
    QPointF center() const;
    
    void showEvent(QShowEvent* /*se*/);
    void closeEvent(QCloseEvent * e);
    void wheelEvent(QWheelEvent* event);
    void hideEvent(QHideEvent* /*he*/);  
    
    void resizeEvent(QResizeEvent* event);    
    
    void setMenu(QPoint pos);
    
    void setSnapToGrid(bool bSnap);
    void setShowSnapGrid(bool bShow);
    
    bool snapToGrid();
    void setSnapGridVisible(bool bShow);
    void toggleShowGrid();
    void toggleSnapToGrid();

  
  public slots:
    void slotShowView();
    void slotActivate();  
    void slotMenuSelection(QAction* action);
    
  signals:
    void sigShowGridToggled(bool);
    void sigSnapToGridToggled(bool);
    
    
  private:
    QPointF m_pos;
    UMLFolder *m_folder;
    Diagram *m_diagram;
    UMLScene *m_scene;
    qreal m_zoom;    
    Uml::Diagram_Type& type();
    QPointF m_currentCenterPoint;
    UMLDoc *m_doc;
    ListPopupMenu *m_menu;
    QPointF m_pastepoint;
    bool m_bUseSnapToGrid, m_bShowSnapGrid;


  };
  
}

#endif