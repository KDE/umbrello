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
//#include <qmath.h>
#include <QMouseEvent>
#include <QScrollBar>

DEBUG_REGISTER(BirdView)

/**
 * @brief Constructor.
 * @param parent
 * @param rect
 */
BirdView::BirdView(QDockWidget *parent, UMLView* view)
    : QFrame(),
      m_view(view)
{
    // create container frame
    QFrame* container = new QFrame(parent);
    container->setLineWidth(1);
    setBackgroundColor(container, Qt::white);

    // create view and add it to the container frame
    UMLScene* scene = m_view->umlScene();
    m_birdView = new QGraphicsView(scene, container);
    m_birdView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_birdView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    m_birdView->fitInView(container->rect(), Qt::KeepAspectRatio);

//    QRectF maxRect = scene->itemsBoundingRect();
//    qreal scaleW = container->rect().width() / maxRect.width();
//    qreal scaleH = container->rect().height() / maxRect.height();
//    qreal scale = scaleH;
//    if (scaleW > scaleH) {
//        scale = scaleW;
//    }
//    DEBUG(DBG_SRC) << "scale: " << scale;
//    QMatrix wm;
//    wm.scale(scale, scale);
//    m_birdView->setMatrix(wm);

    // create a frame on top of the view to hide it from the mouse
    m_baseFrame = new QFrame(container);
    m_baseFrame->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setBackgroundColor(m_baseFrame, QColor(255, 255, 224, 0));

    // draw window frame in the size of shown scene
    setParent(m_baseFrame);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setLineWidth(1);
    setMidLineWidth(2);
    setFrameStyle(Box | Raised);
    setBackgroundColor(this, QColor(115, 205, 240, 100));

    slotDockSizeChanged(parent->rect().size());
    slotViewChanged();

    setSlotsEnabled(true);
    parent->setWidget(container);  // must be the last command
}

BirdView::~BirdView()
{
    delete m_birdView;
}

/**
 * @brief BirdView::slotDockSizeChanged
 * @param
 *
 */
void BirdView::slotDockSizeChanged(const QSize& size)
{
    QWidget* container = static_cast<QWidget*>(m_birdView->parent());
    if (container) {
        QDockWidget* parent = static_cast<QDockWidget*>(container->parent());
        container->setGeometry(parent->rect());

        m_birdView->fitInView(container->rect(), Qt::KeepAspectRatio);

        const int deltaX =  5;
        const int deltaY = 25;
        QRect rect = QRect(container->rect().x()+deltaX, container->rect().y() + deltaY,
                           size.width() - deltaX -5, size.height() - deltaY -5);
        m_baseFrame->setGeometry(container->rect());

        qreal scaleW = rect.width() / m_birdView->scene()->width();
        qreal scaleH = rect.height() / m_birdView->scene()->height();
        qreal scale = scaleH;
        if (scaleW > scaleH) {
            scale = scaleW;
        }
        if (scale > 1.0) {
            scale = 1.0;
        }
        QMatrix wm;
        wm.scale(scale, scale);
        m_birdView->setMatrix(wm);
        DEBUG(DBG_SRC) << "setting the size to the container: " << rect
                       << " / scale: " << scale;
        //crash        slotViewChanged();
    }
    else {
        DEBUG(DBG_SRC) << size << " not set!";
    }
}

/**
 * @brief BirdView::slotViewChanged
 * @param
 *
 */
void BirdView::slotViewChanged()
{
//    QMatrix matrix = m_view->matrix();
////    QRectF rect = matrix.mapRect(m_view->sceneRect());
//    QRectF rect = matrix.mapRect(QRectF(m_birdView->rect()));
//    setGeometry(rect.toRect());

//    QPoint topLeft = m_view->viewport()->rect().topLeft();
//    QSize size = m_birdView->maximumViewportSize();
//    QRect rect = QRect(topLeft.x() - hvalue, topLeft.y() - vvalue,
//                       size.width(), size.height());

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
        hlen = abs(hmax) - hmin + hpage;
        DEBUG(DBG_SRC) << "hvalue: " << hvalue << " / hlen: " << hlen
                       << " / hmin: " << hmin << " / hmax: " << hmax
                       << " / hpage: " << hpage;
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
        vlen = abs(vmax) - vmin + vpage;
        DEBUG(DBG_SRC) << "vvalue: " << vvalue << " / vlen: " << vlen
                       << " / vmin: " << vmin << " / vmax: " << vmax
                       << " / vpage: " << vpage;
    }

    int hdiv = hlen - hpage;
    int vdiv = vlen - vpage;
    if (!(hlen == 0) && !(vlen == 0) &&
        !(hdiv == 0) && !(vdiv == 0)) {
        int width = m_baseFrame->width() * hpage / hlen;
        int height = m_baseFrame->height() * vpage / vlen;
        int x = m_baseFrame->width() * (hvalue - hmin) / hdiv;
        int y = m_baseFrame->height() * (vvalue - vmin) / vdiv;
        QRect rect = QRect(x, y, width, height);
        setGeometry(rect);
        DEBUG(DBG_SRC) << "rect: " << rect;
    }
    else {
        QWidget* container = (QWidget*)m_birdView->parent();
        if (container) {
            setGeometry(container->rect());
        }
    }
}

/**
 * @brief BirdView::mousePressEvent
 * @param event
 */
void BirdView::mousePressEvent(QMouseEvent *event)
{
    m_moveStartPos = event->globalPos();
}

/**
 * @brief BirdView::mouseMoveEvent
 * @param event
 */
void BirdView::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint delta = event->globalPos() - m_moveStartPos;
    int newX = x() + delta.x();
    int newY = y() + delta.y();
    QRect limit = static_cast<QWidget*>(m_baseFrame->parent())->rect();
    DEBUG(DBG_SRC) << limit << " contains ";
    DEBUG(DBG_SRC) << "   top    left  [ " << newX << ", " << newY << "]";
    DEBUG(DBG_SRC) << "   bottom right [ " << newX + width() << ", " << newY + height() << "]";
//    if (limit.contains(newX, newY, true) &&
//        limit.contains(newX + width(), newY + height())) {
        move(newX, newY);
        m_moveStartPos = event->globalPos();
        emit viewPositionChanged(delta * 10);  //:TODO: no no no !
//    }
}

/**
 * @brief BirdView::mouseReleaseEvent
 * @param event
 */
void BirdView::mouseReleaseEvent(QMouseEvent *event)
{
    mouseMoveEvent(event);
}

/**
 * @brief BirdView::setSlots
 * @param
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
 * @brief BirdView::setBackgroundColor
 * @param frame
 * @param color
 */
void BirdView::setBackgroundColor(QFrame *frame, const QColor& color)
{
    QPalette newPalette = frame->palette();
    newPalette.setColor(frame->backgroundRole(), color);
    frame->setPalette(newPalette);
    frame->setAutoFillBackground(true);
}

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
    emit sizeChanged(event->size());
}
