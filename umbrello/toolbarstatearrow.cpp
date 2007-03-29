/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatearrow.h"

// app includes
#include "associationwidget.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"

ToolBarStateArrow::ToolBarStateArrow(UMLView *umlView): ToolBarState(umlView) {
    m_selectionRect.setAutoDelete(true);

    init();
}

ToolBarStateArrow::~ToolBarStateArrow() {
}

void ToolBarStateArrow::init() {
    ToolBarState::init();

    m_selectionRect.clear();
}

void ToolBarStateArrow::mousePressAssociation() {
    getCurrentAssociation()->mousePressEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mousePressWidget() {
    getCurrentWidget()->mousePressEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mousePressEmpty() {
    if (m_pMouseEvent->button() != Qt::LeftButton) {
        // Leave widgets selected upon RMB press on empty diagram area.
        // The popup menu is activated upon RMB release.
        return;
    }
    ToolBarState::mousePressEmpty();

    // Starts the selection rectangle
    if (m_selectionRect.count() == 0) {
        m_startPosition = m_pMouseEvent->pos();

        for (int i = 0; i < 4; i++) {
            QCanvasLine* line = new QCanvasLine(m_pUMLView->canvas());
            line->setPoints(m_pMouseEvent->x(), m_pMouseEvent->y(),
                            m_pMouseEvent->x(), m_pMouseEvent->y());
            line->setPen(QPen(QColor("grey"), 0, Qt::DotLine));
            line->setVisible(true);
            line->setZ(100);
            m_selectionRect.append(line);
        }
    }
}

void ToolBarStateArrow::mouseReleaseAssociation() {
    getCurrentAssociation()->mouseReleaseEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseReleaseWidget() {
    getCurrentWidget()->mouseReleaseEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseReleaseEmpty() {
    if (m_selectionRect.count() == 4) {
        m_selectionRect.clear();
    } else if (m_pMouseEvent->button() == Qt::RightButton) {
        m_pUMLView->setMenu();
    }
}

void ToolBarStateArrow::mouseDoubleClickAssociation() {
    getCurrentAssociation()->mouseDoubleClickEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseDoubleClickWidget() {
    getCurrentWidget()->mouseDoubleClickEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseMoveAssociation() {
    getCurrentAssociation()->mouseMoveEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseMoveWidget() {
    getCurrentWidget()->mouseMoveEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseMoveEmpty() {
    if (m_selectionRect.count() == 4) {
        QCanvasLine* line = m_selectionRect.at(0);
        line->setPoints(m_startPosition.x(), m_startPosition.y(),
                        m_pMouseEvent->x(), m_startPosition.y());

        line = m_selectionRect.at(1);
        line->setPoints(m_pMouseEvent->x(), m_startPosition.y(),
                        m_pMouseEvent->x(), m_pMouseEvent->y());

        line = m_selectionRect.at(2);
        line->setPoints(m_pMouseEvent->x(), m_pMouseEvent->y(),
                        m_startPosition.x(), m_pMouseEvent->y());

        line = m_selectionRect.at(3);
        line->setPoints(m_startPosition.x(), m_pMouseEvent->y(),
                        m_startPosition.x(), m_startPosition.y());

        m_pUMLView->selectWidgets(m_startPosition.x(), m_startPosition.y(),
                                  m_pMouseEvent->x(), m_pMouseEvent->y());
    }
}

void ToolBarStateArrow::changeTool() {
}

void ToolBarStateArrow::setCurrentWidget(UMLWidget* currentWidget) {
    if (currentWidget != 0 && getCurrentWidget() != 0) {
        return;
    }

    ToolBarState::setCurrentWidget(currentWidget);
}

#include "toolbarstatearrow.moc"
