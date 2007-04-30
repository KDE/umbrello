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
#include "umlroleproperties.h"

// qt/kde includes
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>


UMLRoleProperties::UMLRoleProperties ( QWidget *parent, UMLRole *role)
        : UMLRolePropertiesBase (parent)
{

    m_pRole = role;
    constructWidget();

}

UMLRoleProperties::~UMLRoleProperties() { }

void UMLRoleProperties::constructWidget() {

    // Use Parent Role to set starting Properties
    //

    // Rolename
    m_pRoleLE -> setText(m_pRole->getName());

    // Multiplicity
    m_pMultiLE -> setText(m_pRole->getMultiplicity());

    // Visibility
    Uml::Visibility scope = m_pRole->getVisibility();
    if( scope == Uml::Visibility::Public )
        m_pPublicRB -> setChecked( true );
    else if( scope == Uml::Visibility::Private )
        m_pPrivateRB -> setChecked( true );
    else if( scope == Uml::Visibility::Protected )
        m_pProtectedRB -> setChecked( true );
    else if( scope == Uml::Visibility::Implementation )
        m_pImplementationRB -> setChecked( true );

    // Changeability
    Uml::Changeability_Type changeability = m_pRole->getChangeability();
    if( changeability == Uml::chg_Changeable )
        m_pChangeableRB -> setChecked( true );
    else if( changeability == Uml::chg_Frozen )
        m_pFrozenRB -> setChecked( true );
    else
        m_pAddOnlyRB -> setChecked( true );

    // Documentation
    //
    m_pDocTE-> setText(m_pRole-> getDoc());
    //m_pDocTE->setWordWrap(QMultiLineEdit::WidgetWidth);
}

void UMLRoleProperties::updateObject() {

    if(m_pRole) {

        // block signals to save work load..we only need to emit modified once,
        // not each time we update an attribute of the association. I suppose
        // we could check to see IF anything changed, but thats a lot more code,
        // and not much gained. THis way is easier, if less 'beautiful'. -b.t.

        m_pRole->blockSignals(true);

        // set props
        m_pRole->setName(m_pRoleLE->text());
        m_pRole->setMultiplicity(m_pMultiLE->text());

        if(m_pPrivateRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Private);
        else if(m_pProtectedRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Protected);
        else if(m_pPublicRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Public);
        else if(m_pImplementationRB->isChecked())
            m_pRole->setVisibility(Uml::Visibility::Implementation);

        if(m_pFrozenRB->isChecked())
            m_pRole->setChangeability(Uml::chg_Frozen);
        else if(m_pAddOnlyRB->isChecked())
            m_pRole->setChangeability(Uml::chg_AddOnly);
        else
            m_pRole->setChangeability(Uml::chg_Changeable);

        m_pRole->setDoc(m_pDocTE->text());

        m_pRole->blockSignals(false);

        m_pRole->emitModified();

    } //end if m_pRole

}


#include "umlroleproperties.moc"
