/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlroledialog.h"

// qt/kde  includes
#include <qlayout.h>
#include <kdebug.h>
#include <klocale.h>

// app includes
#include "../umlrole.h"
#include "umlroleproperties.h"

UMLRoleDialog::UMLRoleDialog( QWidget * parent, UMLRole * pRole )
        : KDialogBase( Plain, i18n("Role Properties"), Help | Ok | Cancel , Ok, parent, "_UMLROLEDLG_", true, true)
{
    m_pRole = pRole;
    setupDialog();
}

UMLRoleDialog::~UMLRoleDialog() {}

void UMLRoleDialog::setupDialog() {
    //          UMLRoleDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "UMLRoleLayout");
    m_pRoleProps = new UMLRoleProperties(this, m_pRole);
    setMainWidget( m_pRoleProps );

    resize( QSize(425, 620).expandedTo(minimumSizeHint()) );

    //  topLayout -> addWidget( m_pParmsGB);

}

bool UMLRoleDialog::apply() {
    if( m_pRoleProps ) {
        m_pRoleProps->updateObject();
        return true;
    }
    return false;
}

void UMLRoleDialog::slotApply() {
    apply();
}

void UMLRoleDialog::slotOk() {
    if ( apply() ) {
        accept();
    }
}


#include "umlroledialog.moc"
