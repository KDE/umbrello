/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatearrow.h"

// qt includes
#include <QMouseEvent>

// app includes
#include "associationwidget.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"

#include <kdebug.h>

ToolBarStateArrow::ToolBarStateArrow(UMLView *umlView): ToolBarState(umlView)
{
    init();
}

ToolBarStateArrow::~ToolBarStateArrow()
{
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateArrow::init()
{
    ToolBarState::init();

    while (!m_selectionRect.isEmpty())
        delete m_selectionRect.takeFirst();
    m_selectionRect.clear();
}

void ToolBarStateArrow::mousePressAssociation()
{
    getCurrentAssociation()->mousePressEvent(m_pMouseEvent);
}

/**
 * Called when the press event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mousePressWidget()
{
    getCurrentWidget()->mousePressEvent(m_pMouseEvent);
}

/**
 * Called when the press event happened on an empty space.
 * Calls base method and, if left button was pressed, prepares the selection
 * rectangle.
 */
void ToolBarStateArrow::mousePressEmpty()
{
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
            Q3CanvasLine* line = new Q3CanvasLine(m_pUMLView->canvas());
            line->setPoints(m_pMouseEvent->x(), m_pMouseEvent->y(),
                            m_pMouseEvent->x(), m_pMouseEvent->y());
            line->setPen(QPen(QColor("grey"), 0, Qt::DotLine));
            line->setVisible(true);
            line->setZ(100);
            m_selectionRect.append(line);
        }
    }
}

/**
 * Called when the release event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseReleaseAssociation()
{
    getCurrentAssociation()->mouseReleaseEvent(m_pMouseEvent);
}

void ToolBarStateArrow::mouseReleaseWidget() {
    getCurrentWidget()->mouseReleaseEvent(m_pMouseEvent);
}

/**
 * Called when the release event happened on an empty space.
 * If selection rectangle is active, it is cleared. Else, if the right
 * button was released, it shows the pop up menu for the diagram.
 */
void ToolBarStateArrow::mouseReleaseEmpty()
{
    if (m_selectionRect.count() == 4) {
        while (!m_selectionRect.isEmpty())
            delete m_selectionRect.takeFirst();
        m_selectionRect.clear();
    } else if (m_pMouseEvent->button() == Qt::RightButton) {
        m_pUMLView->setMenu();
    }
}

/**
 * Called when the double click event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseDoubleClickAssociation()
{
    getCurrentAssociation()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the double click event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseDoubleClickWidget()
{
    getCurrentWidget()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when an association is
 * currently available.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseMoveAssociation()
{
    getCurrentAssociation()->mouseMoveEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when a widget is
 * currently available.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseMoveWidget()
{
    getCurrentWidget()->mouseMoveEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when no association nor
 * widget are currently available.
 * Updates the selection rectangle to the new position and selectes all the
 * widgets in the rectangle.
 *
 * @todo Fix selection
 */
void ToolBarStateArrow::mouseMoveEmpty()
{
    if (m_selectionRect.count() == 4) {
        Q3CanvasLine* line = m_selectionRect.at(0);
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

/**
 * Overridden from base class to do nothing, as arrow is the default tool.
 */
void ToolBarStateArrow::changeTool()
{
}

/**
 * Sets the widget currently in use.
 * It ensures that the widget is only set if there is no other widget set
 * already.
 * It avoids things like moving a big widget over a little one, clicking
 * right button to cancel the movement and the little widget getting the
 * event, thus not canceling the movement in the big widget.
 */
void ToolBarStateArrow::setCurrentWidget(UMLWidget* currentWidget)
{
    if (currentWidget != 0 && getCurrentWidget() != 0) {
        return;
    }

    ToolBarState::setCurrentWidget(currentWidget);
}

#include "toolbarstatearrow.moc"
