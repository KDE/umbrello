/*
 *  copyright (C) 2005
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

// own header
#include "forkjoinwidget.h"
//qt includes
#include <qdom.h>
//kde includes
#include <kcursor.h>
#include <kdebug.h>
//app includes
#include "umlview.h"
#include "listpopupmenu.h"

ForkJoinWidget::ForkJoinWidget(UMLView * view, bool drawVertical, Uml::IDType id)
  : BoxWidget(view, id), m_drawVertical(drawVertical) {
    init();
}

void ForkJoinWidget::init() {
    WidgetBase::setBaseType( Uml::wt_ForkJoin );
    calculateSize();
    UMLWidget::adjustAssocs( getX(), getY() );
}

ForkJoinWidget::~ForkJoinWidget() {
}

void ForkJoinWidget::calculateSize() {
    int w = width(), h = height();
    constrain(w, h);
    setSize(w, h);
}

void ForkJoinWidget::draw(QPainter& p, int offsetX, int offsetY) {
    p.fillRect( offsetX, offsetY, width(), height(), QBrush( black ));

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY, true);
    }
}

void ForkJoinWidget::drawSelected(QPainter * p, int offsetX, int offsetY, bool resizeable) {
    if (! resizeable) {
        UMLWidget::drawSelected(p, offsetX, offsetY, resizeable);
        return;
    }
}

void ForkJoinWidget::constrain(int& width, int& height) {
    if (m_drawVertical) {
        width = 4;
        if (height < 40)
            height = 40;
    } else {
        height = 4;
        if (width < 40)
            width = 40;
    }
}

void ForkJoinWidget::slotMenuSelection(int sel) {
    switch (sel) {
    case ListPopupMenu::mt_Flip:
        setDrawVertical(!m_drawVertical);
        break;
    default:
        break;
    }
}

void ForkJoinWidget::setDrawVertical(bool to) {
    m_drawVertical = to;
    calculateSize();
    UMLWidget::adjustAssocs( getX(), getY() );
}

bool ForkJoinWidget::getDrawVertical() const {
    return m_drawVertical;
}

void ForkJoinWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement fjElement = qDoc.createElement("forkjoin");
    UMLWidget::saveToXMI(qDoc, fjElement);
    fjElement.setAttribute("drawvertical", m_drawVertical);
    qElement.appendChild(fjElement);
}

bool ForkJoinWidget::loadFromXMI(QDomElement& qElement) {
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString drawVertical = qElement.attribute("drawvertical", "0");
    setDrawVertical( (bool)drawVertical.toInt() );
    return true;
}

