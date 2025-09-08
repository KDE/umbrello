/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlentityattributedialog.h"

// app includes
#include "entityattribute.h"
#include "umlclassifier.h"
#include "umldoc.h"
#include "uml.h"
#include "umldatatypewidget.h"
#include "defaultvaluewidget.h"
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
{
    setCaption(i18n("Entity Attribute Properties"));
    m_pEntityAttribute = pEntityAttribute;
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
    int margin = fontMetrics().height();
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout * mainLayout = new QVBoxLayout(frame);

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame);
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setContentsMargins(0, margin, 0, 0);
    valuesLayout->setSpacing(10);

    m_datatypeWidget = new UMLDatatypeWidget(m_pEntityAttribute);
    m_datatypeWidget->addToLayout(valuesLayout, 0);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 1,
                                    m_pNameL, i18nc("name of entity attribute", "&Name:"),
                                    m_pNameLE, m_pEntityAttribute->name());

    m_defaultValueWidget = new DefaultValueWidget(m_pEntityAttribute->getType(), m_pEntityAttribute->getInitialValue(), this);
    m_defaultValueWidget->addToLayout(valuesLayout, 2);
    connect(m_datatypeWidget, SIGNAL(editTextChanged(QString)), m_defaultValueWidget, SLOT(setType(QString)));

    m_stereotypeWidget = new UMLStereotypeWidget(m_pEntityAttribute);
    m_stereotypeWidget->addToLayout(valuesLayout, 3);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 4,
                                    m_pValuesL, i18n("Length/Values:"),
                                    m_pValuesLE, m_pEntityAttribute->getValues());

    m_pAutoIncrementCB = new QCheckBox(i18n("&Auto increment"), m_pValuesGB);
    m_pAutoIncrementCB->setChecked(m_pEntityAttribute->getAutoIncrement());
    valuesLayout->addWidget(m_pAutoIncrementCB, 5, 0);

    m_pNullCB = new QCheckBox(i18n("Allow &null"), m_pValuesGB);
    m_pNullCB->setChecked(m_pEntityAttribute->getNull());
    valuesLayout->addWidget(m_pNullCB, 6, 0);

    // enable/disable isNull depending on the state of Auto Increment Check Box
    slotAutoIncrementStateChanged(m_pAutoIncrementCB->isChecked());

    m_pAttributesL = new QLabel(i18n("Attributes:"), m_pValuesGB);
    valuesLayout->addWidget(m_pAttributesL, 7, 0);

    m_pAttributesCB = new KComboBox(true, m_pValuesGB);
    valuesLayout->addWidget(m_pAttributesCB, 7, 1);
    m_pAttributesL->setBuddy(m_pAttributesCB);

    insertAttribute(m_pEntityAttribute->getAttributes());
    insertAttribute(QString::fromLatin1("binary"), m_pAttributesCB->count());
    insertAttribute(QString::fromLatin1("unsigned"), m_pAttributesCB->count());
    insertAttribute(QString::fromLatin1("unsigned zerofill"), m_pAttributesCB->count());

    mainLayout->addWidget(m_pValuesGB);

    m_pScopeGB = new QGroupBox(i18n("Indexing"), frame);
    QHBoxLayout* scopeLayout = new QHBoxLayout(m_pScopeGB);
    scopeLayout->setContentsMargins(margin, margin, margin, margin);;

    m_pNoneRB = new QRadioButton(i18n("&Not Indexed"), m_pScopeGB);
    scopeLayout->addWidget(m_pNoneRB);

    /*
    m_pPublicRB = new QRadioButton(i18n("&Primary"), m_pScopeGB);
    scopeLayout->addWidget(m_pPublicRB);

    m_pProtectedRB = new QRadioButton(i18n("&Unique"), m_pScopeGB);
    scopeLayout->addWidget(m_pProtectedRB);
    */

    m_pPrivateRB = new QRadioButton(i18n("&Indexed"), m_pScopeGB);
    scopeLayout->addWidget(m_pPrivateRB);

    mainLayout->addWidget(m_pScopeGB);
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
    connect(m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    connect(m_pAutoIncrementCB, SIGNAL(clicked(bool)), this, SLOT(slotAutoIncrementStateChanged(bool)));
    slotNameChanged(m_pNameLE->text());
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
    QString name = m_pNameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid entity attribute name."),
                           i18n("Entity Attribute Name Invalid"), KMessageBox::Options(0));
        m_pNameLE->setText(m_pEntityAttribute->name());
        return false;
    }
    const UMLClassifier * pConcept = m_pEntityAttribute->umlParent()->asUMLClassifier();
    UMLObject *o = pConcept ? pConcept->findChildObject(name) : nullptr;
    if (o && o != m_pEntityAttribute) {
        KMessageBox::error(this, i18n("The entity attribute name you have chosen is already being used in this operation."),
                           i18n("Entity Attribute Name Not Unique"), KMessageBox::Option(0));
        m_pNameLE->setText(m_pEntityAttribute->name());
        return false;
    }
    m_pEntityAttribute->setName(name);
    m_pEntityAttribute->setInitialValue(m_defaultValueWidget->value());
    m_stereotypeWidget->apply();
    m_pEntityAttribute->setValues(m_pValuesLE->text());
    m_pEntityAttribute->setAttributes(m_pAttributesCB->currentText());
    m_pEntityAttribute->setAutoIncrement(m_pAutoIncrementCB->isChecked());
    m_pEntityAttribute->setNull(m_pNullCB->isChecked());

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

    m_datatypeWidget->apply();
    return true;
}

/**
 * Inserts @p type into the type-combobox as well as its completion object.
 */
void UMLEntityAttributeDialog::insertAttribute(const QString& type, int index)
{
    m_pAttributesCB->insertItem(index, type);
    m_pAttributesCB->completionObject()->addItem(type);
}

/**
 * Is activated when the auto increment state is changed.
 */
void UMLEntityAttributeDialog::slotAutoIncrementStateChanged(bool checked)
{
    if (checked == true) {
        m_pNullCB->setChecked(false);
        m_pNullCB->setEnabled(false);
    } else if (checked == false) {
        m_pNullCB->setEnabled(true);
    }
}
