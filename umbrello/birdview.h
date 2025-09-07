/*
    SPDX-FileCopyrightText: 2014 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

Q_SIGNALS:
    void viewPositionChanged(QPointF delta);

public Q_SLOTS:
    void slotViewChanged();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private Q_SLOTS:
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
