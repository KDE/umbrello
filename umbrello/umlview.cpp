/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlview.h"

// application specific includes
#include "debug_utils.h"
#include "docwindow.h"
#include "model_utils.h"
#include "notewidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umldragdata.h"
#include "umlscene.h"
#include "umlviewdialog.h"
#include "umlwidget.h"

#include <QPointer>
#include <QScrollBar>

DEBUG_REGISTER(UMLView)

/**
 * Constructor.
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : QGraphicsView(UMLApp::app()->mainViewWidget())
{
    setAcceptDrops(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(NoDrag); //:TODO: RubberBandDrag);
    setScene(new UMLScene(parentFolder, this));
    setResizeAnchor(AnchorUnderMouse);
    setTransformationAnchor(AnchorUnderMouse);
}

/**
 * Destructor.
 */
UMLView::~UMLView()
{
    delete umlScene();
}

/**
 * Getter for the uml scene.
 */
UMLScene* UMLView::umlScene() const
{
    return static_cast<UMLScene*>(scene());
}

/**
 * Returns the zoom of the diagram.
 */
qreal UMLView::zoom() const
{
    return matrix().m11()*100.0;
}

/**
 * Sets the zoom of the diagram.
 */
void UMLView::setZoom(qreal zoom)
{
    if (zoom < 10) {
        zoom = 10;
    } else if (zoom > 500) {
        zoom = 500;
    }

    logDebug1("UMLView::setZoom %1", zoom);
    QMatrix wm;
    wm.scale(zoom / 100.0, zoom / 100.0);
    setMatrix(wm);

    umlScene()->resizeSceneToItems();
}

/**
 * Shows the properties dialog for the view.
 */
bool UMLView::showPropertiesDialog(QWidget *parent)
{
    QPointer<UMLViewDialog> dlg = new UMLViewDialog(parent, umlScene());
    bool success = dlg->exec() == QDialog::Accepted;
    delete dlg;
    return success;
}

void UMLView::zoomIn()
{
    QMatrix wm = matrix();
    wm.scale(1.5, 1.5); // adjust zooming step here
    setZoom(wm.m11()*100.0);
}

void UMLView::zoomOut()
{
    QMatrix wm = matrix();
    wm.scale(2.0 / 3.0, 2.0 / 3.0); //adjust zooming step here
    setZoom(wm.m11()*100.0);
}

/**
 * Overrides standard method from QWidget to resize scene when
 * it's shown.
 */
void UMLView::show()
{
    QWidget::show();
    umlScene()->resizeSceneToItems();
}

/**
 * Zoom the view in and out.
 */
void UMLView::wheelEvent(QWheelEvent* event)
{
    // get the position of the mouse before scaling, in scene coords
    QPointF pointBeforeScale(mapToScene(event->pos()));

    // scale the view ie. do the zoom
    double scaleFactor = 1.15;
    if (event->delta() > 0) {
        // zoom in
        if (zoom() < 500) {
            setZoom(zoom() * scaleFactor);
        } else {
            return;
        }
    } else {
        // zooming out
        if (zoom() > 10) {
            setZoom(zoom() / scaleFactor);
        } else {
            return;
        }
    }

    // get the position after scaling, in scene coords
    QPointF pointAfterScale(mapToScene(event->pos()));

    // get the offset of how the screen moved
    QPointF offset = pointBeforeScale - pointAfterScale;

    // adjust to the new center for correct zooming
    QPointF newCenter = mapToScene(viewport()->rect().center()) + offset;

   centerOn(newCenter);

    UMLApp::app()->setZoom(zoom(), false);
}

/**
 * Overrides the standard operation.
 */
void UMLView::showEvent(QShowEvent* se)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    connect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    connect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    umlScene()->showEvent(se);
    us->resetToolbar();
}

/**
 * Overrides the standard operation.
 */
void UMLView::hideEvent(QHideEvent* he)
{
    UMLApp* theApp = UMLApp::app();
    WorkToolBar* tb = theApp->workToolBar();
    UMLScene *us = umlScene();
    disconnect(tb, SIGNAL(sigButtonChanged(int)), us, SLOT(slotToolBarChanged(int)));
    disconnect(us, SIGNAL(sigResetToolBar()), tb, SLOT(slotResetToolBar()));

    us->hideEvent(he);
}

/**
 * Override standard method.
 */
void UMLView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setInteractive(false);
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
    } else
        QGraphicsView::mousePressEvent(event);
}

/**
 * Override standard method.
 */
void UMLView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MidButton) {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
        setInteractive(true);
        setDragMode(QGraphicsView::NoDrag);
    } else
        QGraphicsView::mouseReleaseEvent(event);
}

/**
 * Override standard method.
 */
void UMLView::resizeEvent(QResizeEvent *event)
{
    bool oldState1 = verticalScrollBar()->blockSignals(true);
    bool oldState2 = horizontalScrollBar()->blockSignals(true);
    QGraphicsView::resizeEvent(event);
    verticalScrollBar()->blockSignals(oldState1);
    horizontalScrollBar()->blockSignals(oldState2);
}
