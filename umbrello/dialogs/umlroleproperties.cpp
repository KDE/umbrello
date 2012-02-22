/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlroleproperties.h"

// kde includes
#include <klocale.h>

UMLRoleProperties::UMLRoleProperties ( QWidget *parent, UMLRole *role)
        : UMLRolePropertiesBase (parent)
{
    m_pRole = role;
    constructWidget();
}

UMLRoleProperties::~UMLRoleProperties()
{
}

void UMLRoleProperties::constructWidget()
{
    // Use Parent Role to set starting Properties

    // Rolename
    ui_pRoleLE->setText(m_pRole->name());
    // Multiplicity
    ui_pMultiLE->setText(m_pRole->multiplicity());
    // Visibility
    switch (m_pRole->visibility()) {
    case Uml::Visibility::Public:
        ui_pPublicRB->setChecked( true );
        break;
    case Uml::Visibility::Private:
        ui_pPrivateRB->setChecked( true );
        break;
    case Uml::Visibility::Protected:
        ui_pProtectedRB->setChecked( true );
        break;
    case Uml::Visibility::Implementation:
        ui_pImplementationRB->setChecked( true );
        break;
    default:
        break;
    }
    // Changeability
    switch (m_pRole->changeability()) {
    case Uml::Changeability::Changeable:
        ui_pChangeableRB->setChecked( true );
        break;
    case Uml::Changeability::Frozen:
        ui_pFrozenRB->setChecked( true );
        break;
    default:
        ui_pAddOnlyRB->setChecked( true );
        break;
    }

    // Documentation
    ui_pDocTE->setText(m_pRole->doc());
    //ui_pDocTE->setWordWrap(QMultiLineEdit::WidgetWidth);
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void UMLRoleProperties::updateObject()
{
    if (m_pRole) {

        // block signals to save work load. we only need to emit modified once,
        // not each time we update an attribute of the association. I suppose
        // we could check to see IF anything changed, but thats a lot more code,
        // and not much gained. This way is easier, if less 'beautiful'. -b.t.

        m_pRole->blockSignals(true);

        // set props
        m_pRole->setName(ui_pRoleLE->text());
        m_pRole->setMultiplicity(ui_pMultiLE->text());

        if (ui_pPrivateRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Private);
        else if (ui_pProtectedRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Protected);
        else if (ui_pPublicRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Public);
        else if (ui_pImplementationRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Implementation);

        if (ui_pFrozenRB->isChecked())
            m_pRole->setChangeability(Uml::Changeability::Frozen);
        else if (ui_pAddOnlyRB->isChecked())
            m_pRole->setChangeability(Uml::Changeability::AddOnly);
        else
            m_pRole->setChangeability(Uml::Changeability::Changeable);

        m_pRole->setDoc(ui_pDocTE->toPlainText());

        m_pRole->blockSignals(false);

        m_pRole->emitModified();

    } //end if m_pRole
}

#include "umlroleproperties.moc"
