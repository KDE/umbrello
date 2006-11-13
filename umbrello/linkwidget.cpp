/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "linkwidget.h"
// qt/kde includes
#include <kdebug.h>
// app includes
#include "umlview.h"
#include "umlobject.h"
#include "classifier.h"
#include "operation.h"
#include "uml.h"

LinkWidget::LinkWidget() {
}

LinkWidget::~LinkWidget() {
}

UMLClassifier *LinkWidget::getOperationOwner() {
    UMLOperation *op = getOperation();
    if (op == NULL)
        return NULL;
    return static_cast<UMLClassifier*>(op->parent());
}

QString LinkWidget::getOperationText(UMLView *view /* = NULL */) {
    UMLOperation *op = getOperation();
    if (op == NULL)
        return getCustomOpText();
    if (view == NULL)
        view = UMLApp::app()->getCurrentView();
    Uml::Signature_Type sigType;
    if (view && view->getShowOpSig())
        sigType = Uml::st_SigNoVis;
    else
        sigType = Uml::st_NoSigNoVis;
    QString opText = op->toString(sigType);
    return opText;
}

void LinkWidget::resetTextPositions() {
}

bool LinkWidget::showDialog() {
    return true;
}

void LinkWidget::calculateNameTextSegment() {
}


