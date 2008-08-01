/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "seqlinewidget.h"

//app includes
#include "objectwidget.h"

const qreal SeqLineWidget::DestructionBoxSize = 14;

static void addLineToPath(const QLineF& line, QPainterPath &path)
{
    if (!line.isNull()) {
        path.moveTo(line.p1());
        path.lineTo(line.p2());
    }
}

SeqLineWidget::SeqLineWidget( ObjectWidget * pObject ) :
    QGraphicsItem(pObject),
    m_objectWidget(pObject),
    m_length(250),
    m_lineColor(pObject->lineColor()),
    m_lineWidth(pObject->lineWidth())
{
    updateDestructionBoxVisibility();
}

SeqLineWidget::~SeqLineWidget()
{
}

void SeqLineWidget::setLineColor(const QColor& col)
{
    m_lineColor = col;
    update();
}

void SeqLineWidget::setLineWidth(qreal w)
{
    m_lineWidth = w;
    updateGeometry();
}

void SeqLineWidget::updateDestructionBoxVisibility()
{
    m_destructionBoxLines[0] = m_destructionBoxLines[1] = QLineF();

    if( m_objectWidget->getShowDestruction() ) {
        QRectF rect(0, 0,
                    SeqLineWidget::DestructionBoxSize, SeqLineWidget::DestructionBoxSize);
        rect.moveCenter(QPointF(0, m_length));

        m_destructionBoxLines[0].setPoints(rect.topLeft(), rect.bottomRight());
        m_destructionBoxLines[1].setPoints(rect.bottomLeft(), rect.topRight());
    }

    updateGeometry();
}

bool SeqLineWidget::onDestructionBox(const QPointF& localPos)
{
    QRectF rect(0, 0,
                SeqLineWidget::DestructionBoxSize, SeqLineWidget::DestructionBoxSize);
    rect.moveCenter(QPointF(0, m_length));

    return m_objectWidget->getShowDestruction() && rect.contains(localPos);
}

void SeqLineWidget::setLength(qreal len)
{
    m_length = len;
    const QPointF p = pos();
    m_sequentialLine.setP1(QPointF(0, 0));
    m_sequentialLine.setP2(QPointF(0, m_length));

    updateDestructionBoxVisibility();
}

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

void SeqLineWidget::updateGeometry()
{
    QPainterPath linePath;
    addLineToPath(m_sequentialLine, linePath);
    addLineToPath(m_destructionBoxLines[0], linePath);
    addLineToPath(m_destructionBoxLines[1], linePath);

    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 5);
    m_shape = stroker.createStroke(linePath);

    prepareGeometryChange();
    m_boundingRect = m_shape.boundingRect();
}
