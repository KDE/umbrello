/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "nodewidget.h"

// app includes
#include "debug_utils.h"
#include "node.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// qt includes
#include <QPainter>
#include <QPolygon>

DEBUG_REGISTER_DISABLED(NodeWidget)

NodeWidget::NodeWidget(UMLScene * scene, UMLNode *n)
  : UMLWidget(scene, WidgetBase::wt_Node, n)
{
    setSize(100, 30);
    setZValue(1);  // above box but below UMLWidget because may embed widgets
}

NodeWidget::~NodeWidget()
{
}

void NodeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    }
    const int w = width();
    const int h = height();
    const int wDepth = (w/3 > DEPTH ? DEPTH : w/3);
    const int hDepth = (h/3 > DEPTH ? DEPTH : h/3);
    const int bodyOffsetY = hDepth;
    const int bodyWidth = w - wDepth;
    const int bodyHeight = h - hDepth;
    QFont font = UMLWidget::font();
    font.setBold(true);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
    QString nameStr = name();

    QPolygon pointArray(5);
    pointArray.setPoint(0, 0, bodyOffsetY);
    pointArray.setPoint(1, wDepth, 0);
    pointArray.setPoint(2, w - 1, 0);
    pointArray.setPoint(3, w - 1, bodyHeight);
    pointArray.setPoint(4, bodyWidth, h - 1);
    painter->drawPolygon(pointArray);
    painter->drawRect(0, bodyOffsetY, bodyWidth, bodyHeight);
    painter->drawLine(w - 1, 0, bodyWidth - 2, bodyOffsetY + 1);

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;
    if (m_umlObject) {
        QString stereotype = m_umlObject->stereotype();
        if (!stereotype.isEmpty()) {
            painter->drawText(0, bodyOffsetY + (bodyHeight/2) - fontHeight,
                       bodyWidth, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
            lines = 2;
        }
    }

    if (UMLWidget::isInstance()) {
        font.setUnderline(true);
        painter->setFont(font);
        nameStr = UMLWidget::instanceName() + " : " + nameStr;
    }

    if (lines == 1) {
        painter->drawText(0, bodyOffsetY + (bodyHeight/2) - (fontHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, nameStr);
    } else {
        painter->drawText(0, bodyOffsetY + (bodyHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, nameStr);
    }

    UMLWidget::paint(painter, option, widget);
}

QSizeF NodeWidget::minimumSize()
{
    if (m_umlObject == NULL) {
        DEBUG(DBG_SRC) << "m_umlObject is NULL";
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    QString name = m_umlObject->name();
    if (UMLWidget::isInstance()) {
        name = UMLWidget::instanceName() + " : " + name;
    }

    int width = fm.width(name);

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
    }
    if (tempWidth > width)
        width = tempWidth;
    width += DEPTH;

    int height = (2*fontHeight) + DEPTH;

    return QSizeF(width, height);
}

void NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement conceptElement = qDoc.createElement("nodewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

