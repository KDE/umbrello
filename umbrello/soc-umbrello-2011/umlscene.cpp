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

#include "umlscene.h"

#include <QDialog>

#include "umlview.h"
#include "folder.h"
#include "toolbarstatefactory.h"
#include "toolbarstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlviewdialog.h"
#include "boxwidget.h"
#include "umlviewimageexporter.h"


namespace QGV {
  
UMLScene::UMLScene(UMLFolder *folder, UMLView *view) : 
QGraphicsScene(0, 0, 1300, 1300),
m_view(view),
m_folder(folder),
m_snaptogrid(true),
m_gridvisible(true)
{
  setBackgroundBrush(QColor(195, 195, 195));
  m_layoutgrid = new LayoutGrid(0, this);
  addItem(m_layoutgrid);
  setSceneRect(itemsBoundingRect());
  m_imageexporter = new UMLViewImageExporter(m_view);
  m_pToolBarStateFactory = new ToolBarStateFactory();
  m_pToolBarState = m_pToolBarStateFactory->state(WorkToolBar::tbb_Arrow, m_view);  
}

UMLScene::~UMLScene()
{
}

void UMLScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawBackground(painter, rect);
}

/**
 * Overrides the standard operation.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* ome)
{
  m_pToolBarState->mouseRelease(ome);
}


/**
 * Override standard method.
 * Calls the same method in the current tool bar state.
 */
void UMLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ome)
{
    
    qDebug() << "mouseDoubleClickEvent: " << ome->scenePos();
    m_pToolBarState->mouseDoubleClick(ome);

    //if (isWidgetOrAssociation(ome->scenePos())) {
    if (1 == 0) {
        ome->ignore();
    }
    else {
        // show properties dialog of the scene
        if (showPropDialog() == true) {
            m_doc->setModified();
        }
        ome->accept();
    }
}

bool UMLScene::showPropDialog()
{
    // Be explict to avoid confusion
    QWidget *parent = UMLApp::app()->current_View();
    bool success = false;
    QPointer<UMLViewDialog> dlg = new UMLViewDialog(parent, m_view);
    if (dlg->exec() == QDialog::Accepted) {
        success = true;
    }
    delete dlg;
    return success;
}

void UMLScene::mousePressEvent(QGraphicsSceneMouseEvent* ome)
{
  //m_view->diagram()->setPos(ome->scenePos());
  if(ome != NULL){
    if (ome->button() == Qt::RightButton) {
	m_view->setMenu(QPoint(ome->screenPos().x(), ome->screenPos().y()));
    }else{
	//create new widget
    }
  }else{
    qDebug() << "mouse is NULL"; 
  }
  
  
}

int UMLScene::snapX() const
{
  return m_layoutgrid->gridSpacingX();
}

int UMLScene::snapY() const
{
  return m_layoutgrid->gridSpacingY();
}

void UMLScene::setSnapSpacing(int x, int y)
{
  m_layoutgrid->setGridSpacing(x, y);  
}

qreal UMLScene::snapped(qreal _x, qreal _y)
{
    int n = _x == 0 ? (int)_y : (int)_x;
    if (snapToGrid()) {
        int grid = _x == 0 ? snapY() : snapX();
        int modX = n % grid;
        n -= modX;
        if (modX >= grid / 2)
            n += grid;
    }
    return n;    
}

bool UMLScene::snapToGrid() const
{
  return m_snaptogrid; 
}


const Settings::OptionState &UMLScene::options() const			
{
  return m_Options;
}

void UMLScene::setOptions(Settings::OptionState op)
{
  m_Options = op;
}

void UMLScene::setUseFillColor(bool ufc)
{
    m_Options.uiState.useFillColor = ufc;
}

QColor UMLScene::fillColor() const
{
    return m_Options.uiState.fillColor;
}

QColor UMLScene::gridDotColor() const
{
  return m_gridColor;
  
}

void UMLScene::setGridDotColor(const QColor &gridColor)
{
  m_gridColor = gridColor;
}

void UMLScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    QGraphicsScene::contextMenuEvent(e);
}



UMLViewImageExporter* UMLScene::imageExporter()
{ 
  return m_imageexporter;
}

void UMLScene::resetToolbar()
{
    emit sigResetToolBar();
}

void UMLScene::slotToolBarChanged(int c)
{
    //m_pToolBarState->cleanBeforeChange();
    m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c, m_view);
    //m_pToolBarState->init();

    //m_bPaste = false;
}

LayoutGrid *UMLScene::layoutGrid()
{
  return m_layoutgrid;
}



}