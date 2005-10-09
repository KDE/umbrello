/*
 *  copyright (C) 2003-2004
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

#include "nodewidget.h"
#include "node.h"
#include "umlview.h"
#include <kdebug.h>
#include <qpainter.h>

NodeWidget::NodeWidget(UMLView * view, UMLNode *n )
  : ResizableWidget(view, n) {
    UMLWidget::setBaseType(Uml::wt_Node);
    calculateSize();
    update();
    setSize(100, 30);
    calculateSize();
}

NodeWidget::~NodeWidget() {}

void NodeWidget::draw(QPainter & p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if ( UMLWidget::getUseFillColour() ) {
        p.setBrush( UMLWidget::getFillColour() );
    } else {
        p.setBrush( m_pView->viewport()->backgroundColor() );
    }
    const int w = width();
    const int h = height();
    const int wDepth = (w/3 > DEPTH ? DEPTH : w/3);
    const int hDepth = (h/3 > DEPTH ? DEPTH : h/3);
    const int bodyOffsetY = offsetY + hDepth;
    const int bodyWidth = w - wDepth;
    const int bodyHeight = h - hDepth;
    QFont font = UMLWidget::getFont();
    font.setBold(true);
    QFontMetrics &fm = getFontMetrics(FT_BOLD);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();

    QPointArray pointArray(5);
    pointArray.setPoint(0, offsetX, bodyOffsetY);
    pointArray.setPoint(1, offsetX + wDepth, offsetY);
    pointArray.setPoint(2, offsetX + w - 1, offsetY);
    pointArray.setPoint(3, offsetX + w - 1, offsetY + bodyHeight );
    pointArray.setPoint(4, offsetX + bodyWidth, offsetY + h - 1);
    p.drawPolygon(pointArray);
    p.drawRect(offsetX, bodyOffsetY, bodyWidth, bodyHeight);
    p.drawLine(offsetX + w - 1, offsetY, offsetX + bodyWidth - 2, bodyOffsetY + 1);

    p.setPen( QPen(Qt::black) );
    p.setFont(font);

    int lines = 1;
    if (m_pObject) {
        QString stereotype = m_pObject->getStereotype();
        if (!stereotype.isEmpty()) {
            p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - fontHeight,
                       bodyWidth, fontHeight, Qt::AlignCenter, m_pObject->getStereotype(true));
            lines = 2;
        }
    }

    if ( UMLWidget::getIsInstance() ) {
        font.setUnderline(true);
        p.setFont(font);
        name = UMLWidget::getInstanceName() + " : " + name;
    }

    if (lines == 1) {
        p.drawText(offsetX, bodyOffsetY + (bodyHeight/2) - (fontHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, name);
    } else {
        p.drawText(offsetX, bodyOffsetY + (bodyHeight/2),
                   bodyWidth, fontHeight, Qt::AlignCenter, name);
    }

    if(m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void NodeWidget::calculateSize() {
    int width, height;
    calcMinWidthAndHeight(width, height);
    if (getWidth() >= width && getHeight() >= height)
        return;

    setSize(width, height);
    adjustAssocs( getX(), getY() );//adjust assoc lines
}

void NodeWidget::calcMinWidthAndHeight(int& width, int& height) {
    width = height = 0;
    if (m_pObject == NULL) {
        kdDebug() << "NodeWidget::calcMinWidthAndHeight: m_pObject is NULL" << endl;
        return;
    }

    QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    int fontHeight  = fm.lineSpacing();

    QString name = m_pObject->getName();
    if ( UMLWidget::getIsInstance() ) {
        name = UMLWidget::getInstanceName() + " : " + name;
    }

    width = fm.width(name);

    int tempWidth = 0;
    if (!m_pObject->getStereotype().isEmpty()) {
        tempWidth = fm.width(m_pObject->getStereotype(true));
    }
    if (tempWidth > width)
        width = tempWidth;
    width += DEPTH;

    height = (2*fontHeight) + DEPTH;
}

void NodeWidget::constrain(int& width, int& height) {
    int minWidth, minHeight;
    calcMinWidthAndHeight(minWidth, minHeight);
    if (width < minWidth)
        width = minWidth;
    if (height < minHeight)
        height = minHeight;
}

void NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement conceptElement = qDoc.createElement("nodewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

