/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umltemplatedialog.h"

// app includes
#include "template.h"
#include "classifier.h"
#include "umldoc.h"
#include "uml.h"
#include "dialog_utils.h"
#include "umlstereotypewidget.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <KLocalizedString>
#include <kmessagebox.h>

// qt includes
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
  : QDialog(pParent),
    m_pTemplate(pTemplate)
{
    setWindowTitle(i18n("Template Properties"));
    setModal(true);
    setupDialog();
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

    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    m_pValuesGB = new QGroupBox(i18n("General Properties"));
    QGridLayout* valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_pTypeL = new QLabel(i18n("&Type:"), m_pValuesGB);
    valuesLayout->addWidget(m_pTypeL, 0, 0);

    m_pTypeCB = new KComboBox(m_pValuesGB);
    valuesLayout->addWidget(m_pTypeCB, 0, 1);
    m_pTypeL->setBuddy(m_pTypeCB);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 1,
                                    m_pNameL, i18nc("template name", "&Name:"),
                                    m_pNameLE, m_pTemplate->name());
    m_stereotypeWidget = new UMLStereotypeWidget(m_pTemplate);
    m_stereotypeWidget->addToLayout(valuesLayout, 2);

    mainLayout->addWidget(m_pValuesGB);

    m_pTypeCB->setEditable(true);
    m_pTypeCB->setDuplicatesEnabled(false); // only allow one of each type in box
//    m_pTypeCB->setCompleter(...);
    insertTypesSorted(m_pTemplate->getTypeName());

    m_pNameLE->setFocus();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
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
    types << QLatin1String("class");
    // add the active data types to combo box
    UMLDoc *pDoc = UMLApp::app()->document();
    UMLClassifierList namesList(pDoc->concepts());
    foreach (UMLClassifier* obj, namesList) {
        types << obj->name();
    }
    // add the given parameter
    if (!types.contains(type)) {
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
    UMLClassifierList namesList(pDoc->concepts());
    foreach (UMLClassifier* obj, namesList) {
        if (typeName == obj->name()) {
            m_pTemplate->setType(obj);
        }
    }
    if (namesList.isEmpty()) { // not found.
        // FIXME: This implementation is not good yet.
        m_pTemplate->setTypeName(typeName);
    }
    QString name = m_pNameLE->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid template name."),
                           i18n("Template Name Invalid"), 0);
        m_pNameLE->setText(m_pTemplate->name());
        return false;
    }

    UMLClassifier * pClass = dynamic_cast<UMLClassifier *>(m_pTemplate->parent());
    if (pClass) {
        UMLObject *o = pClass->findChildObject(name);
        if (o && o != m_pTemplate) {
            KMessageBox::error(this, i18n("The template parameter name you have chosen is already being used in this operation."),
                               i18n("Template Name Not Unique"), 0);
            m_pNameLE->setText(m_pTemplate->name());
            return false;
        }
    }
    m_pTemplate->setName(name);
    m_stereotypeWidget->apply();

    return true;
}

/**
 * Used when the OK button is clicked. Calls apply()
 */
void UMLTemplateDialog::slotOk()
{
    if (apply()) {
        accept();
    }
}
