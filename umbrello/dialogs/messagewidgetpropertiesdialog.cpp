/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "messagewidgetpropertiesdialog.h"

// local includes
#include "messagewidget.h"
#include "umlapp.h"

/**
 *  Sets up an Message Widget Properties Dialog.
 *  @param  parent  The parent of the dialog
 *  @param  widget  The Message Widget to display properties of.
 */
MessageWidgetPropertiesDialog::MessageWidgetPropertiesDialog (QWidget *parent, MessageWidget * widget)
  : MultiPageDialogBase(parent),
    m_widget(widget)
{
    setupPages();

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 *  Standard destructor.
 */
MessageWidgetPropertiesDialog::~MessageWidgetPropertiesDialog()
{
}

void MessageWidgetPropertiesDialog::slotOk()
{
    slotApply();
    accept();
}

void MessageWidgetPropertiesDialog::slotApply()
{
    MultiPageDialogBase::apply();
    if (m_widget) {
        applyFontPage(m_widget);
    }
}

void MessageWidgetPropertiesDialog::setupPages()
{
    setupGeneralPage(m_widget);
    setupStylePage(m_widget);
    setupFontPage(m_widget);
}

