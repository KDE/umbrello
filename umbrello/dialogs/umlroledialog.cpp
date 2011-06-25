/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlroledialog.h"

// kde includes
#include <klocale.h>

// app includes
#include "umlrole.h"
#include "umlroleproperties.h"

UMLRoleDialog::UMLRoleDialog( QWidget * parent, UMLRole * pRole )
  : KDialog( parent)
{
    setCaption( i18n("Role Properties") );
    setButtons( Help | Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    m_pRole = pRole;
    setupDialog();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

UMLRoleDialog::~UMLRoleDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLRoleDialog::setupDialog()
{
    // UMLRoleDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "UMLRoleLayout");
    m_pRoleProps = new UMLRoleProperties(this, m_pRole);
    setMainWidget( m_pRoleProps );

    resize( QSize(425, 620).expandedTo(minimumSizeHint()) );

    //  topLayout->addWidget( m_pParmsGB);
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLRoleDialog::apply()
{
    if ( m_pRoleProps ) {
        m_pRoleProps->updateObject();
        return true;
    }
    return false;
}

/**
 * I don't think this is used, but if we had an apply button
 * it would slot into here
 */
void UMLRoleDialog::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked.  Calls apply()
 */
void UMLRoleDialog::slotOk()
{
    if ( apply() ) {
        accept();
    }
}

#include "umlroledialog.moc"
