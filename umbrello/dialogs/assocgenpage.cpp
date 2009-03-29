/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocgenpage.h"

// local includes
#include "association.h"
#include "dialog_utils.h"
#include "assocrules.h"

// kde includes
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// qt includes
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

AssocGenPage::AssocGenPage (UMLDoc *d, QWidget *parent, AssociationWidget *assoc)
        : QWidget(parent)
{
    m_pAssociationWidget = assoc;
    m_pWidget = 0;
    m_pTypeCB = 0;
    m_pAssocNameLE = 0;
    m_pUmldoc = d;

    constructWidget();
}

AssocGenPage::~AssocGenPage()
{
}

void AssocGenPage::constructWidget()
{
    // general configuration of the GUI
    int margin = fontMetrics().height();
    setMinimumSize(310,330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(6);

    // group boxes for name, documentation properties
    QGroupBox *nameGB = new QGroupBox(this);
    QGroupBox *docGB = new QGroupBox(this);
    nameGB->setTitle(i18n("Properties"));
    docGB->setTitle(i18n("Documentation"));
    topLayout->addWidget(nameGB);
    topLayout->addWidget(docGB);

    QGridLayout * nameLayout = new QGridLayout(nameGB);
    nameLayout->setSpacing(6);
    nameLayout->setMargin(margin);

    //Association name
    QLabel *pAssocNameL = NULL;
    KLineEdit* nameField = Dialog_Utils::makeLabeledEditField( nameGB, nameLayout, 0,
                           pAssocNameL, i18nc("name of association widget", "Name:"),
                           m_pAssocNameLE, m_pAssociationWidget->getName() );
    nameField->setFocus();

    // document
    QHBoxLayout * docLayout = new QHBoxLayout(docGB);
    docLayout->setMargin(margin);

    m_pDoc = new KTextEdit(docGB);
    docLayout->addWidget(m_pDoc);
    m_pDoc-> setText(m_pAssociationWidget-> getDoc());
    Uml::Association_Type currentType =  m_pAssociationWidget->getAssocType();
    QString currentTypeAsString = UMLAssociation::toString(currentType);
    QLabel *pTypeL = new QLabel(i18n("Type:"), nameGB);
    nameLayout->addWidget(pTypeL, 1, 0);

    /* Here is a list of all the supported choices for changing
       association types */

    m_AssocTypes.clear();

    m_AssocTypes << currentType;

    // dynamically load all allowed associations
    for ( int i = Uml::at_Generalization; i<= Uml::at_Relationship ;  ++i ) {
        // we don't need to check for current type
        if (  ( Uml::Association_Type )i == currentType )
            continue;

        if ( AssocRules::allowAssociation( ( Uml::Association_Type )i, m_pAssociationWidget->getWidget( Uml::A ),
                                           m_pAssociationWidget->getWidget( Uml::B ))
             ) {
            m_AssocTypes << (Uml::Association_Type)i;
        }
    }

    bool found=false;
    m_AssocTypeStrings.clear();
    for (int i = 0; i < m_AssocTypes.size(); ++i) {
        if (m_AssocTypes[i] == currentType) found=true;
        QString typeStr = UMLAssociation::toString(m_AssocTypes[i]);
        m_AssocTypeStrings << typeStr;
    }

    if (!found) {
        m_AssocTypes.clear();
        m_AssocTypes << currentType;
        m_AssocTypeStrings.clear();
        m_AssocTypeStrings << currentTypeAsString;
    }

    m_pTypeCB = new KComboBox(nameGB);
    pTypeL->setBuddy(m_pTypeCB);
    m_pTypeCB->addItems(m_AssocTypeStrings);
    m_pTypeCB->setCompletedItems(m_AssocTypeStrings);

    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    m_pDoc->setWordWrapMode(QTextOption::WordWrap);
    nameLayout->addWidget(m_pTypeCB, 1, 1);
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void AssocGenPage::updateObject()
{
    if (m_pAssociationWidget) {
        int comboBoxItem = m_pTypeCB->currentIndex();
        Uml::Association_Type newType = m_AssocTypes[comboBoxItem];
        m_pAssociationWidget->setAssocType(newType);
        m_pAssociationWidget->setName(m_pAssocNameLE->text());
        m_pAssociationWidget->setDoc(m_pDoc->toPlainText());
    }
}

#include "assocgenpage.moc"
