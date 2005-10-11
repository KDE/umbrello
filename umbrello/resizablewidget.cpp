/*
 *  copyright (C) 2003-2005
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "resizablewidget.h"
// qt/kde includes
#include <qevent.h>
#include <kcursor.h>
#include <kdebug.h>
// app includes
#include "umlview.h"
#include "uml.h"
#include "umldoc.h"
#include "worktoolbar.h"

ResizableWidget::ResizableWidget(UMLView *view, UMLObject *o)
        : UMLWidget(view, o) {
    init();
}

ResizableWidget::ResizableWidget(UMLView * view, Uml::IDType id)
        : UMLWidget(view, id) {
    init();
}

void ResizableWidget::init() {
    m_bResizing = false;
    m_nOldH = m_nOldW = 0;
}

ResizableWidget::~ResizableWidget() {
}

void ResizableWidget::mouseMoveEvent(QMouseEvent *me) {
    if(!m_bResizing) {
        UMLWidget::mouseMoveEvent(me);
        return;
    }
    if( !m_bMouseDown )
        return;
    int newX = me->x();
    int newY = me->y();
    if (! m_bIgnoreSnapToGrid) {
        newX = m_pView->snappedX( newX );
        newY = m_pView->snappedY( newY );
    }
    int newW = m_nOldW + newX - m_nOldX - m_nPressOffsetX;
    int newH = m_nOldH + newY - m_nOldY - m_nPressOffsetY;
    constrain(newW, newH);
    setSize( newW, newH );
    adjustAssocs( getX(), getY() );
}

void ResizableWidget::mousePressEvent(QMouseEvent *me) {
    UMLWidget::mousePressEvent(me);
    if ( m_pView->getCurrentCursor() != WorkToolBar::tbb_Arrow ) {
        return;
    }
    m_nOldW = width();
    m_nOldH = height();
    const int m = 10;
    //see if clicked on bottom right corner
    if( (m_nOldX + m_nPressOffsetX) >= (getX() + width() - m) &&
        (m_nOldY + m_nPressOffsetY) >= (getY() + height() - m) && me->button() == Qt::LeftButton) {
        m_bResizing = true;
        m_pView->setCursor(WidgetBase::m_Type == Uml::wt_Message ?
                           KCursor::sizeVerCursor() : KCursor::sizeFDiagCursor());
    } else {
        m_bResizing = false;
        m_pView -> setCursor(KCursor::arrowCursor());
    }
}

void ResizableWidget::mouseReleaseEvent(QMouseEvent* me) {
    UMLWidget::mouseReleaseEvent(me);
    if (m_bResizing) {
        m_bResizing = false;
        m_pView -> setCursor( KCursor::arrowCursor() );
        UMLApp::app()->getDocument()->setModified(true);
    }
}

void ResizableWidget::calcMinWidthAndHeight(int& width, int& height) {
    width = 20;
    height = 20;
}

void ResizableWidget::constrain(int& width, int& height) {
    int minW, minH;
    calcMinWidthAndHeight(minW, minH);
    if (width < minW)
        width = minW;
    if (height < minH)
        height = minH;
}

