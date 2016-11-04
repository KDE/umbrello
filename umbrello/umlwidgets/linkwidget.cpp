/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "linkwidget.h"

// app includes
#include "classifier.h"
#include "operation.h"
#include "uml.h"
#include "umlobject.h"
#include "umlview.h"

LinkWidget::LinkWidget()
{
}

LinkWidget::~LinkWidget()
{
}

/**
 * Motivated by FloatingTextWidget::slotMenuSelection(mt_Operation)
 */
UMLClassifier *LinkWidget::operationOwner()
{
    UMLOperation *op = operation();
    if (op == 0)
        return 0;
    return op->umlParent()->asUMLClassifier();
}

/**
 * Return the operation text.
 * When no scene parameter is given, the scene of the current view
 * is taken instead.
 * @param scene   the given scene
 * @return the operation text
 */
QString LinkWidget::operationText(UMLScene *scene)
{
    UMLOperation *op = operation();
    if (op == 0)
        return customOpText();
    if (scene == 0)
        scene = UMLApp::app()->currentView()->umlScene();
    Uml::SignatureType::Enum sigType;
    if (scene && scene->showOpSig())
        sigType = Uml::SignatureType::SigNoVis;
    else
        sigType = Uml::SignatureType::NoSigNoVis;
    QString opText = op->toString(sigType);
    return opText;
}

/**
 * Motivated by FloatingTextWidget::slotMenuSelection(mt_Reset_Label_Positions)
 * Only applies to AssociationWidget.
 */
void LinkWidget::resetTextPositions()
{
}

/**
 * Motivated by FloatingTextWidget::mouseDoubleClickEvent()
 * Only applies to AssociationWidget.
 */
void LinkWidget::showPropertiesDialog()
{
}

/**
 * Motivated by FloatingTextWidget::setLink().
 * Only applies to AssociationWidget.
 */
void LinkWidget::calculateNameTextSegment()
{
}


/**
 * Write property of QString m_SequenceNumber.
 */
void LinkWidget::setSequenceNumber(const QString &sequenceNumber)
{
    m_SequenceNumber = sequenceNumber;
}

/**
 * Read property of QString m_SequenceNumber.
 */
QString LinkWidget::sequenceNumber() const
{
    return m_SequenceNumber;
}

/**
 * Load data from XMI.
 */
bool LinkWidget::loadFromXMI(QDomElement &qElement)
{
    m_SequenceNumber = qElement.attribute(QLatin1String("seqnum"));
    return true;
}

/**
 * Save data to XMI.
 */
void LinkWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    Q_UNUSED(qDoc);

    qElement.setAttribute(QLatin1String("seqnum"), m_SequenceNumber);
}
