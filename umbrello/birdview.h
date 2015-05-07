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
#ifndef BIRDVIEW_H
#define BIRDVIEW_H

#include <QDockWidget>
#include <QFrame>

class UMLView;

QT_BEGIN_NAMESPACE
class QGraphicsView;
QT_END_NAMESPACE

/**
 * @brief The class BirdViewDockWidget contains the bird view.
 *
 * BirdViewDockWidget is a subclass of QDockWidget.
 * It reimplements the resize event handler by emitting a size changed signal.
 *
 * @author Andi Fischer
 */
class BirdViewDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit BirdViewDockWidget(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0);

signals:
    void sizeChanged(const QSize& size);

protected:
    virtual void resizeEvent(QResizeEvent *event);
};

/**
 * @brief The class BirdView controls the view of the whole scene.
 *
 * A view of the whole scene is shown together with a representation of the
 * working window. This working window can be moved with the mouse and will
 * therefore change the visible view of the working window.
 * A frame is laid over the graphics view to prevent mouse events acting on the
 * graphics elements.
 *
 * @author Andi Fischer
 */
class BirdView : public QFrame
{
    Q_OBJECT
public:
    BirdView(QDockWidget *parent, UMLView *view);
    virtual ~BirdView();

    void setSlotsEnabled(bool enabled);

signals:
    void viewPositionChanged(QPoint delta);

public slots:
    void slotViewChanged();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void slotDockSizeChanged(const QSize &size);
    void slotDestroyed(QObject *object);

private:
    UMLView*       m_view;
    QGraphicsView* m_birdView;
    QFrame*        m_protectFrame;
    QPoint         m_moveStartPos;

    static void setBackgroundColor(QFrame* frame, const QColor& color);
};

#endif // BIRDVIEW_H
