/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umltemplatedialog.h"

// qt includes
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

// kde includes
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// app includes
#include "../template.h"
#include "../classifier.h"
#include "../umldoc.h"
#include "../uml.h"
#include "../dialog_utils.h"

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
        : KDialogBase( Plain, i18n("Template Properties"), Help | Ok | Cancel , Ok, pParent, "_UMLTemplateDLG_", true, true) {
    m_pTemplate = pTemplate;
    setupDialog();
}

UMLTemplateDialog::~UMLTemplateDialog() {}

void UMLTemplateDialog::setupDialog() {
    int margin = fontMetrics().height();

    QVBoxLayout* mainLayout = new QVBoxLayout( plainPage() );

    m_pValuesGB = new QGroupBox(i18n("General Properties"), plainPage() );
    QGridLayout* valuesLayout = new QGridLayout(m_pValuesGB, 3, 2);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
    valuesLayout->addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(m_pValuesGB);
    valuesLayout->addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 1,
                                    m_pNameL, i18n("&Name:"),
                                    m_pNameLE, m_pTemplate->getName() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 2,
                                    m_pStereoTypeL, i18n("&Stereotype name:"),
                                    m_pStereoTypeLE, m_pTemplate->getStereotype() );

    mainLayout->addWidget(m_pValuesGB);

    // "class" is the nominal type of template parameter
    insertType( "class" );
    // Add the active data types to combo box
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    UMLClassifierList namesList( pDoc->getConcepts() );
    UMLClassifier* obj = 0;
    for (obj = namesList.first(); obj; obj = namesList.next()) {
        insertType( obj->getName() );
    }

    m_pTypeCB->setEditable(true);
    m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
//    m_pTypeCB->setAutoCompletion(true);

    //work out which one to select
    int typeBoxCount = 0;
    bool foundType = false;
    while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
        QString typeBoxString = m_pTypeCB->text(typeBoxCount);
        if ( typeBoxString == m_pTemplate->getTypeName() ) {
            foundType = true;
            m_pTypeCB->setCurrentItem(typeBoxCount);
        } else {
            typeBoxCount++;
        }
    }

    if (!foundType) {
        insertType( m_pTemplate->getTypeName(), 0 );
        m_pTypeCB->setCurrentItem(0);
    }

    m_pNameLE->setFocus();
}

void UMLTemplateDialog::insertType( const QString& type, int index )
{
    m_pTypeCB->insertItem( type, index );
    m_pTypeCB->completionObject()->addItem( type );
}

bool UMLTemplateDialog::apply() {
    QString typeName = m_pTypeCB->currentText();
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    UMLClassifierList namesList( pDoc->getConcepts() );
    UMLClassifier* obj = 0;
    for (obj = namesList.first(); obj; obj = namesList.next()) {
        if (typeName == obj->getName()) {
            m_pTemplate->setType( obj );
        }
    }
    if (obj == NULL) { // not found.
        // FIXME: This implementation is not good yet.
        m_pTemplate->setTypeName( typeName );
    }
    QString name = m_pNameLE->text();
    if( name.length() == 0 ) {
        KMessageBox::error(this, i18n("You have entered an invalid template name."),
                           i18n("Template Name Invalid"), false);
        m_pNameLE->setText( m_pTemplate->getName() );
        return false;
    }

    UMLClassifier * pClass = dynamic_cast<UMLClassifier *>( m_pTemplate->parent() );
    if (pClass) {
        UMLObject *o = pClass->findChildObject(name);
        if (o && o != m_pTemplate) {
            KMessageBox::error(this, i18n("The template parameter name you have chosen is already being used in this operation."),
                               i18n("Template Name Not Unique"), false);
            m_pNameLE->setText( m_pTemplate->getName() );
            return false;
        }
    }
    m_pTemplate->setName(name);

    m_pTemplate->setStereotype( m_pStereoTypeLE->text() );

    return true;
}

void UMLTemplateDialog::slotApply() {
    apply();
}

void UMLTemplateDialog::slotOk() {
    if ( apply() ) {
        accept();
    }
}

#include "umltemplatedialog.moc"
