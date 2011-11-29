/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlattributedialog.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "template.h"
#include "umldoc.h"
#include "uml.h"
#include "dialog_utils.h"
#include "object_factory.h"
#include "import_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>

UMLAttributeDialog::UMLAttributeDialog( QWidget * pParent, UMLAttribute * pAttribute )
        : KDialog( pParent)
{
    setCaption( i18n("Attribute Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    m_pAttribute = pAttribute;
    setupDialog();
}

UMLAttributeDialog::~UMLAttributeDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLAttributeDialog::setupDialog()
{
    int margin = fontMetrics().height();

    QFrame * frame = new QFrame( this );
    setMainWidget( frame );
    QVBoxLayout * mainLayout = new QVBoxLayout( frame );

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame );
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
    valuesLayout->addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(true, m_pValuesGB);
    valuesLayout->addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 1,
                                    m_pNameL, i18nc("attribute name", "&Name:"),
                                    m_pNameLE, m_pAttribute->name() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 2,
                                    m_pInitialL, i18n("&Initial value:"),
                                    m_pInitialLE, m_pAttribute->getInitialValue() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 3,
                                    m_pStereoTypeL, i18n("Stereotype name:"),
                                    m_pStereoTypeLE, m_pAttribute->stereotype() );

    m_pStaticCB = new QCheckBox( i18n("Classifier &scope (\"static\")"), m_pValuesGB );
    m_pStaticCB->setChecked( m_pAttribute->isStatic() );
    valuesLayout->addWidget(m_pStaticCB, 4, 0);

    mainLayout->addWidget(m_pValuesGB);

    m_pScopeGB = new QGroupBox(i18n("Visibility"), frame );
    QHBoxLayout * scopeLayout = new QHBoxLayout(m_pScopeGB);
    scopeLayout->setMargin(margin);

    m_pPublicRB = new QRadioButton(i18nc("access control public", "&Public"), m_pScopeGB);
    scopeLayout->addWidget(m_pPublicRB);

    m_pPrivateRB = new QRadioButton(i18nc("access control private", "P&rivate"), m_pScopeGB);
    scopeLayout->addWidget(m_pPrivateRB);

    m_pProtectedRB = new QRadioButton(i18nc("access control protected", "Prot&ected"), m_pScopeGB);
    scopeLayout->addWidget(m_pProtectedRB);

    m_pImplementationRB = new QRadioButton(i18n("I&mplementation"), m_pScopeGB);
    scopeLayout->addWidget(m_pImplementationRB);

    mainLayout->addWidget(m_pScopeGB);

    switch (m_pAttribute->visibility()) {
    case Uml::Visibility::Public:
        m_pPublicRB->setChecked( true );
        break;
    case Uml::Visibility::Private:
        m_pPrivateRB->setChecked( true );
        break;
    case Uml::Visibility::Protected:
        m_pProtectedRB->setChecked( true );
        break;
    case Uml::Visibility::Implementation:
        m_pImplementationRB->setChecked( true );
        break;
    default:
        break;
    }

    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );

    //now add the Concepts
    insertTypesSorted(m_pAttribute->getTypeName());

    m_pNameLE->setFocus();
    connect( m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    slotNameChanged(m_pNameLE->text() );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

void UMLAttributeDialog::slotNameChanged( const QString &_text )
{
    enableButtonOk( !_text.isEmpty() );
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLAttributeDialog::apply()
{
    QString name = m_pNameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Attribute Name Invalid"), 0);
        m_pNameLE->setText( m_pAttribute->name() );
        return false;
    }
    UMLClassifier * pConcept = dynamic_cast<UMLClassifier *>( m_pAttribute->parent() );
    UMLObject *o = pConcept->findChildObject(name);
    if (o && o != m_pAttribute) {
        KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                           i18n("Attribute Name Not Unique"), 0);
        m_pNameLE->setText( m_pAttribute->name() );
        return false;
    }
    m_pAttribute->setName(name);
    Uml::Visibility scope = Uml::Visibility::Protected;
    if ( m_pPublicRB->isChecked() ) {
        scope = Uml::Visibility::Public;
    } else if ( m_pPrivateRB->isChecked() ) {
        scope = Uml::Visibility::Private;
    } else if ( m_pImplementationRB->isChecked() ) {
        scope = Uml::Visibility::Implementation;
    }
    m_pAttribute->setVisibility(scope);
    // Set the scope as the default in the option state
    Settings::OptionState optionState = Settings::optionState();
    optionState.classState.defaultAttributeScope = scope;
    Settings::setOptionState(optionState);

    m_pAttribute->setInitialValue( m_pInitialLE->text() );
    m_pAttribute->setStereotype( m_pStereoTypeLE->text() );
    m_pAttribute->setStatic( m_pStaticCB->isChecked() );

    QString typeName = m_pTypeCB->currentText();
    UMLTemplate *tmplParam = pConcept->findTemplate(typeName);
    if (tmplParam) {
        m_pAttribute->setType(tmplParam);
        return true;
    }
    UMLDoc * pDoc = UMLApp::app()->document();
    UMLObject *obj = pDoc->findUMLObject(typeName);
    UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(obj);
    if (classifier == NULL) {
        Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
        if (pl == Uml::ProgrammingLanguage::Cpp || pl == Uml::ProgrammingLanguage::Java) {
            // Import_Utils::createUMLObject works better for C++ namespace
            // and java package than Object_Factory::createUMLObject
            Import_Utils::setRelatedClassifier(pConcept);
            obj = Import_Utils::createUMLObject(UMLObject::ot_UMLObject, typeName);
            Import_Utils::setRelatedClassifier(NULL);
        } else {
            // If it's obviously a pointer type (C++) then create a datatype.
            // Else we don't know what it is so as a compromise create a class.
            UMLObject::ObjectType ot =
                (typeName.contains('*') ? UMLObject::ot_Datatype : UMLObject::ot_Class);
            obj = Object_Factory::createUMLObject(ot, typeName);
        }
        if (obj == NULL)
            return false;
        classifier = static_cast<UMLClassifier*>(obj);
    }
    m_pAttribute->setType(classifier);
    return true;
}

/**
 * I don't think this is used, but if we had an apply button
 * it would slot into here
 */
void UMLAttributeDialog::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked.  Calls apply()
 */
void UMLAttributeDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add
 */
void UMLAttributeDialog::insertTypesSorted( const QString& type )
{
    UMLDoc * uDoc = UMLApp::app()->document();
    UMLClassifierList namesList( uDoc->concepts() );
    QStringList types;
    foreach (UMLClassifier* obj, namesList) {
         types << obj->fullyQualifiedName();
    }
    if ( !types.contains(type) ) {
        types << type;
    }
    types.sort();

    m_pTypeCB->clear();
    m_pTypeCB->insertItems(-1, types);

    int currentIndex = m_pTypeCB->findText(type);
    if (currentIndex > -1) {
        m_pTypeCB->setCurrentIndex(currentIndex);
    }
    m_pTypeCB->completionObject()->addItem(type);
}

#include "umlattributedialog.moc"
