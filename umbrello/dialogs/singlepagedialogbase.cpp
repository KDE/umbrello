/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "singlepagedialogbase.h"


#if QT_VERSION >= 0x050000
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent, bool withApplyButton)
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
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainWidget();
}
#else

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent, bool withApplyButton)
  : KDialog(parent)
{
    if (withApplyButton)
        setButtons(Help | Ok | Cancel | Apply);
    else
        setButtons(Help | Ok | Cancel);

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
 */
bool SinglePageDialogBase::apply()
{
    return false;
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
}

/**
 * Used when the OK button is clicked. Calls apply().
 */
void SinglePageDialogBase::slotOk()
{
    if (apply()) {
        accept();
    }
}

#if QT_VERSION >= 0x050000
/**
 * Enable the ok button.
 * @param enable   the enable value
 */
void SinglePageDialogBase::enableButtonOk(bool enable)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

#endif
