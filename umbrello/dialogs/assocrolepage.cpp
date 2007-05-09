/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "assocrolepage.h"

// qt includes
#include <qlayout.h>

// kde includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// local includes
#include "../dialog_utils.h"

AssocRolePage::AssocRolePage (UMLDoc *d, QWidget *parent, AssociationWidget *assoc)
        : QWidget(parent)
{

    m_pAssociationWidget = assoc;
    m_pWidget = 0;
    m_pUmldoc = d;

    m_pRoleALE = 0;
    m_pRoleBLE = 0;
    m_pMultiALE = 0;
    m_pMultiBLE = 0;

    constructWidget();

}

AssocRolePage::~AssocRolePage() {}

void AssocRolePage::constructWidget() {

    // underlying roles and objects
    QString nameA = m_pAssociationWidget->getRoleName(Uml::A);
    QString nameB = m_pAssociationWidget->getRoleName(Uml::B);
    QString titleA = i18n("Role A Properties");
    QString titleB = i18n("Role B Properties");
    QString widgetNameA = m_pAssociationWidget->getWidget(Uml::A)->getName();
    QString widgetNameB = m_pAssociationWidget->getWidget(Uml::B)->getName();
    if(!widgetNameA.isEmpty())
        titleA.append(" (" + widgetNameA + ')');
    if(!widgetNameB.isEmpty())
        titleB.append(" (" + widgetNameB + ')');

    // general configuration of the GUI
    int margin = fontMetrics().height();
    
    QGridLayout * mainLayout = new QGridLayout(this, 4, 2);
    mainLayout -> setSpacing(6);

    // group boxes for role, documentation properties
    QGroupBox *propsAGB = new QGroupBox(this);
    QGroupBox *propsBGB = new QGroupBox(this);
    QButtonGroup * scopeABG = new QButtonGroup(i18n("Role A Visibility"), this );
    QButtonGroup * scopeBBG = new QButtonGroup(i18n("Role B Visibility"), this );
    QButtonGroup * changeABG = new QButtonGroup(i18n("Role A Changeability"), this );
    QButtonGroup * changeBBG = new QButtonGroup(i18n("Role B Changeability"), this );
    QGroupBox *docAGB = new QGroupBox(this);
    QGroupBox *docBGB = new QGroupBox(this);
    propsAGB -> setTitle(titleA);
    propsBGB -> setTitle(titleB);
    docAGB -> setTitle(i18n("Documentation"));
    docBGB -> setTitle(i18n("Documentation"));
    
    QGridLayout * propsALayout = new QGridLayout(propsAGB, 2, 2);
    propsALayout -> setSpacing(6);
    propsALayout -> setMargin(margin);

    QGridLayout * propsBLayout = new QGridLayout(propsBGB, 3, 2);
    propsBLayout -> setSpacing(6);
    propsBLayout -> setMargin(margin);

    // Properties
    //

    // Rolename A
    QLabel *pRoleAL = NULL;
    Dialog_Utils::makeLabeledEditField( propsAGB, propsALayout, 0,
                                    pRoleAL, i18n("Rolename:"),
                                    m_pRoleALE, nameA );

    // Multi A
    QLabel *pMultiAL = NULL;
    Dialog_Utils::makeLabeledEditField( propsAGB, propsALayout, 1,
                                    pMultiAL, i18n("Multiplicity:"),
                                    m_pMultiALE, m_pAssociationWidget->getMulti(Uml::A) );

    // Visibility A
    QHBoxLayout * scopeALayout = new QHBoxLayout(scopeABG);
    scopeALayout -> setMargin(margin);

    m_PublicARB = new QRadioButton(i18n("Public"), scopeABG);
    scopeALayout -> addWidget(m_PublicARB);

    m_PrivateARB = new QRadioButton(i18n("Private"), scopeABG);
    scopeALayout -> addWidget(m_PrivateARB);

    m_ProtectedARB = new QRadioButton(i18n("Protected"), scopeABG);
    scopeALayout -> addWidget(m_ProtectedARB);

    m_ImplementationARB = new QRadioButton(i18n("Implementation"), scopeABG);
    scopeALayout -> addWidget(m_ImplementationARB);

    Uml::Visibility scope = m_pAssociationWidget->getVisibility(Uml::A);
    if( scope == Uml::Visibility::Public )
        m_PublicARB -> setChecked( true );
    else if( scope == Uml::Visibility::Private )
      m_PrivateARB -> setChecked( true );
    else if( scope == Uml::Visibility::Implementation )
      m_PrivateARB -> setChecked( true );
    else
      m_ProtectedARB -> setChecked( true );

    // Changeability A
    QHBoxLayout * changeALayout = new QHBoxLayout(changeABG);
    changeALayout -> setMargin(margin);

    m_ChangeableARB = new QRadioButton(i18n("Changeable"), changeABG);
    changeALayout -> addWidget(m_ChangeableARB);

    m_FrozenARB = new QRadioButton(i18n("Frozen"), changeABG);
    changeALayout -> addWidget(m_FrozenARB);

    m_AddOnlyARB = new QRadioButton(i18n("Add only"), changeABG);
    changeALayout -> addWidget(m_AddOnlyARB);

    Uml::Changeability_Type changeability = m_pAssociationWidget->getChangeability(Uml::A);
    if( changeability == Uml::chg_Changeable )
        m_ChangeableARB -> setChecked( true );
    else if( changeability == Uml::chg_Frozen )
        m_FrozenARB -> setChecked( true );
    else
        m_AddOnlyARB -> setChecked( true );

    // Rolename B
    QLabel * pRoleBL = NULL;
    Dialog_Utils::makeLabeledEditField( propsBGB, propsBLayout, 0,
                                    pRoleBL, i18n("Rolename:"),
                                    m_pRoleBLE, nameB );

    // Multi B
    QLabel * pMultiBL = NULL;
    Dialog_Utils::makeLabeledEditField( propsBGB, propsBLayout, 1,
                                    pMultiBL, i18n("Multiplicity:"),
                                    m_pMultiBLE, m_pAssociationWidget->getMulti(Uml::B) );

    // Visibility B

    QHBoxLayout * scopeBLayout = new QHBoxLayout(scopeBBG);
    scopeBLayout -> setMargin(margin);

    m_PublicBRB = new QRadioButton(i18n("Public"), scopeBBG);
    scopeBLayout -> addWidget(m_PublicBRB);

    m_PrivateBRB = new QRadioButton(i18n("Private"), scopeBBG);
    scopeBLayout -> addWidget(m_PrivateBRB);

    m_ProtectedBRB = new QRadioButton(i18n("Protected"), scopeBBG);
    scopeBLayout -> addWidget(m_ProtectedBRB);

    m_ImplementationBRB = new QRadioButton(i18n("Implementation"), scopeBBG);
    scopeBLayout -> addWidget(m_ImplementationBRB);

    scope = m_pAssociationWidget->getVisibility(Uml::B);
    if( scope == Uml::Visibility::Public )
        m_PublicBRB -> setChecked( true );
    else if( scope == Uml::Visibility::Private )
        m_PrivateBRB -> setChecked( true );
    else if( scope == Uml::Visibility::Protected )
          m_ProtectedBRB -> setChecked( true );
    else
        m_ImplementationBRB -> setChecked( true );

    // Changeability B
    QHBoxLayout * changeBLayout = new QHBoxLayout(changeBBG);
    changeBLayout -> setMargin(margin);

    m_ChangeableBRB = new QRadioButton(i18n("Changeable"), changeBBG);
    changeBLayout -> addWidget(m_ChangeableBRB);

    m_FrozenBRB = new QRadioButton(i18n("Frozen"), changeBBG);
    changeBLayout -> addWidget(m_FrozenBRB);

    m_AddOnlyBRB = new QRadioButton(i18n("Add only"), changeBBG);
    changeBLayout -> addWidget(m_AddOnlyBRB);

    changeability = m_pAssociationWidget->getChangeability(Uml::B);
    if( changeability == Uml::chg_Changeable )
        m_ChangeableBRB -> setChecked( true );
    else if( changeability == Uml::chg_Frozen )
        m_FrozenBRB -> setChecked( true );
    else
        m_AddOnlyBRB -> setChecked( true );

    // Documentation
    //

    // Document A
    QHBoxLayout * docALayout = new QHBoxLayout(docAGB);
    docALayout -> setMargin(margin);
    m_pDocA = new QMultiLineEdit(docAGB);
    docALayout -> addWidget(m_pDocA);
    m_pDocA-> setText(m_pAssociationWidget-> getRoleDoc(Uml::A));
    // m_pDocA-> setText("<<not implemented yet>>");
    // m_pDocA-> setEnabled(false);
    m_pDocA->setWordWrap(QMultiLineEdit::WidgetWidth);

    // Document B
    QHBoxLayout * docBLayout = new QHBoxLayout(docBGB);
    docBLayout -> setMargin(margin);
    m_pDocB = new QMultiLineEdit(docBGB);
    docBLayout -> addWidget(m_pDocB);
    m_pDocB-> setText(m_pAssociationWidget-> getRoleDoc(Uml::B));
    // m_pDocB-> setEnabled(false);
    m_pDocB->setWordWrap(QMultiLineEdit::WidgetWidth);

    // add group boxes to main layout 
    mainLayout -> addWidget( propsAGB, 0, 0);
    mainLayout -> addWidget( scopeABG, 1, 0);
    mainLayout -> addWidget(changeABG, 2, 0);
    mainLayout -> addWidget(   docAGB, 3, 0);
    mainLayout -> addWidget( propsBGB, 0, 1);
    mainLayout -> addWidget( scopeBBG, 1, 1);
    mainLayout -> addWidget(changeBBG, 2, 1);
    mainLayout -> addWidget(   docBGB, 3, 1);

}

void AssocRolePage::updateObject() {

    if(m_pAssociationWidget) {

        // set props
        m_pAssociationWidget->setRoleName(m_pRoleALE->text(), Uml::A);
        m_pAssociationWidget->setRoleName(m_pRoleBLE->text(), Uml::B);
        m_pAssociationWidget->setMulti(m_pMultiALE->text(), Uml::A);
        m_pAssociationWidget->setMulti(m_pMultiBLE->text(), Uml::B);

        if(m_PrivateARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Private, Uml::A);
        else if(m_ProtectedARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Protected, Uml::A);
        else if(m_PublicARB->isChecked())
            m_pAssociationWidget->setVisibility(Uml::Visibility::Public, Uml::A);
        else if(m_ImplementationARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Implementation, Uml::A);

        if(m_PrivateBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Private, Uml::B);
        else if(m_ProtectedBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Protected, Uml::B);
        else if(m_PublicBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Public, Uml::B);
        else if(m_ImplementationBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Implementation, Uml::B);

        if(m_FrozenARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::chg_Frozen, Uml::A);
        else if(m_AddOnlyARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::chg_AddOnly, Uml::A);
        else
            m_pAssociationWidget->setChangeability(Uml::chg_Changeable, Uml::A);

        if(m_FrozenBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::chg_Frozen, Uml::B);
        else if(m_AddOnlyBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::chg_AddOnly, Uml::B);
        else
            m_pAssociationWidget->setChangeability(Uml::chg_Changeable, Uml::B);

        m_pAssociationWidget->setRoleDoc(m_pDocA->text(), Uml::A);
        m_pAssociationWidget->setRoleDoc(m_pDocB->text(), Uml::B);

    } //end if m_pAssociationWidget

}


#include "assocrolepage.moc"
