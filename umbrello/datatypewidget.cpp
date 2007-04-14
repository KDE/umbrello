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
#include "datatypewidget.h"

// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>

// app includes
#include "classifier.h"
#include "operation.h"
#include "classifierlistitem.h"
#include "umlview.h"
#include "umldoc.h"
#include "listpopupmenu.h"


#define CIRCLE_SIZE 30

DatatypeWidget::DatatypeWidget(UMLView* view, UMLClassifier *d) : UMLWidget(view, d) {
    init();
}

DatatypeWidget::~DatatypeWidget() {}

void DatatypeWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Datatype);
    setSize(100, 30);
    m_pMenu = 0;
}

void DatatypeWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    if (UMLWidget::getUseFillColour())  {
        p.setBrush(UMLWidget::getFillColour());
    } else {
        p.setBrush(m_pView->viewport()->backgroundColor());
    }

    int w = width();
    int h = height();

    QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    int fontHeight  = fm.lineSpacing();
    QString name = getName();

    p.drawRect(offsetX, offsetY, w, h);
    p.setPen(QPen(Qt::black));

    QFont font = UMLWidget::getFont();
    font.setBold(true);
    p.setFont(font);
    p.drawText(offsetX + DATATYPE_MARGIN, offsetY,
               w - DATATYPE_MARGIN* 2,fontHeight,
               Qt::AlignCenter, m_pObject->getStereotype(true));

    font.setItalic( m_pObject->getAbstract() );
    p.setFont(font);
    p.drawText(offsetX + DATATYPE_MARGIN, offsetY + fontHeight,
               w - DATATYPE_MARGIN * 2, fontHeight, Qt::AlignCenter, name);

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSize DatatypeWidget::calculateSize() {
    if (!m_pObject)  {
        return UMLWidget::calculateSize();
    }
    int width, height;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;//always have one line - for name
    lines++; //for the stereotype

    height = width = 0;
    height += lines * fontHeight;

    //now set the width of the concept
    //set width to name to start with
    //set width to name to start with
    width = getFontMetrics(FT_BOLD_ITALIC).boundingRect(m_pObject->getFullyQualifiedName()).width();
    int w = getFontMetrics(FT_BOLD).boundingRect(m_pObject->getStereotype(true)).width();

    width = w > width?w:width;

    //allow for width margin
    width += DATATYPE_MARGIN * 2;

    return QSize(width, height);
}

void DatatypeWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement conceptElement = qDoc.createElement("datatypewidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

bool DatatypeWidget::loadFromXMI( QDomElement & qElement ) {
    return UMLWidget::loadFromXMI(qElement);
}

