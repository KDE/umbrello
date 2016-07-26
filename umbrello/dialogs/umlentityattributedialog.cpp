/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2014                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

// own header
#include "umlentityattributedialog.h"

// app includes
#include "entityattribute.h"
#include "classifier.h"
#include "umldoc.h"
#include "uml.h"
#include "umldatatypewidget.h"
#include "umlstereotypewidget.h"
#include "codegenerator.h"
#include "dialog_utils.h"
#include "object_factory.h"
#include "umlclassifierlist.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QApplication>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QVBoxLayout>

UMLEntityAttributeDialog::UMLEntityAttributeDialog(QWidget * pParent, UMLEntityAttribute * pEntityAttribute)
    : SinglePageDialogBase(pParent)
    , ui(new Ui::UMLEntityAttributeDialog)
    , m_pEntityAttribute(pEntityAttribute)
{
    setCaption(i18n("Entity Attribute Properties"));
    ui->setupUi(mainWidget());
    setupDialog();
}

UMLEntityAttributeDialog::~UMLEntityAttributeDialog()
{
}

/**
 *   Sets up the dialog.
 */
void UMLEntityAttributeDialog::setupDialog()
{
    ui->dataTypeWidget->setEntityAttribute(m_pEntityAttribute);
    ui->stereotypeWidget->setUMLObject(m_pEntityAttribute);
    ui->autoIncrementCB->setChecked(m_pEntityAttribute->getAutoIncrement());
    ui->allowNullCB->setChecked(m_pEntityAttribute->getNull());

    ui->nameLE->setText(m_pEntityAttribute->name());
    ui->defaultValueLE->setText(m_pEntityAttribute->getInitialValue());
    ui->lenghtValuesLE->setText(m_pEntityAttribute->getValues());

    // enable/disable isNull depending on the state of Auto Increment Check Box
    slotAutoIncrementStateChanged(ui->autoIncrementCB->isChecked());

    insertAttribute(m_pEntityAttribute->getAttributes());
    insertAttribute(QString::fromLatin1("binary"), ui->attributeTypesCB->count());
    insertAttribute(QString::fromLatin1("unsigned"), ui->attributeTypesCB->count());
    insertAttribute(QString::fromLatin1("unsigned zerofill"), ui->attributeTypesCB->count());

    UMLEntityAttribute::DBIndex_Type scope = m_pEntityAttribute->indexType();
    if(scope == UMLEntityAttribute::Index)
        ui->indexedRB->setChecked(true);
    else
        ui->notIndexedRB->setChecked(true);
#if 0
    /*
    m_pPublicRB = new QRadioButton(i18n("&Primary"), m_pScopeGB);
    scopeLayout->addWidget(m_pPublicRB);

    m_pProtectedRB = new QRadioButton(i18n("&Unique"), m_pScopeGB);
    scopeLayout->addWidget(m_pProtectedRB);
    */

    UMLEntityAttribute::DBIndex_Type scope = m_pEntityAttribute->indexType();

    /*
    if (scope == UMLEntityAttribute::Primary)
        m_pPublicRB->setChecked(true);
    else if(scope == UMLEntityAttribute::Unique)
        m_pProtectedRB->setChecked(true);
    else */

    if (scope == UMLEntityAttribute::Index)
        m_pPrivateRB->setChecked(true);
    else {
        m_pNoneRB->setChecked(true);
    }

    m_pNameLE->setFocus();
    connect(m_pNameLE, &KLineEdit::textChanged, this, &UMLEntityAttributeDialog::slotNameChanged);
    connect(m_pAutoIncrementCB, &QCheckBox::clicked, this, &UMLEntityAttributeDialog::slotAutoIncrementStateChanged);
    slotNameChanged(m_pNameLE->text());
#endif
    ui->nameLE->setFocus();
    connect(ui->nameLE, &QLineEdit::textChanged, this, &UMLEntityAttributeDialog::slotNameChanged);
    connect(ui->autoIncrementCB, &QCheckBox::clicked, this, &UMLEntityAttributeDialog::slotAutoIncrementStateChanged);
    slotNameChanged(ui->nameLE->text());
}

void UMLEntityAttributeDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLEntityAttributeDialog::apply()
{
    QString name = ui->nameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid entity attribute name."),
                           i18n("Entity Attribute Name Invalid"), 0);
        ui->nameLE->setText(m_pEntityAttribute->name());
        return false;
    }
    UMLClassifier * pConcept = dynamic_cast<UMLClassifier *>(m_pEntityAttribute->parent());
    UMLObject *o = pConcept->findChildObject(name);
    if (o && o != m_pEntityAttribute) {
        KMessageBox::error(this, i18n("The entity attribute name you have chosen is already being used in this operation."),
                           i18n("Entity Attribute Name Not Unique"), 0);
        ui->nameLE->setText(m_pEntityAttribute->name());
        return false;
    }
    m_pEntityAttribute->setName(name);
    m_pEntityAttribute->setInitialValue(ui->defaultValueLE->text());
    ui->stereotypeWidget->apply();
    m_pEntityAttribute->setValues(ui->lenghtValuesLE->text());
    m_pEntityAttribute->setAttributes(ui->attributeTypesCB->currentText());
    m_pEntityAttribute->setAutoIncrement(ui->autoIncrementCB->isChecked());
    m_pEntityAttribute->setNull(ui->allowNullCB->isChecked());
    //This is correct? - Lays Rodrigues - July/2016
    if(ui->indexedRB->isChecked())
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::Index);
    else
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::None);
#if 0
    /*
    if (m_pPublicRB->isChecked()) {
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::Primary);
    } else if (m_pProtectedRB->isChecked()) {
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::Unique);
    } else
    */

    if (m_pPrivateRB->isChecked()) {
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::Index);
    } else {
        m_pEntityAttribute->setIndexType(UMLEntityAttribute::None);
    }
#endif
    ui->dataTypeWidget->apply();
    return true;
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void UMLEntityAttributeDialog::insertAttribute(const QString& type, int index)
{
    ui->attributeTypesCB->insertItem(index, type);
}

/**
 * Is activated when the auto increment state is changed.
 */
void UMLEntityAttributeDialog::slotAutoIncrementStateChanged(bool checked)
{
    if (checked == true) {
        ui->allowNullCB->setChecked(false);
        ui->allowNullCB->setEnabled(false);
    } else if (checked == false) {
        ui->allowNullCB->setEnabled(true);
    }

}

