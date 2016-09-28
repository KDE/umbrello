/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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
#include <KLocalizedString>
#include <KMessageBox>

UMLAttributeDialog::UMLAttributeDialog(QWidget *pParent, UMLAttribute *pAttribute)
  : SinglePageDialogBase(pParent)
  , ui(new Ui::UMLAttributeDialog)
  , m_pAttribute(pAttribute)
{
    setCaption(i18n("Attribute Properties"));
    activeLanguage = UMLApp::app()->activeLanguage();
    ui->setupUi(mainWidget());
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
    ui->dataTypeWidget->setClassifierItem(dynamic_cast<UMLClassifierListItem*>(m_pAttribute));
    if(activeLanguage == Uml::ProgrammingLanguage::Cpp){
        ui->typeQualifiersWidget->setUMLClassifierItem(dynamic_cast<UMLClassifierListItem*>(m_pAttribute));
        ui->typeModifierWidget->setUMLClassifierItem(dynamic_cast<UMLClassifierListItem*>(m_pAttribute));
        ui->classifierScopeCB->setVisible(true);
    }else{
        ui->classifierScopeCB->setVisible(false);
        ui->typeModifierWidget->setVisible(false);
        ui->typeQualifiersWidget->setVisible(false);
    }
    ui->nameLE->setText(m_pAttribute->name());
    ui->nameLE->setFocus();
    slotNameChanged(ui->nameLE->text());
    ui->classifierScopeCB->setChecked(m_pAttribute->isStatic());
    ui->initialValueLE->setText(m_pAttribute->getInitialValue());
    connect(ui->nameLE, &QLineEdit::textChanged, this, &UMLAttributeDialog::slotNameChanged);
    ui->stereotypeWidget->setUMLObject(m_pAttribute);
    ui->visibilityWidget->setUMLObject(m_pAttribute);
    ui->documentationWidget->setUMLObject(m_pAttribute);
}

void UMLAttributeDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLAttributeDialog::apply()
{
    QString name = ui->nameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Attribute Name Invalid"), 0);
        ui->nameLE->setText(m_pAttribute->name());
        return false;
    }
    UMLClassifier * pConcept = m_pAttribute->umlParent()->asUMLClassifier();
    UMLObject *o = pConcept ? pConcept->findChildObject(name) : 0;
    if (o && o != m_pAttribute) {
        KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                           i18n("Attribute Name Not Unique"), 0);
        ui->nameLE->setText(m_pAttribute->name());
        return false;
    }
    m_pAttribute->setName(name);
    ui->visibilityWidget->apply();

    // Set the scope as the default in the option state
    Settings::optionState().classState.defaultAttributeScope = m_pAttribute->visibility();

    m_pAttribute->setInitialValue(ui->initialValueLE->text());
    ui->stereotypeWidget->apply();
    m_pAttribute->setStatic(ui->classifierScopeCB->isChecked());
    if(activeLanguage == Uml::ProgrammingLanguage::Cpp){
        ui->typeQualifiersWidget->apply();
        ui->typeModifierWidget->apply();
    }
    ui->dataTypeWidget->apply();
    ui->documentationWidget->apply();

    return true;
}
