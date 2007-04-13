/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "nodewidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "node.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

NodeWidget::NodeWidget(UMLView * view, UMLNode *n )
  : UMLWidget(view, n) {
    UMLWidget::setBaseType(Uml::wt_Node);
    setZ(m_origZ = 1);  // above box but below UMLWidget because may embed widgets
    setSize(100, 30);
    if (n && !UMLApp::app()->getDocument()->loading())
        updateComponentSize();
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
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();
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

QSize NodeWidget::calculateSize() {
    if (m_pObject == NULL) {
        kDebug() << "NodeWidget::calculateSize: m_pObject is NULL" << endl;
        return UMLWidget::calculateSize();
    }

    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight  = fm.lineSpacing();

    QString name = m_pObject->getName();
    if ( UMLWidget::getIsInstance() ) {
        name = UMLWidget::getInstanceName() + " : " + name;
    }

    int width = fm.width(name);

    int tempWidth = 0;
    if (!m_pObject->getStereotype().isEmpty()) {
        tempWidth = fm.width(m_pObject->getStereotype(true));
    }
    if (tempWidth > width)
        width = tempWidth;
    width += DEPTH;

    int height = (2*fontHeight) + DEPTH;

    return QSize(width, height);
}

void NodeWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement conceptElement = qDoc.createElement("nodewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

