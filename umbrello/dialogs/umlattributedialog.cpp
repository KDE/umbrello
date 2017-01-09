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
#include "documentationwidget.h"
#include "template.h"
#include "umldoc.h"
#include "uml.h"
#include "umldatatypewidget.h"
#include "umlstereotypewidget.h"
#include "visibilityenumwidget.h"
#include "dialog_utils.h"
#include "object_factory.h"
#include "import_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QVBoxLayout>

UMLAttributeDialog::UMLAttributeDialog(QWidget * pParent, UMLAttribute * pAttribute)
  : SinglePageDialogBase(pParent)
{
    setCaption(i18n("Attribute Properties"));
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

    QFrame * frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout * mainLayout = new QVBoxLayout(frame);

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame);
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_datatypeWidget = new UMLDatatypeWidget(m_pAttribute->asUMLClassifierListItem());
    m_datatypeWidget->addToLayout(valuesLayout, 0);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 1,
                                    m_pNameL, i18nc("attribute name", "&Name:"),
                                    m_pNameLE, m_pAttribute->name());

    Dialog_Utils::makeLabeledEditField(valuesLayout, 2,
                                    m_pInitialL, i18n("&Initial value:"),
                                    m_pInitialLE, m_pAttribute->getInitialValue());

    m_stereotypeWidget = new UMLStereotypeWidget(m_pAttribute);
    m_stereotypeWidget->addToLayout(valuesLayout, 3);

    m_pStaticCB = new QCheckBox(i18n("Classifier &scope (\"static\")"), m_pValuesGB);
    m_pStaticCB->setChecked(m_pAttribute->isStatic());
    valuesLayout->addWidget(m_pStaticCB, 4, 0);

    mainLayout->addWidget(m_pValuesGB);
    m_visibilityEnumWidget = new VisibilityEnumWidget(m_pAttribute, this);
    m_visibilityEnumWidget->addToLayout(mainLayout);

    m_docWidget = new DocumentationWidget(m_pAttribute, this);
    mainLayout->addWidget(m_docWidget);

    m_pNameLE->setFocus();
    connect(m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    slotNameChanged(m_pNameLE->text());
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
    QString name = m_pNameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Attribute Name Invalid"), 0);
        m_pNameLE->setText(m_pAttribute->name());
        return false;
    }
    UMLClassifier * pConcept = m_pAttribute->umlParent()->asUMLClassifier();
    UMLObject *o = pConcept ? pConcept->findChildObject(name) : 0;
    if (o && o != m_pAttribute) {
        KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                           i18n("Attribute Name Not Unique"), 0);
        m_pNameLE->setText(m_pAttribute->name());
        return false;
    }
    m_pAttribute->blockSignals(true);
    m_visibilityEnumWidget->apply();
    m_pAttribute->setInitialValue(m_pInitialLE->text());
    m_stereotypeWidget->apply();
    m_pAttribute->setStatic(m_pStaticCB->isChecked());
    m_datatypeWidget->apply();
    m_docWidget->apply();
    m_pAttribute->blockSignals(false);
    // trigger signals
    m_pAttribute->setName(name);

    // Set the scope as the default in the option state
    Settings::optionState().classState.defaultAttributeScope = m_pAttribute->visibility();
    return true;
}
