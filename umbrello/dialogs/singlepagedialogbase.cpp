/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "singlepagedialogbase.h"

// app include
#include <debug_utils.h>
#include <KLocalizedString>

DEBUG_REGISTER(SinglePageDialogBase)

#if QT_VERSION >= 0x050000
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent, bool withApplyButton, bool withSearchButton)
  : QDialog(parent),
    m_mainWidget(0)
{
    setModal(true);
    QDialogButtonBox::StandardButtons buttons;
    if (withApplyButton)
        buttons = QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel;
    else
        buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;

    m_buttonBox = new QDialogButtonBox(buttons);
    if (withSearchButton)
        m_buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Search"));

    connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotClicked(QAbstractButton*)));
    mainWidget();
}
#else

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent, bool withApplyButton, bool withSearchButton)
  : KDialog(parent)
{
    if (withApplyButton)
        setButtons(Help | Ok | Cancel | Apply);
    else
        setButtons(Help | Ok | Cancel);

    if (withSearchButton)
        setButtonText(Ok, i18n("Search"));

    setDefaultButton(Ok);
    setModal(true);
    showButtonSeparator(true);

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}
#endif

SinglePageDialogBase::~SinglePageDialogBase()
{
}

/**
 * Apply dialog changes to the related object.
 *
 * This method could be overridden in derived dialogs to suppport post dialog applying.
 *
 * @return true apply succeeds
 * @return false apply does not succeed
 */
bool SinglePageDialogBase::apply()
{
    DEBUG() << "no derived apply() method present, called empty base implementation";
    return true;
}

#if QT_VERSION >= 0x050000
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
        m_mainWidget ->layout()->setMargin(0);
    }

    delete layout();
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
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
 * @return The current main widget. Will create a QWidget as the mainWidget
 * if none was set before. This way you can write
 * \code
 *   ui.setupUi(mainWidget());
 * \endcode
 * when using designer.
 */
QWidget *SinglePageDialogBase::mainWidget()
{
    if (!m_mainWidget)
        setMainWidget(new QWidget(this));

    return m_mainWidget;
}
#endif

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

#if QT_VERSION >= 0x050000
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
#endif

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

