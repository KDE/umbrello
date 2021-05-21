/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "selectoperationpage.h"

// local includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "documentationwidget.h"
#include "messagewidget.h"
#include "operation.h"
#include "umlclassifierlistitemlist.h"
#include "umlscene.h"
#include "umlview.h"
#include "dialog_utils.h"

// kde includes
#include <klineedit.h>
#include <kcombobox.h>
#include <KLocalizedString>

// qt includes
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

static bool caseInsensitiveLessThan(const UMLOperation *s1, const UMLOperation *s2)
{
    return s1->name().toLower() < s2->name().toLower();
}

/**
 *  Constructs a SelectOperationPage instance.
 *
 *  @param  parent  The parent to this instance.
 *  @param  c       The concept to get the operations from.
 *  @param  enableAutoIncrement Flag to enable auto increment checkbox
 */
SelectOperationPage::SelectOperationPage(UMLView *parent, UMLClassifier *c, LinkWidget *widget, bool enableAutoIncrement)
  : DialogPageBase(parent),
    m_id(CUSTOM),
    m_pView(parent),
    m_classifier(c),
    m_widget(widget)
{
    QVBoxLayout * topLayout = new QVBoxLayout(this);

    m_pOpGB = new QGroupBox(i18n("Select Operation"));
    topLayout->addWidget(m_pOpGB);

    QGridLayout * mainLayout = new QGridLayout(m_pOpGB);

    Dialog_Utils::makeLabeledEditField(mainLayout, 0,
                                    m_pSeqL, i18n("Sequence number:"),
                                    m_pSeqLE);

    m_pOpAS = new QCheckBox(i18n("Auto increment:"), m_pOpGB);
    mainLayout->addWidget(m_pOpAS, 0, 2);
    connect(m_pOpAS, SIGNAL(toggled(bool)), this, SLOT(slotAutoIncrementChecked(bool)));
    m_pOpAS->setEnabled(enableAutoIncrement);

    m_pOpRB = new QLabel(i18n("Class operation:"), m_pOpGB);
    mainLayout->addWidget(m_pOpRB, 1, 0);

    m_pOpCB = new KComboBox(m_pOpGB);
#if QT_VERSION < 0x050000
    m_pOpCB->setCompletionMode(KGlobalSettings::CompletionPopup);
#endif
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
    if (m_widget->operation())
        m_docWidget = new DocumentationWidget(m_widget->operation(), this);
    else {
        MessageWidget *w = dynamic_cast<MessageWidget*>(m_widget);
        if (w)
            m_docWidget = new DocumentationWidget(w, this);
    }
    topLayout->addWidget(m_docWidget);

    setupOperationsList();
    setupDialog();
}

/**
 *  Standard destructor.
 */
SelectOperationPage::~SelectOperationPage()
{
}

/**
 *  Returns the operation to display.
 *
 *  @return The operation to display.
 */
QString SelectOperationPage::getOpText()
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
bool SelectOperationPage::isClassOp() const
{
    return (m_id == OP);
}

/**
 * Set the custom operation text.
 *
 *  @param op The operation to set as the custom operation.
 */
void SelectOperationPage::setCustomOp(const QString &op)
{
    m_pOpLE->setText(op);
    slotTextChanged(op);
}

/**
 * Handle auto increment checkbox click.
 */
void SelectOperationPage::slotAutoIncrementChecked(bool state)
{
    if (state && m_pSeqLE->text().isEmpty())
        m_pSeqLE->setText(m_pView->umlScene()->autoIncrementSequenceValue());
}

/**
 * Handle new operation button click.
 */
void SelectOperationPage::slotNewOperation()
{
    UMLOperation *op = m_classifier->createOperation();
    if (!op)
        return;
    setupOperationsList();
    setClassOp(op->toString(Uml::SignatureType::SigNoVis));
    emit enableButtonOk(true);
}

/**
 * Handle combox box changes.
 */
void SelectOperationPage::slotIndexChanged(int index)
{
    if (index != -1) {
        m_pOpLE->setText(QString());
        m_id = OP;
        emit enableButtonOk(true);
    }
    if (m_pOpCB->currentText().isEmpty()) {
        emit enableButtonOk(false);
    }
}

/**
 * Handle custom line edit changes.
 */
void SelectOperationPage::slotTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emit enableButtonOk(false);
    }
    else {
        m_pOpCB->setCurrentIndex(-1);
        m_id = CUSTOM;
        emit enableButtonOk(true);
    }
}

/**
 * Set the class operation text.
 *
 *  @param op The operation to set as the class operation.
 * @return false if no such operation exists.
 */
bool SelectOperationPage::setClassOp(const QString &op)
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
 * Setup dialog operations list.
 */
void SelectOperationPage::setupOperationsList()
{
    m_pOpCB->clear();
    UMLOperationList list = m_classifier->getOpList(true);
    if (list.count() > 0)
        m_pOpCB->insertItem(0, QString());
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
QString SelectOperationPage::getSeqNumber()
{
    return m_pSeqLE->text();
}

/**
 * Set the sequence number text.
 *
 *  @param  num     The number to set the sequence to.
 */
void SelectOperationPage::setSeqNumber(const QString &num)
{
    m_pSeqLE->setText(num);
}

/**
 * Set the flag for auto increment sequence numbering.
 * @param state   the state of the flag
 */
void SelectOperationPage::setAutoIncrementSequence(bool state)
{
   m_pOpAS->setChecked(state);
}

/**
 * Return the flag for auto increment sequence numbering.
 */
bool SelectOperationPage::autoIncrementSequence()
{
   return m_pOpAS->isChecked();
}

/**
 * internal setup function
 */
void SelectOperationPage::setupDialog()
{
    if (m_enableAutoIncrement && m_pView->umlScene()->autoIncrementSequence()) {
        setAutoIncrementSequence(true);
        setSeqNumber(m_pView->umlScene()->autoIncrementSequenceValue());
   } else
        setSeqNumber(m_widget->sequenceNumber());

    if (m_widget->operation() == 0) {
        setCustomOp(m_widget->lwOperationText());
    } else {
        setClassOp(m_widget->lwOperationText());
    }
    emit enableButtonOk(false);
}

/**
 * apply changes to the related instamces
 * @return true - success
 * @return false - failure
 */
bool SelectOperationPage::apply()
{
    m_docWidget->apply();

    QString opText = getOpText();
    if (isClassOp()) {
        Model_Utils::OpDescriptor od;
        Model_Utils::Parse_Status st = Model_Utils::parseOperation(opText, od, m_classifier);
        if (st == Model_Utils::PS_OK) {
            UMLClassifierList selfAndAncestors = m_classifier->findSuperClassConcepts();
            selfAndAncestors.prepend(m_classifier);
            UMLOperation *op = 0;
            foreach (UMLClassifier *cl, selfAndAncestors) {
                op = cl->findOperation(od.m_name, od.m_args);
                if (op) {
                    break;
                }
            }
            if (!op) {
                // The op does not yet exist. Create a new one.
                UMLObject *o = m_classifier->createOperation(od.m_name, 0, &od.m_args);
                op = o->asUMLOperation();
            }
            if (od.m_pReturnType) {
                op->setType(od.m_pReturnType);
            }

            m_widget->setOperation(op);
            opText.clear();
        } else {
            m_widget->setOperation(0);
        }
    } else {
        m_widget->setOperation(0);
    }
    m_widget->setSequenceNumber(getSeqNumber());
    m_widget->setOperationText(opText);
    if (m_enableAutoIncrement) {
        m_pView->umlScene()->setAutoIncrementSequence(autoIncrementSequence());
    }
    return true;
}
