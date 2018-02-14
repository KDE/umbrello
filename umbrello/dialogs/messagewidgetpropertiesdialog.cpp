/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2018                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "messagewidgetpropertiesdialog.h"

// local includes
#include "messagewidget.h"
#include "uml.h"

/**
 *  Sets up an Message Widget Properties Dialog.
 *  @param  parent  The parent of the dialog
 *  @param  widget  The Message Widget to display properties of.
 *  @param  pageNum The page to show first.
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

