/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umlroledialog.h"

// kde includes
#include <klocale.h>

// app includes
#include "umlrole.h"
#include "umlroleproperties.h"

/**
 * Constructor.
 */
UMLRoleDialog::UMLRoleDialog(QWidget *parent, UMLRole *pRole)
  : QDialog(parent),
    m_pRole(pRole)
{
    setWindowTitle(i18n("Role Properties"));
    setModal(true);
    setupDialog();
}

/**
 * Destructor.
 */
UMLRoleDialog::~UMLRoleDialog()
{
}

/**
 * Sets up the dialog.
 */
void UMLRoleDialog::setupDialog()
{
    QVBoxLayout *topLayout = new QVBoxLayout();
    setLayout(topLayout);

//   UMLRoleDialogLayout = new QGridLayout(this, 1, 1, 11, 6, "UMLRoleLayout");
    m_pRoleProps = new UMLRoleProperties(this, m_pRole);
    topLayout->addWidget(m_pRoleProps);

    resize(QSize(425, 620).expandedTo(minimumSizeHint()));

//    topLayout->addWidget(m_pParmsGB);

//FIXME KF5
//    setButtons(Help | Ok | Cancel);
//    setDefaultButton(Ok);
//    showButtonSeparator(true);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLRoleDialog::apply()
{
    if (m_pRoleProps) {
        m_pRoleProps->updateObject();
        return true;
    }
    return false;
}

/**
 * Used when the OK button is clicked. Calls apply().
 */
void UMLRoleDialog::slotOk()
{
    if (apply()) {
        accept();
    }
}
