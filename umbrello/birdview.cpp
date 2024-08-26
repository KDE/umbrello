/*
    SPDX-FileCopyrightText: 2014 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "birdview.h"

#include "debug_utils.h"
#include "uml.h"
#include "umlscene.h"
#include "umlview.h"

#include <QDockWidget>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>

// Currently this file is not using debug statements. Activate this line when inserting them:
//DEBUG_REGISTER_DISABLED(BirdView)

#define VERBOSE_DBG_OUT 0

/**
 * @brief Constructor.
 * @param parent   the dock widget where the bird view is loaded
 * @param view     the view to show
 */
BirdView::BirdView(QDockWidget *parent, UMLView* view)
  : QFrame(),
    m_view(view)
{
    // create view and add it to the container frame
    UMLScene* scene = m_view->umlScene();
    m_birdView = new QGraphicsView(scene);
    m_birdView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_birdView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_birdView->setFrameStyle(QFrame::Box);  //:TODO: remove this line - only for debugging

    // create a frame on top of the view to hide it from the mouse
    m_protectFrame = new QFrame(m_birdView);
    m_protectFrame->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setBackgroundColor(m_protectFrame, QColor(255, 255, 220, 0));

    // draw window frame in the size of shown scene
    setParent(m_birdView);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setLineWidth(1);
    setMidLineWidth(2);
    setFrameStyle(Box | Raised);
    setBackgroundColor(this, QColor(115, 205, 240, 100));
    setFocusPolicy(Qt::StrongFocus);  // enable key press event

    slotDockSizeChanged(parent->rect().size());

    setSlotsEnabled(true);
    parent->setWidget(m_birdView);  // must be the last command
    connect(m_view, SIGNAL(destroyed(QObject*)), this, SLOT(slotDestroyed(QObject*)));
    slotViewChanged();
}

/**
 * Destructor.
 */
BirdView::~BirdView()
{
    disconnect(m_view, SIGNAL(destroyed(QObject*)), this, SLOT(slotDestroyed(QObject*)));
    setParent(nullptr);
    delete m_protectFrame;
    delete m_birdView;
}

/**
 * Handle destroyed view.
 */
void BirdView::slotDestroyed(QObject *object)
{
    if (m_view == object) {
        m_birdView->setScene(nullptr);
        m_view = nullptr;
    }
}

/**
 * Event handler for size changed events of the dock window.
 * @param size   new size to which the dock window was resized
 *
 */
void BirdView::slotDockSizeChanged(const QSize& size)
{
    if (!m_view)
        return;
    QRectF itemsRect = m_birdView->scene()->itemsBoundingRect();
    m_birdView->setSceneRect(itemsRect);
    m_birdView->fitInView(itemsRect, Qt::KeepAspectRatio);

    QRect frameRect = QRect(0, 0, size.width(), size.height());
    m_protectFrame->setGeometry(frameRect);

    qreal scaleW = frameRect.width() / m_birdView->scene()->width();
    qreal scaleH = frameRect.height() / m_birdView->scene()->height();
    qreal scale = scaleH;
    if (scaleW < scaleH) {
        scale = scaleW;
    }
    QTransform wm;
    wm.scale(scale, scale);
    m_birdView->setTransform(wm);
#if VERBOSE_DBG_OUT
    DEBUG() << "setting the size to the scene: " << itemsRect
                   << " / to the frame: " << frameRect
                   << " / scaleW: " << scaleW << " / scaleH: " << scaleH << " / scale: " << scale;
#endif
    QTimer::singleShot(0, this, SLOT(slotViewChanged()));
}

/**
 * Event handler for view changed events of the graphics view.
 * This is done by changing the scroll bars.
 */
void BirdView::slotViewChanged()
{
    if (!m_view) {
        return;
    }
    QRectF r = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
    QRect v = m_birdView->mapFromScene(r).boundingRect();
    setGeometry(v);
}

/**
 * Event handler for mouse press events.
 * Keep the start position for later.
 * @param event   mouse event
 */
void BirdView::mousePressEvent(QMouseEvent *event)
{
    m_moveStartPos = event->globalPos();
}

/**
 * Event handler for mouse move events.
 * Move the frame which represents the viewable window to a new position.
 * Move is only done inside the container.
 * @param event   mouse event
 */
void BirdView::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint delta = m_view->mapFromGlobal(event->globalPos()) - m_view->mapFromGlobal(m_moveStartPos);
    QSizeF scale(m_view->viewport()->rect().width() / rect().width(), m_view->viewport()->rect().height() / rect().height());
    QPoint scaledDelta(delta.x() * scale.width(), delta.y() * scale.height());
    QPointF oldCenter = m_view->mapToScene(m_view->viewport()->rect().center());
    QPointF newCenter = m_view->mapToScene(m_view->viewport()->rect().center() + scaledDelta);
    Q_EMIT viewPositionChanged(newCenter - oldCenter);
    slotViewChanged();
    m_moveStartPos = event->globalPos();
}

/**
 * Event handler for mouse release events.
 * @param event   mouse event
 */
void BirdView::mouseReleaseEvent(QMouseEvent *event)
{
    mouseMoveEvent(event);
}

/**
 * Event handler for key press events.
 * @param event   key press event
 */
void BirdView::keyPressEvent(QKeyEvent *event)
{
    const int STEP = 10;
    QPoint point = pos();
    m_moveStartPos = mapToGlobal(point);
    QString key;
    bool doMove = true;
    QPoint newPoint;
    switch (event->key()) {
    case Qt::Key_Left:
        key = QStringLiteral("LEFT");
        newPoint = QPoint(point.x() - STEP, point.y());
        break;
    case Qt::Key_Right:
        key = QStringLiteral("RIGHT");
        newPoint = QPoint(point.x() + STEP, point.y());
        break;
    case Qt::Key_Up:
        key = QStringLiteral("UP");
        newPoint = QPoint(point.x(), point.y() - STEP);
        break;
    case Qt::Key_Down:
        key = QStringLiteral("DOWN");
        newPoint = QPoint(point.x(), point.y() + STEP);
        break;
    default:
        QFrame::keyPressEvent(event);
        doMove = false;
        break;
    }
    if (doMove) {
        event->accept();
#if VERBOSE_DBG_OUT
        DEBUG() << key << " pressed. start=" << m_moveStartPos << ", " << point << " / new=" << newPoint;
#endif
        QMouseEvent* e = new QMouseEvent(QEvent::MouseMove, newPoint, mapToGlobal(newPoint),
                                         Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent(e);
    }
}

/**
 * Enable or disable the value changed slots of the scroll bars of the view.
 * @param enabled   flag whether to enable or disable the slots
 *
 */
void BirdView::setSlotsEnabled(bool enabled)
{
    UMLView* view = UMLApp::app()->currentView();
    if (enabled) {
        connect(view->verticalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(slotViewChanged()));
        connect(view->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(slotViewChanged()));
    }
    else {
        disconnect(view->verticalScrollBar(), SIGNAL(valueChanged(int)),
                   this, SLOT(slotViewChanged()));
        disconnect(view->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                   this, SLOT(slotViewChanged()));
    }
}

/**
 * Method to set the background color of a frame to a new color.
 * @param frame   frame where the new color has to be set
 * @param color   new color, which has to be set to the frame
 */
void BirdView::setBackgroundColor(QFrame *frame, const QColor& color)
{
    QPalette newPalette = frame->palette();
    newPalette.setColor(frame->backgroundRole(), color);
    frame->setPalette(newPalette);
    frame->setAutoFillBackground(true);
}

//------------------------------------------------------------------------------

/**
 * Constructor.
 */
BirdViewDockWidget::BirdViewDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags)
{
}

/**
 * Handle resize event of the dock widget.
 * Emits size changed signal.
 */
void BirdViewDockWidget::resizeEvent(QResizeEvent *event)
{
    Q_EMIT sizeChanged(event->size());
}

