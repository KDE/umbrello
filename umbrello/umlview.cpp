/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlview.h"
#include "umlscene.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"

// constructor
UMLView::UMLView(UMLFolder *f) : QGraphicsView(UMLApp::app()->getMainViewWidget())
{
    m_nZoom = 100;
    setAcceptDrops(true);
    // [PORT] For now the following is used. Shd check for creation of
    // new scene later.
    setScene(new UMLScene(f));
}

UMLView::~UMLView()
{
    //TODO: Check if the scene shd be deleted
}

void UMLView::setZoom(int zoom)
{
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }

    QMatrix wm;
    wm.scale(zoom / 100.0, zoom / 100.0);
    setMatrix(wm);

    m_nZoom = currentZoom();
    umlScene()->resizeCanvasToItems();
}

int UMLView::currentZoom()
{
    return (int)(matrix().m11()*100.0);
}

void UMLView::zoomIn()
{
    QMatrix wm = matrix();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::zoomOut()
{
    QMatrix wm = matrix();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom((int)(wm.m11()*100.0));
}

void UMLView::showEvent(QShowEvent* /*se*/)
{
    UMLApp* theApp = UMLApp::app();
    UMLDoc *doc = theApp->getDocument();
    WorkToolBar* tb = theApp->getWorkToolBar();
    UMLScene *us = umlScene();
    connect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    connect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    connect(doc, SIGNAL(sigObjectCreated(UMLObject *)),
            us, SLOT(slotObjectCreated(UMLObject *)));
    connect(us, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            UMLApp::app()->getDocWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(us, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            UMLApp::app()->getDocWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
    us->resetToolbar();

}

void UMLView::hideEvent(QHideEvent* /*he*/)
{
    UMLApp* theApp = UMLApp::app();
    UMLDoc *doc = theApp->getDocument();
    WorkToolBar* tb = theApp->getWorkToolBar();
    UMLScene *us = umlScene();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    disconnect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));
    disconnect(doc, SIGNAL(sigObjectCreated(UMLObject *)), us, SLOT(slotObjectCreated(UMLObject *)));
    disconnect(us, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               UMLApp::app()->getDocWindow(), SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(us, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               UMLApp::app()->getDocWindow(), SLOT(slotWidgetRemoved(UMLWidget*)));
}

void UMLView::closeEvent(QCloseEvent * e)
{
    QWidget::closeEvent(e);
}

UMLScene* UMLView::umlScene() const
{
    return qobject_cast<UMLScene*>(scene());
}

void UMLView::show()
{
    QWidget::show();
    umlScene()->resizeCanvasToItems();
}

#include "umlview.moc"
