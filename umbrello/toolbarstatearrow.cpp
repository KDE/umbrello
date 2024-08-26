/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "toolbarstatearrow.h"

// app includes
#include "associationwidget.h"
#include "umlscene.h"
#include "umlwidget.h"

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
    cleanup();
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateArrow::init()
{
    ToolBarState::init();

    cleanup();
}

/**
 * Clean up anything before deletion.
 */
void ToolBarStateArrow::cleanup()
{
    while (!m_selectionRect.isEmpty())
        delete m_selectionRect.takeFirst();
    m_selectionRect.clear();
}

/**
 * Called when the press event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mousePressAssociation()
{
    currentAssociation()->mousePressEvent(m_pMouseEvent);
}

/**
 * Called when the press event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mousePressWidget()
{
    currentWidget()->mousePressEvent(m_pMouseEvent);
}

/**
 * Called when the press event happened on an empty space.
 * Calls base method and, if left button was pressed, prepares the selection
 * rectangle.
 */
void ToolBarStateArrow::mousePressEmpty()
{
    if (!m_pMouseEvent)
        return;

    if (m_pMouseEvent->button() != Qt::LeftButton) {
        // Leave widgets selected upon RMB press on empty diagram area.
        // The popup menu is activated upon RMB release.
        return;
    }
    ToolBarState::mousePressEmpty();

    // Starts the selection rectangle
    // TODO: let QGraphicsScene show the selection rectangle
    if (m_selectionRect.count() == 0) {
        m_startPosition = m_pMouseEvent->scenePos();

        for (int i = 0; i < 4; i++) {
            QGraphicsLineItem* line = new QGraphicsLineItem();
            m_pUMLScene->addItem(line);
            line->setLine(m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y(),
                          m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y());
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
    currentAssociation()->mouseReleaseEvent(m_pMouseEvent);
}

/**
 * Called when the release event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseReleaseWidget()
{
    currentWidget()->mouseReleaseEvent(m_pMouseEvent);
}

/**
 * Called when the release event happened on an empty space.
 * If selection rectangle is active, it is cleared. Else, if the right
 * button was released, it shows the pop up menu for the diagram.
 */
void ToolBarStateArrow::mouseReleaseEmpty()
{
    cleanup();
}

/**
 * Called when the double click event happened on an association.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseDoubleClickAssociation()
{
    currentAssociation()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the double click event happened on a widget.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseDoubleClickWidget()
{
    currentWidget()->mouseDoubleClickEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when an association is
 * currently available.
 * Delivers the event to the association.
 */
void ToolBarStateArrow::mouseMoveAssociation()
{
    currentAssociation()->mouseMoveEvent(m_pMouseEvent);
}

/**
 * Called when the move event happened when a widget is
 * currently available.
 * Delivers the event to the widget.
 */
void ToolBarStateArrow::mouseMoveWidget()
{
    currentWidget()->mouseMoveEvent(m_pMouseEvent);
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
        QGraphicsLineItem* line = m_selectionRect.at(0);
        line->setLine(m_startPosition.x(), m_startPosition.y(),
                      m_pMouseEvent->scenePos().x(), m_startPosition.y());

        line = m_selectionRect.at(1);
        line->setLine(m_pMouseEvent->scenePos().x(), m_startPosition.y(),
                      m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y());

        line = m_selectionRect.at(2);
        line->setLine(m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y(),
                      m_startPosition.x(), m_pMouseEvent->scenePos().y());

        line = m_selectionRect.at(3);
        line->setLine(m_startPosition.x(), m_pMouseEvent->scenePos().y(),
                      m_startPosition.x(), m_startPosition.y());

        m_pUMLScene->selectWidgets(m_startPosition.x(), m_startPosition.y(),
                                   m_pMouseEvent->scenePos().x(), m_pMouseEvent->scenePos().y());
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
void ToolBarStateArrow::setCurrentWidget(UMLWidget* widget)
{
    if (widget != nullptr && currentWidget() != nullptr) {
        return;
    }

    ToolBarState::setCurrentWidget(widget);
}

