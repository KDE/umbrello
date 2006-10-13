/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
    UMLWidget::updateComponentSize();
}

ForkJoinWidget::~ForkJoinWidget() {
}

QSize ForkJoinWidget::calculateSize() {
    if (m_drawVertical) {
        return QSize(4, 40);
    } else {
        return QSize(40, 4);
    }
}

void ForkJoinWidget::draw(QPainter& p, int offsetX, int offsetY) {
    p.fillRect( offsetX, offsetY, width(), height(), QBrush( Qt::black ));

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void ForkJoinWidget::drawSelected(QPainter *, int /*offsetX*/, int /*offsetY*/) {
}

void ForkJoinWidget::constrain(int& width, int& height) {
    if (m_drawVertical) {
        if (width < 4)
            width = 4;
        else if (width > 10)
            width = 10;
        if (height < 40)
            height = 40;
        else if (height > 100)
            height = 100;
    } else {
        if (height < 4)
            height = 4;
        else if (height > 10)
            height = 10;
        if (width < 40)
            width = 40;
        else if (width > 100)
            width = 100;
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
    updateComponentSize();
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

