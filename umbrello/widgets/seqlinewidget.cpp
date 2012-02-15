/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "seqlinewidget.h"

//app includes
#include "objectwidget.h"

/// The width and height of the destruction box.
const qreal SeqLineWidget::DestructionBoxSize = 14;

/**
 * @internal Utility method to add line to path if line is non null.
 */
static void addLineToPath(const QLineF& line, QPainterPath &path)
{
    if (!line.isNull()) {
        path.moveTo(line.p1());
        path.lineTo(line.p2());
    }
}

/**
 * Constructs a Sequential line widget.
 *
 * @param pObject The ObjectWidget to which this widget belongs to.
 */
SeqLineWidget::SeqLineWidget(ObjectWidget * pObject)
  : QGraphicsItem(pObject), // also make object widget as parent
    m_objectWidget(pObject),
    m_length(250),
    m_lineColor(pObject->lineColor()),
    m_lineWidth(pObject->lineWidth())
{
    updateDestructionBoxVisibility();
}

/**
 * Destructor.
 */
SeqLineWidget::~SeqLineWidget()
{
}

/**
 * Sets the color of lines drawn.
 */
void SeqLineWidget::setLineColor(const QColor& col)
{
    m_lineColor = col;
    update();
}

/**
 * Sets the width of the lines drawn.
 */
void SeqLineWidget::setLineWidth(int w)
{
    m_lineWidth = w;
    updateGeometry();
}

/**
 * Creates/destroyes the lines corresponding to destruction box based
 * on whether it should be shown or not.
 */
void SeqLineWidget::updateDestructionBoxVisibility()
{
    m_destructionBoxLines[0] = m_destructionBoxLines[1] = QLineF();

    if( m_objectWidget->showDestruction() ) {
        QRectF rect(0, 0,
                    SeqLineWidget::DestructionBoxSize, SeqLineWidget::DestructionBoxSize);
        rect.moveCenter(QPointF(0, m_length));

        m_destructionBoxLines[0].setPoints(rect.topLeft(), rect.bottomRight());
        m_destructionBoxLines[1].setPoints(rect.bottomLeft(), rect.topRight());
    }

    updateGeometry();
}

/**
 * @retval true If \a localPos is inside destruction box.
 */
bool SeqLineWidget::onDestructionBox(const QPointF& localPos)
{
    QRectF rect(0, 0,
                SeqLineWidget::DestructionBoxSize, SeqLineWidget::DestructionBoxSize);
    rect.moveCenter(QPointF(0, m_length));

    return m_objectWidget->showDestruction() && rect.contains(localPos);
}

/**
 * Sets the length of the sequential line of this widget to \a len.
 */
void SeqLineWidget::setLength(qreal len)
{
    m_length = len;
    m_sequentialLine.setP1(QPointF(0, 0));
    m_sequentialLine.setP2(QPointF(0, m_length));

    if (m_objectWidget) {
        m_objectWidget->alignPreconditionWidgets();
    }

    updateDestructionBoxVisibility();
}

/**
 * Sets 'y' coordinate  of end of line.
 *
 * @param yPos This represents 'y' in scene coords which will become
 *             end of line.
 */
void SeqLineWidget::setEndOfLine(qreal yPos)
{
    qreal len = yPos - scenePos().y();
//:TODO:    if (len > 0.0) {
        setLength(len);
//    }
}

/**
 * Reimplemented from QGraphicsItem::paint to draw the sequential line
 * and also the destruction box if it is shown.
 */
void SeqLineWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QPen pen(m_lineColor, m_lineWidth, Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(m_sequentialLine);

    if (!m_destructionBoxLines[0].isNull() && !m_destructionBoxLines[1].isNull()) {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(m_lineWidth + 2);
        painter->setPen(pen);

        painter->drawLines(m_destructionBoxLines, 2);
    }
}

/**
 * Calculates the new shape and new bounding rect for this widget
 * based on current values of m_sequentialLine and
 * m_destructionBoxLines.
 */
void SeqLineWidget::updateGeometry()
{
    QPainterPath linePath;
    addLineToPath(m_sequentialLine, linePath);
    addLineToPath(m_destructionBoxLines[0], linePath);
    addLineToPath(m_destructionBoxLines[1], linePath);

    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 10);
    m_shape = stroker.createStroke(linePath);

    prepareGeometryChange();
    m_boundingRect = m_shape.boundingRect();
}
