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

// own header
#include "assocgenpage.h"

// qt includes
#include <qlayout.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

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
    m_pTypeLE = 0;
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
    Q3GroupBox *nameGB = new Q3GroupBox(this);
    Q3GroupBox *docGB = new Q3GroupBox(this);
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

    m_pDoc = new Q3MultiLineEdit(docGB);
    docLayout -> addWidget(m_pDoc);
    m_pDoc-> setText(m_pAssociationWidget-> getDoc());

    // Association Type
    QLabel *pTypeL = NULL;
    Dialog_Utils::makeLabeledEditField( nameGB, nameLayout, 1,
                                    pTypeL, i18n("Type:"),
                                    m_pTypeLE,
                                    UMLAssociation::typeAsString(m_pAssociationWidget->getAssocType()) );
    m_pTypeLE->setEnabled(false);

    m_pDoc->setWordWrap(Q3MultiLineEdit::WidgetWidth);

}


void AssocGenPage::updateObject() {

    if(m_pAssociationWidget) {
        QString name = m_pAssocNameLE -> text();

        m_pAssociationWidget->setName(m_pAssocNameLE->text());
        m_pAssociationWidget->setDoc(m_pDoc->text());

    } //end if m_pAssociationWidget
}


#include "assocgenpage.moc"
