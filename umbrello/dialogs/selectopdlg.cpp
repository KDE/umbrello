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
#include "selectopdlg.h"

// local includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umlclassifierlistitemlist.h"
#include "umlscene.h"
#include "umlview.h"
#include "dialog_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

bool caseInsensitiveLessThan(const UMLOperation *s1, const UMLOperation *s2)
{
    return s1->name().toLower() < s2->name().toLower();
}

/**
 *  Constructs a SelectOpDlg instance.
 *
 *  @param  parent  The parent to this instance.
 *  @param  c       The concept to get the operations from.
 *  @param  enableAutoIncrement Flag to enable auto increment checkbox
 */
SelectOpDlg::SelectOpDlg(UMLView *parent, UMLClassifier * c, bool enableAutoIncrement)
   : KDialog(parent), m_pView(parent), m_classifier(c)
{
    setCaption(i18n("Select Operation"));
    setButtons(Ok | Cancel);
    setDefaultButton(Yes);
    setModal(true);
    showButtonSeparator(true);

    QFrame *frame = new QFrame(this);
    setMainWidget(frame);

    QVBoxLayout * topLayout = new QVBoxLayout(frame);

    m_pOpGB = new QGroupBox(i18n("Select Operation"), frame);
    topLayout->addWidget(m_pOpGB);

    QGridLayout * mainLayout = new QGridLayout(m_pOpGB);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(fontMetrics().height());

    Dialog_Utils::makeLabeledEditField(m_pOpGB, mainLayout, 0,
                                    m_pSeqL, i18n("Sequence number:"),
                                    m_pSeqLE);

    m_pOpAS = new QCheckBox(i18n("Auto increment:"), m_pOpGB);
    mainLayout->addWidget(m_pOpAS, 0, 2);
    connect(m_pOpAS, SIGNAL(toggled(bool)), this, SLOT(slotAutoIncrementChecked(bool)));
    m_pOpAS->setEnabled(enableAutoIncrement);

    m_pOpRB = new QLabel(i18n("Class operation:"), m_pOpGB);
    mainLayout->addWidget(m_pOpRB, 1, 0);

    m_pOpCB = new KComboBox(m_pOpGB);
    m_pOpCB->setCompletionMode(KGlobalSettings::CompletionPopup);
    m_pOpCB->setDuplicatesEnabled(false); // only allow one of each type in box
    connect(m_pOpCB, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged(int)));
    mainLayout->addWidget(m_pOpCB, 1, 1, 1, 2);

    m_newOperationButton = new QPushButton(i18n("New Operation..."), m_pOpGB);
    connect(m_newOperationButton, SIGNAL(clicked()), this, SLOT(slotNewOperation()));
    mainLayout->addWidget(m_newOperationButton, 1, 3);

    m_pCustomRB = new QLabel(i18n("Custom operation:"), m_pOpGB);
    mainLayout->addWidget(m_pCustomRB, 2, 0);

    m_pOpLE = new KLineEdit(m_pOpGB);
    connect(m_pOpLE, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    mainLayout->addWidget(m_pOpLE, 2, 1, 1, 2);
    setupOperationsList();
    enableButtonOk(false);
}

/**
 *  Standard destructor.
 */
SelectOpDlg::~SelectOpDlg()
{
}

/**
 *  Returns the operation to display.
 *
 *  @return The operation to display.
 */
QString SelectOpDlg::getOpText()
{
    if (m_pOpLE->text().isEmpty())
        return m_pOpCB->currentText();
    else
        return m_pOpLE->text();
}

/**
 * Return whether the user selected a class operation
 * or a custom operation.
 *
 * @return  True if user selected a class operation,
 *          false if user selected a custom operation
 */
bool SelectOpDlg::isClassOp() const
{
    return (m_id == OP);
}

/**
 * Set the custom operation text.
 *
 *  @param op The operation to set as the custom operation.
 */
void SelectOpDlg::setCustomOp(const QString &op)
{
    m_pOpLE->setText(op);
    slotTextChanged(op);
}

/**
 * handle auto increment checkbox click
 */
void SelectOpDlg::slotAutoIncrementChecked(bool state)
{
    if (state && m_pSeqLE->text().isEmpty())
        m_pSeqLE->setText(m_pView->umlScene()->autoIncrementSequenceValue());
}

/**
 * handle new operation button click
 */
void SelectOpDlg::slotNewOperation()
{
    UMLOperation *op = m_classifier->createOperation();
    if (!op)
        return;
    setupOperationsList();
    setClassOp(op->toString(Uml::SignatureType::SigNoVis));
    enableButtonOk(true);
}

/**
 * Handle combox box changes.
 */
void SelectOpDlg::slotIndexChanged(int index)
{
    if (index != -1) {
        m_pOpLE->setText("");
        m_id = OP;
        enableButtonOk(true);
    }
}

/**
 * Handle custom line edit changes.
 */
void SelectOpDlg::slotTextChanged(const QString &text)
{
    if (!text.isEmpty()) {
        m_pOpCB->setCurrentIndex(-1);
        m_id = CUSTOM;
        enableButtonOk(true);
    }
}

/**
 * Set the class operation text.
 *
 *  @param op The operation to set as the class operation.
 * @return false if no such operation exists.
 */
bool SelectOpDlg::setClassOp(const QString &op)
{
    for (int i = 1; i != m_pOpCB->count(); ++i) {
        if (m_pOpCB->itemText(i) == op) {
            m_pOpCB->setCurrentIndex(i);
            slotIndexChanged(i);
            return true;
        }
    }
    return false;
}

/**
 * setup dialog operations list
 */
void SelectOpDlg::setupOperationsList()
{
    m_pOpCB->clear();
    UMLOperationList list = m_classifier->getOpList(true);
    if (list.count() > 0)
        m_pOpCB->insertItem(0, "");
    qSort(list.begin(), list.end(), caseInsensitiveLessThan);
    foreach(UMLOperation * obj, list) {
        QString s = obj->toString(Uml::SignatureType::SigNoVis);
        m_pOpCB->insertItem(list.count(), s);
        m_pOpCB->completionObject()->addItem(s);
    }
    m_nOpCount = m_classifier->operations();
}

/**
 *  Returns the sequence number for the operation.
 *
 *  @return Returns the sequence number for the operation.
 */
QString SelectOpDlg::getSeqNumber()
{
    return m_pSeqLE->text();
}

/**
 * Set the sequence number text.
 *
 *  @param  num     The number to set the sequence to.
 */
void SelectOpDlg::setSeqNumber(const QString &num)
{
    m_pSeqLE->setText(num);
}

/**
 */
void SelectOpDlg::setAutoIncrementSequence(bool state)
{
   m_pOpAS->setChecked(state);
}

/**
 */
bool SelectOpDlg::autoIncrementSequence()
{
   return m_pOpAS->isChecked();
}

#include "selectopdlg.moc"
