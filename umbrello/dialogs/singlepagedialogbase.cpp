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

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent)
  : KDialog(parent)
{
    setButtons(Help | Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);
    showButtonSeparator(true);

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

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
