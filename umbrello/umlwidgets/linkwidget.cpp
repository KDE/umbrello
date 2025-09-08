/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "linkwidget.h"

// app includes
#include "umlclassifier.h"
#include "debug_utils.h"
#include "umloperation.h"
#include "uml.h"
#include "umlobject.h"
#include "umlview.h"

// qt includes
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(LinkWidget)

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
    if (op == nullptr)
        return nullptr;
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
    if (op == nullptr)
        return customOpText();
    if (scene == nullptr)
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
 * Motivated by FloatingTextWidget::showPropertiesDialog()
 */
bool LinkWidget::showPropertiesDialog()
{
    return false;
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
    m_SequenceNumber = qElement.attribute(QStringLiteral("seqnum"));
    return true;
}

/**
 * Save data to XMI.
 */
void LinkWidget::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeAttribute(QStringLiteral("seqnum"), m_SequenceNumber);
}
