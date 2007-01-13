/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "umlentityattributedialog.h"

// qt includes
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>

// kde includes
#include <kcombobox.h>
#include <kcompletion.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kdebug.h>

// app includes
#include "../entityattribute.h"
#include "../classifier.h"
#include "../umldoc.h"
#include "../uml.h"
#include "../codegenerator.h"
#include "../dialog_utils.h"
#include "../object_factory.h"
#include "../umlclassifierlist.h"

UMLEntityAttributeDialog::UMLEntityAttributeDialog( QWidget * pParent, UMLEntityAttribute * pEntityAttribute )
        : KDialogBase( Plain, i18n("Entity Attribute Properties"), Help | Ok | Cancel , Ok, pParent, "_UMLENTITYATTRIBUTEDLG_", true, true) {
    m_pEntityAttribute = pEntityAttribute;
    setupDialog();
}

UMLEntityAttributeDialog::~UMLEntityAttributeDialog() {}

void UMLEntityAttributeDialog::setupDialog() {
    UMLDoc * pDoc = UMLApp::app()->getDocument();
    int margin = fontMetrics().height();

    QVBoxLayout * mainLayout = new QVBoxLayout( plainPage() );

    m_pValuesGB = new QGroupBox(i18n("General Properties"), plainPage() );
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB, 5, 2);
    valuesLayout -> setMargin(margin);
    valuesLayout -> setSpacing(10);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
    valuesLayout -> addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(true, m_pValuesGB);
    valuesLayout -> addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 1,
                                    m_pNameL, i18n("&Name:"),
                                    m_pNameLE, m_pEntityAttribute->getName() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 2,
                                    m_pInitialL, i18n("&Default value:"),
                                    m_pInitialLE, m_pEntityAttribute->getInitialValue() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 3,
                                    m_pStereoTypeL, i18n("Stereotype name:"),
                                    m_pStereoTypeLE, m_pEntityAttribute->getStereotype() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 4,
                                    m_pValuesL, i18n("Length/Values:"),
                                    m_pValuesLE, m_pEntityAttribute->getValues() );

    m_pAutoIncrementCB = new QCheckBox( i18n("&Auto increment"), m_pValuesGB );
    m_pAutoIncrementCB->setChecked( m_pEntityAttribute->getAutoIncrement() );
    valuesLayout->addWidget(m_pAutoIncrementCB, 5, 0);

    m_pNullCB = new QCheckBox( i18n("Allow &null"), m_pValuesGB );
    m_pNullCB->setChecked( m_pEntityAttribute->getNull() );
    valuesLayout->addWidget(m_pNullCB, 6, 0);

    m_pAttributesL = new QLabel(i18n("Attributes:"), m_pValuesGB);
    valuesLayout->addWidget(m_pAttributesL, 7, 0);

    m_pAttributesCB = new KComboBox(true, m_pValuesGB);
    m_pAttributesCB->setCompletionMode( KGlobalSettings::CompletionPopup );
    valuesLayout->addWidget(m_pAttributesCB, 7, 1);
    m_pTypeL->setBuddy(m_pAttributesCB);

    insertAttribute( m_pEntityAttribute->getAttributes() );
    insertAttribute("");
    insertAttribute("binary");
    insertAttribute("unsigned");
    insertAttribute("unsigned zerofill");

    mainLayout -> addWidget(m_pValuesGB);

    m_pScopeBG = new QButtonGroup(i18n("Indexing"), plainPage() );
    QHBoxLayout* scopeLayout = new QHBoxLayout(m_pScopeBG);
    scopeLayout->setMargin(margin);

    m_pNoneRB = new QRadioButton(i18n("&None"), m_pScopeBG);
    scopeLayout->addWidget(m_pNoneRB);

    m_pPublicRB = new QRadioButton(i18n("&Primary"), m_pScopeBG);
    scopeLayout->addWidget(m_pPublicRB);

    m_pPrivateRB = new QRadioButton(i18n("&Index"), m_pScopeBG);
    scopeLayout->addWidget(m_pPrivateRB);

    m_pProtectedRB = new QRadioButton(i18n("&Unique"), m_pScopeBG);
    scopeLayout->addWidget(m_pProtectedRB);

    mainLayout->addWidget(m_pScopeBG);
    Uml::DBIndex_Type scope = m_pEntityAttribute->getIndexType();
    if ( scope == Uml::Primary )
        m_pPublicRB->setChecked( true );
    else if( scope == Uml::Index )
        m_pPrivateRB -> setChecked( true );
    else if( scope == Uml::Unique )
        m_pProtectedRB -> setChecked( true );
    else {
        m_pNoneRB->setChecked(true);
    }

    m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );

    // Add the data types.
    UMLClassifierList dataTypes = pDoc->getDatatypes();
    if (dataTypes.count() == 0) {
        // Switch to SQL as the active language if no datatypes are set.
        UMLApp::app()->setActiveLanguage("SQL");
        pDoc->addDefaultDatatypes();
        kapp->processEvents();
        dataTypes = pDoc->getDatatypes();
    }
    UMLClassifier *dat;
    for (UMLClassifierListIt dit(dataTypes); (dat = dit.current()) != NULL; ++dit) {
        insertType(dat->getName());
    }

    //work out which one to select
    int typeBoxCount = 0;
    bool foundType = false;
    while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
        QString typeBoxString = m_pTypeCB->text(typeBoxCount);
        if ( typeBoxString == m_pEntityAttribute->getTypeName() ) {
            foundType = true;
            m_pTypeCB->setCurrentItem(typeBoxCount);
        } else {
            typeBoxCount++;
        }
    }

    if (!foundType) {
        insertType( m_pEntityAttribute->getTypeName(), 0 );
        m_pTypeCB->setCurrentItem(0);
    }

    m_pNameLE->setFocus();
    connect( m_pNameLE, SIGNAL( textChanged ( const QString & ) ), SLOT( slotNameChanged( const QString & ) ) );
    slotNameChanged(m_pNameLE->text() );
}

void UMLEntityAttributeDialog::slotNameChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

bool UMLEntityAttributeDialog::apply() {
    QString name = m_pNameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid entity attribute name."),
                           i18n("Entity Attribute Name Invalid"), false);
        m_pNameLE->setText( m_pEntityAttribute->getName() );
        return false;
    }
    UMLClassifier * pConcept = dynamic_cast<UMLClassifier *>( m_pEntityAttribute->parent() );
    UMLObject *o = pConcept->findChildObject(name);
    if (o && o != m_pEntityAttribute) {
        KMessageBox::error(this, i18n("The entity attribute name you have chosen is already being used in this operation."),
                           i18n("Entity Attribute Name Not Unique"), false);
        m_pNameLE->setText( m_pEntityAttribute->getName() );
        return false;
    }
    m_pEntityAttribute->setName(name);
    m_pEntityAttribute->setInitialValue( m_pInitialLE->text() );
    m_pEntityAttribute->setStereotype( m_pStereoTypeLE->text() );
    m_pEntityAttribute->setValues( m_pValuesLE->text() );
    m_pEntityAttribute->setAttributes( m_pAttributesCB->currentText() );
    m_pEntityAttribute->setAutoIncrement( m_pAutoIncrementCB->isChecked() );
    m_pEntityAttribute->setNull( m_pNullCB->isChecked() );

    if ( m_pPublicRB->isChecked() ) {
        m_pEntityAttribute->setIndexType(Uml::Primary);
    } else if ( m_pPrivateRB -> isChecked() ) {
        m_pEntityAttribute->setIndexType(Uml::Index);
    } else if ( m_pProtectedRB -> isChecked() ) {
        m_pEntityAttribute->setIndexType(Uml::Unique);
    } else {
        m_pEntityAttribute->setIndexType(Uml::None);
    }

    QString typeName = m_pTypeCB->currentText();
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    UMLClassifierList dataTypes = pDoc->getDatatypes();
    UMLClassifier *dat;
    for (UMLClassifierListIt dit(dataTypes); (dat = dit.current()) != NULL; ++dit) {
        if (typeName == dat->getName()) {
            m_pEntityAttribute->setType(dat);
            return true;
        }
    }
    UMLObject *obj = pDoc->findUMLObject(typeName);
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
    if (classifier == NULL) {
        // If it's obviously a pointer type (C++) then create a datatype.
        // Else we don't know what it is so as a compromise create a class.
        Uml::Object_Type ot = (typeName.contains('*') ? Uml::ot_Datatype
                               : Uml::ot_Class);
        obj = Object_Factory::createUMLObject(ot, typeName);
        if (obj == NULL)
            return false;
        classifier = static_cast<UMLClassifier*>(obj);
    }
    m_pEntityAttribute->setType( classifier );
    return true;
}

void UMLEntityAttributeDialog::slotApply() {
    apply();
}

void UMLEntityAttributeDialog::slotOk() {
    if ( apply() ) {
        accept();
    }
}

void UMLEntityAttributeDialog::insertType( const QString& type, int index ) {
    m_pTypeCB->insertItem( type, index );
    m_pTypeCB->completionObject()->addItem( type );
}

void UMLEntityAttributeDialog::insertAttribute( const QString& type, int index ) {
    m_pAttributesCB->insertItem( type, index );
    m_pAttributesCB->completionObject()->addItem( type );
}


#include "umlentityattributedialog.moc"
