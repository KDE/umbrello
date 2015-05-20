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
#include <KLocalizedString>

// app includes
#include "umlrole.h"
#include "umlroleproperties.h"

UMLRoleDialog::UMLRoleDialog(QWidget * parent, UMLRole * pRole)
  : SinglePageDialogBase(parent)
{
    setCaption(i18n("Role Properties"));
    m_pRole = pRole;
    setupDialog();
}

UMLRoleDialog::~UMLRoleDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLRoleDialog::setupDialog()
{
    // UMLRoleDialogLayout = new QGridLayout(this, 1, 1, 11, 6, "UMLRoleLayout");
    m_pRoleProps = new UMLRoleProperties(this, m_pRole);
    setMainWidget(m_pRoleProps);

    resize(QSize(425, 620).expandedTo(minimumSizeHint()));

    //  topLayout->addWidget(m_pParmsGB);
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLRoleDialog::apply()
{
    if (m_pRoleProps) {
        m_pRoleProps->apply();
        return true;
    }
    return false;
}

