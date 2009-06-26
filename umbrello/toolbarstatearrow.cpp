/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstatearrow.h"

// qt includes
#include <QtGui/QGraphicsSceneMouseEvent>

// app includes
#include "associationwidget.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlscene.h"

#include <kdebug.h>

/**
 * Creates a new ToolBarStateArrow.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateArrow::ToolBarStateArrow(UMLScene *umlScene)
  : ToolBarState(umlScene)
{
    init();
}

/**
 * Destroys this ToolBarStateArrow.
 */
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

void ToolBarStateArrow::mousePress(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseMove(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseRelease(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}

void ToolBarStateArrow::mouseDoubleClick(QGraphicsSceneMouseEvent *ome)
{
    m_pUMLScene->callBaseMouseMethod(ome);
}


/**
 * Called when the press event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mousePressAssociation()
{
    // [PORT] getCurrentAssociation()->mousePressEvent(m_pMouseEvent);
}

/**
 * Called when the press event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mousePressWidget()
{
    // [PORT] getCurrentWidget()->mousePressEvent(m_pMouseEvent);
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

        qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();

        for (int i = 0; i < 4; i++) {
            QGraphicsLineItem* line = new QGraphicsLineItem();
            m_pUMLScene->addItem(line);

            line->setLine(x, y, x, y);
            line->setPen(QPen(QColor("grey"), 0, Qt::DotLine));
            line->setVisible(true);
            line->setZValue(100);
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
    // [PORT] getCurrentAssociation()->mouseReleaseEvent(m_pMouseEvent);
}

/**
 * Called when the release event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseReleaseWidget()
{
    // [PORT] getCurrentWidget()->mouseReleaseEvent(m_pMouseEvent);
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
        m_pUMLScene->setMenu();
    }
}

/**
 * Called when the double click event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseDoubleClickAssociation()
{
    // [PORT] getCurrentAssociation()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the double click event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseDoubleClickWidget()
{
    // [PORT] getCurrentWidget()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when an association is
 * currently available.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseMoveAssociation()
{
    // [PORT] getCurrentAssociation()->mouseMoveEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when a widget is
 * currently available.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseMoveWidget()
{
    // [PORT] getCurrentWidget()->mouseMoveEvent(m_pMouseEvent);
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
    qreal x = m_pMouseEvent->scenePos().x(), y = m_pMouseEvent->scenePos().y();
    if (m_selectionRect.count() == 4) {

        QGraphicsLineItem* line = m_selectionRect.at(0);
        line->setLine(m_startPosition.x(), m_startPosition.y(),
                      x, m_startPosition.y());

        line = m_selectionRect.at(1);
        line->setLine(x, m_startPosition.y(),
                      x, y);

        line = m_selectionRect.at(2);
        line->setLine(x, y,
                      m_startPosition.x(), y);

        line = m_selectionRect.at(3);
        line->setLine(m_startPosition.x(), y,
                      m_startPosition.x(), m_startPosition.y());

        m_pUMLScene->selectWidgets(m_startPosition.x(), m_startPosition.y(),
                                   x, y);
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
