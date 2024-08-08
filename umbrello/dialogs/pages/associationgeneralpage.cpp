/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "associationgeneralpage.h"

// local includes
#include "associationwidget.h"
#include "association.h"
#include "assocrules.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "documentationwidget.h"
#include "objectwidget.h"
#include "umldoc.h"
#include "umlobject.h"
#include "uml.h"

// kde includes
#include <kcombobox.h>
#include <QLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <ktextedit.h>

// qt includes
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>

DEBUG_REGISTER(AssociationGeneralPage)

/**
 *  Sets up the AssociationGeneralPage.
 *
 *  @param  parent  The parent to the AssociationGeneralPage.
 *  @param  assoc   The AssociationWidget to display the properties of.
 */
AssociationGeneralPage::AssociationGeneralPage (QWidget *parent, AssociationWidget *assoc)
  : DialogPageBase(parent),
    m_pAssocNameL(nullptr),
    m_pAssocNameLE(nullptr),
    m_pAssocNameComB(nullptr),
    m_pStereoChkB(nullptr),
    m_pTypeCB(nullptr),
    m_pAssociationWidget(assoc),
    m_pWidget(nullptr)
{
    constructWidget();
}

/**
 *  Standard destructor.
 */
AssociationGeneralPage::~AssociationGeneralPage()
{
}

/**
 * Construct all the widgets for this dialog.
 */
void AssociationGeneralPage::constructWidget()
{
    // general configuration of the GUI
    int margin = fontMetrics().height();
    setMinimumSize(310, 330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(6);

    // group boxes for name+type, documentation properties
    QGroupBox *nameAndTypeGB = new QGroupBox(this);
    nameAndTypeGB->setTitle(i18n("Properties"));
    topLayout->addWidget(nameAndTypeGB);

    m_pNameAndTypeLayout = new QGridLayout(nameAndTypeGB);
    m_pNameAndTypeLayout->setSpacing(6);
    m_pNameAndTypeLayout->setMargin(margin);

    // Association name
    m_pAssocNameL = new QLabel(i18nc("name of association widget", "Name:"));
    m_pNameAndTypeLayout->addWidget(m_pAssocNameL, 0, 0);

    m_pAssocNameLE = new QLineEdit(m_pAssociationWidget->name());
    m_pAssocNameComB = new KComboBox(true, nameAndTypeGB);
    m_pAssocNameComB->setDuplicatesEnabled(false);  // only allow one of each type in box

    QWidget *nameInputWidget = m_pAssocNameLE;
    UMLAssociation *umlAssoc = m_pAssociationWidget->association();
    if (umlAssoc && umlAssoc->umlStereotype()) {
        m_pAssocNameLE->hide();
        Dialog_Utils::insertStereotypesSorted(m_pAssocNameComB, umlAssoc->stereotype());
        nameInputWidget = m_pAssocNameComB;
    } else {
        m_pAssocNameComB->hide();
    }
    m_pNameAndTypeLayout->addWidget(nameInputWidget, 0, 1);
    nameInputWidget->setFocus();
    m_pAssocNameL->setBuddy(nameInputWidget);

    if (umlAssoc) {
        // stereotype checkbox
        m_pStereoChkB = new QCheckBox(i18n("Stereotype"), nameAndTypeGB);
        m_pStereoChkB->setChecked(umlAssoc->umlStereotype() != nullptr);
        connect(m_pStereoChkB, SIGNAL(stateChanged(int)), this, SLOT(slotStereoCheckboxChanged(int)));
        m_pNameAndTypeLayout->addWidget(m_pStereoChkB, 0, 2);
    }

    // type
    Uml::AssociationType::Enum currentType =  m_pAssociationWidget->associationType();
    QString currentTypeAsString = Uml::AssociationType::toStringI18n(currentType);
    QLabel *pTypeL = new QLabel(i18n("Type:"), nameAndTypeGB);
    m_pNameAndTypeLayout->addWidget(pTypeL, 1, 0);

    // Here is a list of all the supported choices for changing
    // association types.

    m_AssocTypes.clear();

    m_AssocTypes << currentType;
    logDebug1("AssociationGeneralPage::constructWidget: current type = %1",
              Uml::AssociationType::toString(currentType));

    // dynamically load all allowed associations
    for (int i = Uml::AssociationType::Generalization; i < Uml::AssociationType::Reserved;  ++i) {
        // we don't need to check for current type
        Uml::AssociationType::Enum assocType = Uml::AssociationType::fromInt(i);
        if (assocType == currentType)
            continue;

        // UMLScene based checks
        if (m_pAssociationWidget->umlScene()->type() == Uml::DiagramType::Collaboration
                && !(assocType == Uml::AssociationType::Coll_Mesg_Async
                    || assocType == Uml::AssociationType::Coll_Mesg_Sync
                    || assocType == Uml::AssociationType::Anchor))
            continue;

        if (AssocRules::allowAssociation(assocType,
                                         m_pAssociationWidget->widgetForRole(Uml::RoleType::A),
                                         m_pAssociationWidget->widgetForRole(Uml::RoleType::B))) {
            m_AssocTypes << assocType;
            logDebug1("AssociationGeneralPage::constructWidget: adding %1 to assoctype list",
                      Uml::AssociationType::toString(assocType));
        }
    }

    bool found = false;
    m_AssocTypeStrings.clear();
    for (int i = 0; i < m_AssocTypes.size(); ++i) {
        if (m_AssocTypes[i] == currentType) {
            found = true;
        }
        m_AssocTypeStrings << Uml::AssociationType::toStringI18n(m_AssocTypes[i]);
    }

    if (!found) {
        m_AssocTypes.clear();
        m_AssocTypes << currentType;
        m_AssocTypeStrings.clear();
        m_AssocTypeStrings << currentTypeAsString;
    }

    m_pTypeCB = new KComboBox(nameAndTypeGB);
    pTypeL->setBuddy(m_pTypeCB);
    m_pTypeCB->addItems(m_AssocTypeStrings);
    m_pTypeCB->setCompletedItems(m_AssocTypeStrings);

    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pNameAndTypeLayout->addWidget(m_pTypeCB, 1, 1);

    // documentation
    m_docWidget = new DocumentationWidget(m_pAssociationWidget, this);
    topLayout->addWidget(m_docWidget);
}

void AssociationGeneralPage::slotStereoCheckboxChanged(int state)
{
    QWidget  *nameInputWidget = nullptr;
    if (state) {
        m_pAssocNameLE->hide();
        m_pNameAndTypeLayout->removeWidget(m_pAssocNameLE);
        UMLAssociation *umlAssoc = m_pAssociationWidget->association();
        Dialog_Utils::insertStereotypesSorted(m_pAssocNameComB, umlAssoc->stereotype());
        nameInputWidget = m_pAssocNameComB;
    } else {
        m_pAssocNameComB->hide();
        m_pNameAndTypeLayout->removeWidget(m_pAssocNameComB);
        nameInputWidget = m_pAssocNameLE;
    }
    m_pNameAndTypeLayout->addWidget(nameInputWidget, 0, 1);
    nameInputWidget->show();
    nameInputWidget->setFocus();
    m_pAssocNameL->setBuddy(nameInputWidget);
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void AssociationGeneralPage::apply()
{
    if (m_pAssociationWidget) {
        int comboBoxItem = m_pTypeCB->currentIndex();
        Uml::AssociationType::Enum newType = m_AssocTypes[comboBoxItem];
        m_pAssociationWidget->setAssociationType(newType);
        m_docWidget->apply();
        if (m_pStereoChkB && m_pStereoChkB->isChecked()) {
            QString stereo = m_pAssocNameComB->currentText();
            // keep the order
            m_pAssociationWidget->setName(QStringLiteral(""));
            m_pAssociationWidget->setStereotype(stereo);
        } else {
            // keep the order
            m_pAssociationWidget->setStereotype(QStringLiteral(""));
            m_pAssociationWidget->setName(m_pAssocNameLE->text());
        }
    }
}

