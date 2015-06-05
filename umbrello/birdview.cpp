/*
    Copyright 2014  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

DEBUG_REGISTER(BirdView)

#define VERBOSE_DBG_OUT 1

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
    setParent(m_birdView);  //m_protectFrame);
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
}

/**
 * Destructor.
 */
BirdView::~BirdView()
{
    disconnect(m_view, SIGNAL(destroyed(QObject*)), this, SLOT(slotDestroyed(QObject*)));
    setParent(0);
    delete m_protectFrame;
    delete m_birdView;
}

/**
 * Handle destroyed view.
 */
void BirdView::slotDestroyed(QObject *object)
{
    if (m_view == object) {
        m_birdView->setScene(0);
        m_view = 0;
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
    m_birdView->scene()->setSceneRect(itemsRect);
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
    QMatrix wm;
    wm.scale(scale, scale);
    m_birdView->setMatrix(wm);
#if VERBOSE_DBG_OUT
    DEBUG(DBG_SRC) << "setting the size to the scene: " << itemsRect
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
    int hvalue = 0;
    int hmin = 0;
    int hmax = 0;
    int hpage = 0;
    int hlen = 0;
    QScrollBar* hScroll = m_view->horizontalScrollBar();
    if (hScroll) {
        hvalue = hScroll->value();
        hmin = hScroll->minimum();
        hmax = hScroll->maximum();
        hpage = hScroll->pageStep();
        hlen = abs(hmax - hmin) + hpage;
#if VERBOSE_DBG_OUT
        DEBUG(DBG_SRC) << "hvalue: " << hvalue << " / hlen: " << hlen
                       << " / hmin: " << hmin << " / hmax: " << hmax
                       << " / hpage: " << hpage;
#endif
    }

    int vvalue = 0;
    int vmin = 0;
    int vmax = 0;
    int vpage = 0;
    int vlen = 0;
    QScrollBar* vScroll = m_view->verticalScrollBar();
    if (vScroll) {
        vvalue = vScroll->value();
        vmin = vScroll->minimum();
        vmax = vScroll->maximum();
        vpage = vScroll->pageStep();
        vlen = abs(vmax - vmin) + vpage;
#if VERBOSE_DBG_OUT
        DEBUG(DBG_SRC) << "vvalue: " << vvalue << " / vlen: " << vlen
                       << " / vmin: " << vmin << " / vmax: " << vmax
                       << " / vpage: " << vpage;
#endif
    }

    if ((vlen == 0) || (hlen == 0)) {
        DEBUG(DBG_SRC) << "Geometry cannot be changed!";
    }
    else {
        if (!((hmin == 0) && (hmax == 0)) |
            !((vmin == 0) && (vmax == 0))) {
            int width = m_protectFrame->width() * hpage / hlen;
            int height = m_protectFrame->height() * vpage / vlen;
            int x = m_protectFrame->width() * (hvalue - hmin) / hlen;
            if (x > m_protectFrame->width() - width) {
                x = m_protectFrame->width() - width;
            }
            if (x <= m_protectFrame->x()) {
                x = m_protectFrame->x();
            }
            int y = m_protectFrame->height() * (vvalue - vmin) / vlen;
            if (y > m_protectFrame->height() - height) {
                y = m_protectFrame->height() - height;
            }
            if (y <= m_protectFrame->y()) {
                y = m_protectFrame->y();
            }
            QRect rect = QRect(x, y, width, height);
            setGeometry(rect);
#if VERBOSE_DBG_OUT
            DEBUG(DBG_SRC) << "protectFrame: " << m_protectFrame->rect() << " / rect: " << rect;
#endif
        }
        else {
            QWidget* container = (QWidget*)m_birdView->parent();
            if (container) {
                setGeometry(container->rect());
            }
        }
    }
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
    const QPoint delta = event->globalPos() - m_moveStartPos;
    int newX = x() + delta.x();
    int newY = y() + delta.y();

//    QRect limit = m_protectFrame->frameRect();
//    if (limit.contains(newX, newY) &&
//        limit.contains(newX + width(), newY + height())) {

    QRect frameRect = m_protectFrame->frameRect();
    QRect limit = QRect(m_protectFrame->mapToGlobal(QPoint(0, 0)), frameRect.size());
#if VERBOSE_DBG_OUT
    DEBUG(DBG_SRC) << "mouse event pos=" << event->pos() << " / globalPos=" << event->globalPos();
    DEBUG(DBG_SRC) << limit << " contains ";
    DEBUG(DBG_SRC) << "   top    left  [ " << newX << ", " << newY << "]";
    DEBUG(DBG_SRC) << "   bottom right [ " << newX + width() << ", " << newY + height() << "]";
#endif
    if (limit.contains(event->globalPos())) {
        if (newX < 0) {
            newX = 0;
        }
        if (newX > frameRect.width() - width()) {
            newX = frameRect.width() - width();
        }
        if (newY < 0) {
            newY = 0;
        }
        if (newY > frameRect.height() - height()) {
            newY = frameRect.height() - height();
        }
#if VERBOSE_DBG_OUT
    DEBUG(DBG_SRC) << "moving delta=" << delta;
#endif
        move(newX, newY);
        m_moveStartPos = event->globalPos();
        event->accept();
        // calculate the scale value for the "real" view
        int hmin = 0;
        int hmax = 0;
        int hpage = 0;
        int hlen = 0;
        QScrollBar* hScroll = m_view->horizontalScrollBar();
        qreal scaleW = 1.0;
        if (hScroll) {
            hmin = hScroll->minimum();
            hmax = hScroll->maximum();
            hpage = hScroll->pageStep();
            hlen = (abs(hmax - hmin) + hpage);
            scaleW = hlen / m_protectFrame->width();
        }
        int vmin = 0;
        int vmax = 0;
        int vpage = 0;
        int vlen = 0;
        QScrollBar* vScroll = m_view->verticalScrollBar();
        qreal scaleH = 1.0;
        if (vScroll) {
            vmin = vScroll->minimum();
            vmax = vScroll->maximum();
            vpage = vScroll->pageStep();
            vlen = abs(vmax - vmin) + vpage;
            scaleH = vlen / m_protectFrame->height();
        }
        qreal scale = scaleH;
        if (scaleW > scaleH) {
            scale = scaleW;
        }
#if VERBOSE_DBG_OUT
            DEBUG(DBG_SRC) << "scaleW: " << scaleW << " / scaleH: " << scaleH << " / scale: " << scale;
#endif
        emit viewPositionChanged(delta * scale);
    }
#if VERBOSE_DBG_OUT
    else {
        DEBUG(DBG_SRC) << "not moving.";
    }
#endif
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
        key = QLatin1String("LEFT");
        newPoint = QPoint(point.x() - STEP, point.y());
        break;
    case Qt::Key_Right:
        key = QLatin1String("RIGHT");
        newPoint = QPoint(point.x() + STEP, point.y());
        break;
    case Qt::Key_Up:
        key = QLatin1String("UP");
        newPoint = QPoint(point.x(), point.y() - STEP);
        break;
    case Qt::Key_Down:
        key = QLatin1String("DOWN");
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
        DEBUG(DBG_SRC) << key << " pressed. start=" << m_moveStartPos << ", " << point << " / new=" << newPoint;
#endif
        QMouseEvent* e = new QMouseEvent(QEvent::MouseMove, QPointF(newPoint), QPointF(mapToGlobal(newPoint)),
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
    setFocusPolicy(Qt::StrongFocus);  // enable key press event
}

/**
 * Handle resize event of the dock widget.
 * Emits size changed signal.
 */
void BirdViewDockWidget::resizeEvent(QResizeEvent *event)
{
    emit sizeChanged(event->size());
}

/**
 * Event handler for key press events.
 * @param event   key press event
 */
void BirdViewDockWidget::keyPressEvent(QKeyEvent *event)
{
    DEBUG(DBG_SRC) << event->key();  //:TODO: is not working
    QDockWidget::keyPressEvent(event);
}
