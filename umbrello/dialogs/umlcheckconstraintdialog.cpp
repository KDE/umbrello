/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2014                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
 ***************************************************************************/

#include "umlcheckconstraintdialog.h"

#include "uml.h"
#include "umldoc.h"
#include "checkconstraint.h"

// kde includes
#include <KLocalizedString>
#include <ktextedit.h>
#include <klineedit.h>

// qt includes
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

/**
 *  Sets up the UMLCheckConstraintDialog.
 *
 *  @param parent   The parent to the UMLUniqueConstraintDialog.
 *  @param pUniqueConstraint The Unique Constraint to show the properties of.
 */
UMLCheckConstraintDialog::UMLCheckConstraintDialog(QWidget* parent, UMLCheckConstraint* pCheckConstraint)
  : QDialog(parent),
    m_pCheckConstraint(pCheckConstraint),
    m_doc(UMLApp::app()->document())
{
    setWindowTitle(i18n("Check Constraint Properties"));
    setModal(true);
    setupDialog();
}

/**
 *  Standard destructor.
 */
UMLCheckConstraintDialog::~UMLCheckConstraintDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLCheckConstraintDialog::setupDialog()
{
    //main layout contains the name fields, the text field
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(15);
    setLayout(mainLayout);

    // layout to hold the name label and line edit
    QHBoxLayout* nameLayout = new QHBoxLayout();
    mainLayout->addItem(nameLayout);

    // name label
    m_pNameL = new QLabel(i18nc("name label", "Name"));
    nameLayout->addWidget(m_pNameL);
    // name lineEdit
    m_pNameLE = new KLineEdit(this);
    nameLayout->addWidget(m_pNameLE);

    QVBoxLayout* checkConditionLayout = new QVBoxLayout();
    mainLayout->addItem(checkConditionLayout);

    m_pCheckConditionL = new QLabel(i18n("Check Condition :"));
    checkConditionLayout->addWidget(m_pCheckConditionL);

    m_pCheckConditionTE = new KTextEdit();
    checkConditionLayout->addWidget(m_pCheckConditionTE);

    // set text of text edit
    m_pCheckConditionTE->setText(m_pCheckConstraint->getCheckCondition());

    // set text of label
    m_pNameLE->setText(m_pCheckConstraint->name());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
}

/**
 * Used when the OK button is clicked. Calls apply()
 */
void UMLCheckConstraintDialog::slotOk()
{
    if (apply()) {
        accept();
    }
}

/**
 * Apply Changes
 */
bool UMLCheckConstraintDialog::apply()
{
    m_pCheckConstraint->setCheckCondition(m_pCheckConditionTE->toPlainText().trimmed());

    // set name
    m_pCheckConstraint->setName(m_pNameLE->text().trimmed());

    return true;
}
