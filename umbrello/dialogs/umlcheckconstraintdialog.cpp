/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlcheckconstraintdialog.h"

#include "uml.h"
#include "umldoc.h"
#include "checkconstraint.h"

// kde includes
#include <KLocalizedString>
#include <ktextedit.h>
#include <klineedit.h>

// qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

UMLCheckConstraintDialog::UMLCheckConstraintDialog(QWidget* parent, UMLCheckConstraint* pCheckConstraint)
  : SinglePageDialogBase(parent)
{
    setCaption(i18n("Check Constraint Properties"));

    m_pCheckConstraint = pCheckConstraint;
    setupDialog();
}

UMLCheckConstraintDialog::~UMLCheckConstraintDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLCheckConstraintDialog::setupDialog()
{
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);

    //main layout contains the name fields, the text field
    QVBoxLayout* mainLayout = new QVBoxLayout(frame);
    mainLayout->setSpacing(15);

    // layout to hold the name label and line edit
    QHBoxLayout* nameLayout = new QHBoxLayout();
    mainLayout->addItem(nameLayout);

    // name label
    m_pNameL = new QLabel(i18nc("name label", "Name"), this);
    nameLayout->addWidget(m_pNameL);
    // name lineEdit
    m_pNameLE = new KLineEdit(this);
    nameLayout->addWidget(m_pNameLE);

    QVBoxLayout* checkConditionLayout = new QVBoxLayout();
    mainLayout->addItem(checkConditionLayout);

    m_pCheckConditionL = new QLabel(i18n("Check Condition :"), frame);
    checkConditionLayout->addWidget(m_pCheckConditionL);

    m_pCheckConditionTE = new KTextEdit(frame);
    checkConditionLayout->addWidget(m_pCheckConditionTE);

    // set text of text edit
    m_pCheckConditionTE->setText(m_pCheckConstraint->getCheckCondition());

    // set text of label
    m_pNameLE->setText(m_pCheckConstraint->name());
}

/**
 * Apply Changes
 */
bool UMLCheckConstraintDialog::apply()
{
    m_pCheckConstraint->setCheckCondition(m_pCheckConditionTE->toPlainText().trimmed());

    // set name
    m_pCheckConstraint->setName(m_pNameLE->text().trimmed());

    // propagate changes to tree view
    m_pCheckConstraint->emitModified();
    return true;
}

