/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "singlepagedialogbase.h"

// app include
#include "debug_utils.h"
#include "umlapp.h"

// kde include
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

DEBUG_REGISTER(SinglePageDialogBase)

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent, bool withApplyButton, bool withSearchButton)
  : QDialog(parent),
    m_mainWidget(nullptr)
{
    setModal(true);
    QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    // buttons |= QDialogButtonBox::Help;
    if (withApplyButton)
        buttons |= QDialogButtonBox::Apply;

    m_buttonBox = new QDialogButtonBox(buttons);
    if (withSearchButton)
        m_buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Search"));

    connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotClicked(QAbstractButton*)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(m_buttonBox);

    if (withSearchButton)
        okButton->setText(i18n("Search"));

    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
    if (withApplyButton)
        connect(m_buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()));
}

SinglePageDialogBase::~SinglePageDialogBase()
{
}

/**
 * Apply dialog changes to the related object.
 *
 * This method could be overridden in derived dialogs to support post dialog applying.
 *
 * @return true apply succeeds
 * @return false apply does not succeed
 */
bool SinglePageDialogBase::apply()
{
    logDebug0("SinglePageDialogBase::apply: no derived apply() method present, "
              "called empty base implementation");
    return true;
}

void SinglePageDialogBase::setCaption(const QString &caption)
{
    setWindowTitle(caption);
}

/**
 * Sets the main widget of the dialog.
 */
void SinglePageDialogBase::setMainWidget(QWidget *widget)
{
    if (widget == m_mainWidget)
        return;
    m_mainWidget = widget;
    if (m_mainWidget && m_mainWidget->layout()) {
        // Avoid double-margin problem
        m_mainWidget ->layout()->setContentsMargins({});
    }

    delete layout();
    if (!m_mainWidget)
        return;
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins({});
    vlayout->addWidget(m_mainWidget);
    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(m_buttonBox);
    hlayout->setContentsMargins (8,0,8,8);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);
}

/**
 * Set the text of a dedicated button.
 * @param code button code
 * @param text button text
 */
void SinglePageDialogBase::setButtonText(SinglePageDialogBase::ButtonCode code, const QString &text)
{
    switch(code) {
    case Ok:
        m_buttonBox->button(QDialogButtonBox::Ok)->setText(text);
        break;
    case Apply:
        m_buttonBox->button(QDialogButtonBox::Apply)->setText(text);
        break;
    case Cancel:
        m_buttonBox->button(QDialogButtonBox::Cancel)->setText(text);
        break;
    }
}

/**
 * @return The main widget.  Will return `this` as the mainWidget
 * if none was set before. This way you can write
 * \code
 *   ui.setupUi(mainWidget());
 * \endcode
 * when using designer.
 */
QWidget *SinglePageDialogBase::mainWidget()
{
    if (m_mainWidget)
        return m_mainWidget;
    return this;
}

/**
 * Used when the Apply button is clicked. Calls apply().
 */
void SinglePageDialogBase::slotApply()
{
    apply();
    done(Apply);
}

/**
 * Used when the OK button is clicked. Calls apply().
 */
void SinglePageDialogBase::slotOk()
{
    if (!validate())
        return;
    if (apply()) {
        done(Ok);
    }
}

/**
 * Used when the Cancel button is clicked.
 */
void SinglePageDialogBase::slotCancel()
{
    done(Cancel);
}

/**
 * Used when the Cancel button is clicked.
 */
void SinglePageDialogBase::slotClicked(QAbstractButton *button)
{
    if (button == m_buttonBox->button(QDialogButtonBox::Apply))
        slotApply();
    else if (button == m_buttonBox->button(QDialogButtonBox::Ok))
        slotOk();
    else if (button == m_buttonBox->button(QDialogButtonBox::Cancel))
        slotCancel();
}

/**
 * Enable the ok button.
 * @param enable   the enable value
 */
void SinglePageDialogBase::enableButtonOk(bool enable)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

/**
 * Return state of dialog input validation.
 *
 * The false state is used to prevent closing the dialog.
 *
 * @return true if dialog entries are valid
 */
bool SinglePageDialogBase::validate()
{
    return true;
}

