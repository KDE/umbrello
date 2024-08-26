/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "seqlinewidget.h"

//app includes
#include "debug_utils.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "umlscene.h"
#include "umlview.h"
#include "widgetbasepopupmenu.h"

//qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(SeqLineWidget)

// class members
int const SeqLineWidget::m_nMouseDownEpsilonX = 20;

/**
 * Constructor.
 */
SeqLineWidget::SeqLineWidget(UMLScene *scene, ObjectWidget * pObject)
  : QGraphicsLineItem(),
    m_scene(scene)
{
    scene->addItem(this);
    m_pObject = pObject;
    setPen(QPen(m_pObject->lineColor(), 0, Qt::DashLine));
    setZValue(0);
    setVisible(true);
    m_nLengthY = 250;
    setupDestructionBox();
}

/**
 * Destructor.
 */
SeqLineWidget::~SeqLineWidget()
{
    cleanupDestructionBox();
}

/**
 * Return whether point is on sequence line.
 * Takes into account destruction box if shown.
 *
 * @param p The point to investigate.
 * @return  True if point is on this sequence line.
 */
bool SeqLineWidget::onWidget(const QPointF & p)
{
    bool isOnWidget = false;
    QPointF sp = line().p1();
    QPointF ep = line().p2();
    //see if on widget (for message creation)
    if (sp.x() - m_nMouseDownEpsilonX < p.x()
            && ep.x() + m_nMouseDownEpsilonX > p.x()
            && sp.y() < p.y() && ep.y() + 3 > p.y())
    {
        isOnWidget = true;
    }
    return isOnWidget;
}

/**
 * Return whether point is on the destruction box.
 *
 * @param p The point to investigate.
 * @return  True if point is on the destruction box of this sequence line.
 */
bool SeqLineWidget::onDestructionBox(const QPointF & p)
{
    bool isOnDestructionBox = false;
    int x = m_pObject->x() + m_pObject->width() / 2;
    int y = m_pObject->y() + m_pObject->height() + m_nLengthY;

    //see if on destruction box
    if (!m_pObject->showDestruction()) {
        return false;
    }
    if (x - 10 < p.x() && x + 10 > p.x()
            && y - 10 < p.y() && y + 10 > p.y())
    {
        isOnDestructionBox = true;
    }
    return isOnDestructionBox;
}

/**
 * Clean up anything before deletion.
 */
void SeqLineWidget::cleanup()
{
    cleanupDestructionBox();
}

/**
 * Set the start point of the line.
 *
 * @param startX    X coordinate of the start point.
 * @param startY    Y coordinate of the start point.
 */
void SeqLineWidget::setStartPoint(int startX, int startY)
{
    int endX = startX;
    int endY = startY + m_nLengthY;
    QGraphicsLineItem::setLine(startX, startY, endX, endY);
    moveDestructionBox();
}

/**
 * Clean up destruction box.
 */
void SeqLineWidget::cleanupDestructionBox()
{
    if (m_DestructionBox.line1) {
        delete m_DestructionBox.line1;
        m_DestructionBox.line1 = nullptr;
        delete m_DestructionBox.line2;
        m_DestructionBox.line2 = nullptr;
    }
}

/**
 * Set up destruction box.
 */
void SeqLineWidget::setupDestructionBox()
{
    cleanupDestructionBox();
    if(!m_pObject->showDestruction()) {
        return;
    }
    QRect rect;
    rect.setX(m_pObject->x() + m_pObject->width() / 2 - 7);
    rect.setY(m_pObject->y() + m_pObject->height() + m_nLengthY - 7);
    rect.setWidth(14);
    rect.setHeight(14);

    m_DestructionBox.line1 = new QGraphicsLineItem;
    m_scene->addItem(m_DestructionBox.line1);
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.line1->setVisible(true);
    m_DestructionBox.line1->setPen(QPen(m_pObject->lineColor(), 2));
    m_DestructionBox.line1->setZValue(3);

    m_DestructionBox.line2 = new QGraphicsLineItem;
    m_scene->addItem(m_DestructionBox.line2);
    m_DestructionBox.setLine2Points(rect);
    m_DestructionBox.line2->setVisible(true);
    m_DestructionBox.line2->setPen(QPen(m_pObject->lineColor(), 2));
    m_DestructionBox.line2->setZValue(3);
}

/**
 * Move destruction box.
 */
void SeqLineWidget::moveDestructionBox()
{
    if(!m_DestructionBox.line1) {
        return;
    }
    QRect rect;
    rect.setX(m_pObject->x() + m_pObject->width() / 2 - 7);
    rect.setY(m_pObject->y() + m_pObject->height() + m_nLengthY - 7);
    rect.setWidth(14);
    rect.setHeight(14);
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.setLine2Points(rect);
}

/**
 * Sets the y position of the bottom of the vertical line.
 *
 * @param yPosition The y coordinate for the bottom of the line.
 */
void SeqLineWidget::setEndOfLine(int yPosition)
{
    QPointF sp = line().p1();
    int newY = yPosition;
    m_nLengthY = yPosition - m_pObject->y() - m_pObject->height();
    // normally the managing Objectwidget is responsible for the call of this function
    // but to be sure - make a double check _against current position_
    if (m_nLengthY < 0) {
        m_nLengthY = 0;
        newY = m_pObject->y() + m_pObject->height();
    }
    setLine(sp.x(), sp.y(), sp.x(), newY);
    moveDestructionBox();
}

void SeqLineWidget::setLineColorCmd(const QColor &color)
{
    QPen pen = this->pen();
    pen.setColor(color);
    setPen(pen);
    m_DestructionBox.setLineColorCmd(color);
}

void SeqLineWidget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    m_pObject->contextMenuEvent(event);
}
