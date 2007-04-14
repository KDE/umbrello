/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "assocgenpage.h"

// qt includes
#include <qlayout.h>
#include <kcombobox.h>

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// local includes
#include "../association.h"
#include "../dialog_utils.h"

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

AssocGenPage::~AssocGenPage() {}

void AssocGenPage::constructWidget() {

    // general configuration of the GUI
    int margin = fontMetrics().height();
    setMinimumSize(310,330);
    QVBoxLayout * topLayout = new QVBoxLayout(this);
    topLayout -> setSpacing(6);

    // group boxes for name, documentation properties
    QGroupBox *nameGB = new QGroupBox(this);
    QGroupBox *docGB = new QGroupBox(this);
    nameGB -> setTitle(i18n("Properties"));
    docGB -> setTitle(i18n("Documentation"));
    topLayout -> addWidget(nameGB);
    topLayout -> addWidget(docGB);

    QGridLayout * nameLayout = new QGridLayout(nameGB, 2, 2);
    nameLayout -> setSpacing(6);
    nameLayout -> setMargin(margin);

    //Association name
    QLabel *pAssocNameL = NULL;
    QLineEdit* nameField = Dialog_Utils::makeLabeledEditField( nameGB, nameLayout, 0,
                           pAssocNameL, i18n("Name:"),
                           m_pAssocNameLE, m_pAssociationWidget->getName() );
    nameField->setFocus();

    // document
    QHBoxLayout * docLayout = new QHBoxLayout(docGB);
    docLayout -> setMargin(margin);

    m_pDoc = new QMultiLineEdit(docGB);
    docLayout -> addWidget(m_pDoc);
    m_pDoc-> setText(m_pAssociationWidget-> getDoc());
    Uml::Association_Type currentType =  m_pAssociationWidget->getAssocType();
    QString currentTypeAsString = UMLAssociation::typeAsString(currentType);
    QLabel *pTypeL = new QLabel(i18n("Type:"), nameGB);
    nameLayout->addWidget(pTypeL, 1, 0);

    /* Here is a list of all the supported choices for changing
       association types */
    m_AssocTypes.clear();
    m_AssocTypes <<  Uml::at_Aggregation
        << Uml::at_Composition << Uml::at_Containment;

    bool found=false;
    m_AssocTypeStrings.clear();
    for (uint i=0; i<m_AssocTypes.size(); ++i) {
        if (m_AssocTypes[i] == currentType) found=true;
        QString typeStr = UMLAssociation::typeAsString(m_AssocTypes[i]);
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
    m_pTypeCB->insertStringList(m_AssocTypeStrings);
    m_pTypeCB->setCompletedItems(m_AssocTypeStrings);
    m_pTypeCB->setCurrentText(currentTypeAsString);
    m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
    nameLayout->addWidget(m_pTypeCB, 1, 1);


}


void AssocGenPage::updateObject() {

    if (m_pAssociationWidget) {
        int comboBoxItem = m_pTypeCB->currentItem();
        Uml::Association_Type newType = m_AssocTypes[comboBoxItem];
        m_pAssociationWidget->setAssocType(newType);
        m_pAssociationWidget->setName(m_pAssocNameLE->text());
        m_pAssociationWidget->setDoc(m_pDoc->text());

    } //end if m_pAssociationWidget
}


#include "assocgenpage.moc"
