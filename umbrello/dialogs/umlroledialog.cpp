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

// app includes
#include "debug_utils.h"
#include "umlrole.h"
#include "umlroleproperties.h"

// kde includes
#include <KHelpClient>
#include <KLocalizedString>

/**
 * Constructor.
 */
UMLRoleDialog::UMLRoleDialog(QWidget *parent, UMLRole *pRole)
  : SinglePageDialogBase(parent),
    m_pRole(pRole)
{
    setCaption(i18n("Role Properties"));
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
    QFrame *frame = new QFrame(this);
    setMainWidget(frame);

    QVBoxLayout *topLayout = new QVBoxLayout(frame);

//   UMLRoleDialogLayout = new QGridLayout(this, 1, 1, 11, 6, "UMLRoleLayout");
    m_pRoleProps = new UMLRoleProperties(this, m_pRole);
    topLayout->addWidget(m_pRoleProps);

    resize(QSize(425, 620).expandedTo(minimumSizeHint()));
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false.
 */
bool UMLRoleDialog::apply()
{
    if (m_pRoleProps) {
        m_pRoleProps->apply();
        return true;
    }
    return false;
}

