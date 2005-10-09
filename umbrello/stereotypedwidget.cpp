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

#include "stereotypedwidget.h"

StereotypedWidget::StereotypedWidget(UMLView *view, UMLObject *o)
  : UMLWidget(view, o) {
}

StereotypedWidget::~StereotypedWidget() {}

void StereotypedWidget::setShowStereotype(bool _status) {
    m_bShowStereotype = _status;
    calculateSize();
    update();
}

bool StereotypedWidget::getShowStereotype() const {
    return m_bShowStereotype;
}

void StereotypedWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    UMLWidget::saveToXMI(qDoc, qElement);
    qElement.setAttribute("showstereotype", m_bShowStereotype);
}

bool StereotypedWidget::loadFromXMI(QDomElement& qElement) {
    if ( !UMLWidget::loadFromXMI(qElement) ) {
        return false;
    }
    QString showstereo = qElement.attribute("showstereotype", "0");
    m_bShowStereotype = (bool)showstereo.toInt();
    return true;
}

