/*
    SPDX-License-Identifier: GPL-3.0-or-later

    SPDX-FileCopyrightText: 2015 Tzvetelin Katchov <katchov@gmail.com>
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlenumliteraldialog.h"

// app includes
#include "enumliteral.h"
#include "classifier.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "uml.h"  // Only needed for log{Warn,Error}

// kde includes
#include <klineedit.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

UMLEnumLiteralDialog::UMLEnumLiteralDialog(QWidget * pParent, UMLEnumLiteral * pEnumLiteral)
  : SinglePageDialogBase(pParent)
{
    setCaption(i18n("EnumLiteral Properties"));
    m_pEnumLiteral = pEnumLiteral;
    setupDialog();
}

UMLEnumLiteralDialog::~UMLEnumLiteralDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLEnumLiteralDialog::setupDialog()
{
    int margin = fontMetrics().height();

    QFrame * frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout * mainLayout = new QVBoxLayout(frame);

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame);
    QGridLayout * valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setSpacing(10);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 0,
                                    m_pNameL, i18nc("literal name", "&Name:"),
                                    m_pNameLE, m_pEnumLiteral->name());

    Dialog_Utils::makeLabeledEditField(valuesLayout, 1,
                                    m_pValueL, i18n("&Value:"),
                                    m_pValueLE, m_pEnumLiteral->value());

    mainLayout->addWidget(m_pValuesGB);

    m_pNameLE->setFocus();
    connect(m_pNameLE, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)));
    slotNameChanged(m_pNameLE->text());
}

void UMLEnumLiteralDialog::slotNameChanged(const QString &_text)
{
    enableButtonOk(!_text.isEmpty());
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLEnumLiteralDialog::apply()
{
    QString name = m_pNameLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Attribute Name Invalid"), 0);
        m_pNameLE->setText(m_pEnumLiteral->name());
        return false;
    }
    const UMLClassifier * pConcept = m_pEnumLiteral->umlParent()->asUMLClassifier();
    if (!pConcept) {
        logError1("UMLEnumLiteralDialog::apply: Could not get parent of enum literal '%1'",
                  m_pEnumLiteral->name());
        return false;
    }
    UMLObject *o = pConcept->findChildObject(name);
    if (o && o != m_pEnumLiteral) {
        KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                           i18n("Attribute Name Not Unique"), 0);
        m_pNameLE->setText(m_pEnumLiteral->name());
        return false;
    }
    m_pEnumLiteral->setName(name);

    m_pEnumLiteral->setValue(m_pValueLE->text());

    return true;
}
