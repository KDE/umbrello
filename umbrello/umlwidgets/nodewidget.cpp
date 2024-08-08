/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "nodewidget.h"

// app includes
#include "debug_utils.h"
#include "node.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QPainter>
#include <QPolygon>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(NodeWidget)

/**
 * Constructs a NodeWidget.
 *
 * @param scene   The parent of this NodeWidget.
 * @param n       The UMLNode this will be representing.
 */
NodeWidget::NodeWidget(UMLScene * scene, UMLNode *n)
  : UMLWidget(scene, WidgetBase::wt_Node, n)
{
    setSize(100, 30);
    setZValue(1);  // above box but below UMLWidget because may embed widgets
}

/**
 * Destructor.
 */
NodeWidget::~NodeWidget()
{
}

/**
 * Overrides standard method.
 */
void NodeWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    setPenFromSettings(painter);
    if (UMLWidget::useFillColor()) {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->backgroundColor());
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
    pointArray.setPoint(2, w, 0);
    pointArray.setPoint(3, w, bodyHeight);
    pointArray.setPoint(4, bodyWidth, h);
    painter->drawPolygon(pointArray);
    painter->drawRect(0, bodyOffsetY, bodyWidth, bodyHeight);
    painter->drawLine(w, 0, bodyWidth, bodyOffsetY);

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
        nameStr = UMLWidget::instanceName() + QStringLiteral(" : ") + nameStr;
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

/**
 * Overrides method from UMLWidget.
 */
QSizeF NodeWidget::minimumSize() const
{
    if (m_umlObject == nullptr) {
        DEBUG() << "m_umlObject is NULL";
        return UMLWidget::minimumSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    QString name = m_umlObject->name();
    if (UMLWidget::isInstance()) {
        name = UMLWidget::instanceName() + QStringLiteral(" : ") + name;
    }

    int width = fm.width(name) + 2 * defaultMargin;

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

/**
 * Saves to the "nodewidget" XMI element.
 * Note: For loading we use the method inherited from UMLWidget.
 */
void NodeWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("nodewidget"));
    UMLWidget::saveToXMI(writer);
    writer.writeEndElement();
}

