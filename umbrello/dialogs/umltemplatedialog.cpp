/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umltemplatedialog.h"

// app includes
#include "template.h"
#include "classifier.h"
#include "umldoc.h"
#include "uml.h"
#include "dialog_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
        : KDialog( pParent)
{
    m_pTemplate = pTemplate;
    setCaption( i18n("Template Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setupDialog();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

UMLTemplateDialog::~UMLTemplateDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLTemplateDialog::setupDialog()
{
    int margin = fontMetrics().height();

    QFrame *frame = new QFrame( this );
    setMainWidget( frame );
    QVBoxLayout* mainLayout = new QVBoxLayout( frame );

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame );
    QGridLayout* valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
    valuesLayout->addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(m_pValuesGB);
    valuesLayout->addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 1,
                                    m_pNameL, i18nc("template name", "&Name:"),
                                    m_pNameLE, m_pTemplate->name() );

    Dialog_Utils::makeLabeledEditField( m_pValuesGB, valuesLayout, 2,
                                    m_pStereoTypeL, i18n("&Stereotype name:"),
                                    m_pStereoTypeLE, m_pTemplate->stereotype() );

    mainLayout->addWidget(m_pValuesGB);

    m_pTypeCB->setEditable(true);
    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
    m_pTypeCB->setCompletionMode( KGlobalSettings::CompletionPopup );
//    m_pTypeCB->setCompleter(...);
    insertTypesSorted(m_pTemplate->getTypeName());

    m_pNameLE->setFocus();
}

/**
 * Inserts @p type into the type-combobox.
 * The combobox is cleared and all types together with the optional new one
 * sorted and then added again.
 * @param type   a new type to add and selected
 */
void UMLTemplateDialog::insertTypesSorted(const QString& type)
{
    QStringList types;
    // "class" is the nominal type of template parameter
    types << "class";
    // add the active data types to combo box
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList( pDoc->concepts() );
    foreach (UMLClassifier* obj, namesList) {
        types << obj->name();
    }
    // add the given parameter
    if ( !types.contains(type) ) {
        types << type;
    }
    types.sort();

    m_pTypeCB->clear();
    m_pTypeCB->insertItems(-1, types);

    // select the given parameter
    int currentIndex = m_pTypeCB->findText(type);
    if (currentIndex > -1) {
        m_pTypeCB->setCurrentIndex(currentIndex);
    }
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLTemplateDialog::apply()
{
    QString typeName = m_pTypeCB->currentText();
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList( pDoc->concepts() );
    foreach (UMLClassifier* obj, namesList) {
        if (typeName == obj->name()) {
            m_pTemplate->setType(obj);
        }
    }
    if (namesList.isEmpty()) { // not found.
        // FIXME: This implementation is not good yet.
        m_pTemplate->setTypeName( typeName );
    }
    QString name = m_pNameLE->text();
    if( name.length() == 0 ) {
        KMessageBox::error(this, i18n("You have entered an invalid template name."),
                           i18n("Template Name Invalid"), 0);
        m_pNameLE->setText( m_pTemplate->name() );
        return false;
    }

    UMLClassifier * pClass = dynamic_cast<UMLClassifier *>( m_pTemplate->parent() );
    if (pClass) {
        UMLObject *o = pClass->findChildObject(name);
        if (o && o != m_pTemplate) {
            KMessageBox::error(this, i18n("The template parameter name you have chosen is already being used in this operation."),
                               i18n("Template Name Not Unique"), 0);
            m_pNameLE->setText( m_pTemplate->name() );
            return false;
        }
    }
    m_pTemplate->setName(name);

    m_pTemplate->setStereotype( m_pStereoTypeLE->text() );

    return true;
}

/**
 * I don't think this is used, but if we had an apply button
 * it would slot into here
 */
void UMLTemplateDialog::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked.  Calls apply()
 */
void UMLTemplateDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

#include "umltemplatedialog.moc"
