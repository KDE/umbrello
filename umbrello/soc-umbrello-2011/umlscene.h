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

#ifndef UMLSCENE_H
#define UMLSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "layoutgrid.h"
#include "optionstate.h"

class UMLFolder;
class LayoutGrid;
class ToolBarStateFactory;
class ToolBarState;
class UMLDoc;
class UMLViewImageExporter;

namespace QGV {
  
  class UMLView;
  
  class UMLScene : public QGraphicsScene {
  
  Q_OBJECT
  public:
    UMLScene(UMLFolder *folder, UMLView *view);
    virtual ~UMLScene();
    
    virtual void drawBackground(QPainter* painter, const QRectF& rect);
    
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* ome);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ome);
    void mousePressEvent(QGraphicsSceneMouseEvent* ome);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *e);
    
    bool showPropDialog();
    
    int snapX() const;
    int snapY() const;
    void setSnapSpacing(int x, int y);
    qreal snapped(qreal _x, qreal _y);
    bool snapToGrid() const;
    void setSnapGridVisible(bool bShow);
    LayoutGrid *layoutGrid();
        
    void setUseFillColor(bool ufc);
    QColor fillColor() const;
    
    const Settings::OptionState &options() const;
    void setOptions(Settings::OptionState op);    
    QColor gridDotColor() const;
    void setGridDotColor(const QColor &gridColor);
    
    UMLViewImageExporter* imageExporter();
    
    void resetToolbar();
    
  signals:
    void sigResetToolBar();
    
  public slots:
    void slotToolBarChanged(int c);
  
  private:
    LayoutGrid *m_layoutgrid;
    UMLView *m_view;
    UMLFolder *m_folder;
    ToolBarStateFactory *m_pToolBarStateFactory;
    ToolBarState *m_pToolBarState;
    UMLDoc *m_doc;
    QColor m_gridColor;
    bool  m_snaptogrid, m_gridvisible;
    Settings::OptionState m_Options;
    UMLViewImageExporter *m_imageexporter;
    
  };
  
}

#endif