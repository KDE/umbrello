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
#include "assocrolepage.h"

// local includes
#include "dialog_utils.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>

AssocRolePage::AssocRolePage (UMLDoc *d, QWidget *parent, AssociationWidget *assoc)
        : QWidget(parent)
{
    m_pAssociationWidget = assoc;
    m_pWidget = 0;
    m_pUmldoc = d;

    m_pRoleALE = 0;
    m_pRoleBLE = 0;
    m_pMultiACB = 0;
    m_pMultiBCB = 0;

    constructWidget();
}

AssocRolePage::~AssocRolePage()
{
}

void AssocRolePage::constructWidget()
{
    // underlying roles and objects
    QString nameA = m_pAssociationWidget->roleName(Uml::A);
    QString nameB = m_pAssociationWidget->roleName(Uml::B);
    QString titleA = i18n("Role A Properties");
    QString titleB = i18n("Role B Properties");
    QString widgetNameA = m_pAssociationWidget->widgetForRole(Uml::A)->name();
    QString widgetNameB = m_pAssociationWidget->widgetForRole(Uml::B)->name();
    if(!widgetNameA.isEmpty())
        titleA.append(" (" + widgetNameA + ')');
    if(!widgetNameB.isEmpty())
        titleB.append(" (" + widgetNameB + ')');

    // general configuration of the GUI
    int margin = fontMetrics().height();

    QGridLayout * mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(6);

    // group boxes for role, documentation properties
    QGroupBox *propsAGB = new QGroupBox(this);
    QGroupBox *propsBGB = new QGroupBox(this);
    QGroupBox *scopeABG = new QGroupBox(i18n("Role A Visibility"), this );
    QGroupBox *scopeBBG = new QGroupBox(i18n("Role B Visibility"), this );
    QGroupBox *changeABG = new QGroupBox(i18n("Role A Changeability"), this );
    QGroupBox *changeBBG = new QGroupBox(i18n("Role B Changeability"), this );
    QGroupBox *docAGB = new QGroupBox(this);
    QGroupBox *docBGB = new QGroupBox(this);
    propsAGB->setTitle(titleA);
    propsBGB->setTitle(titleB);
    docAGB->setTitle(i18n("Documentation"));
    docBGB->setTitle(i18n("Documentation"));

    QGridLayout * propsALayout = new QGridLayout(propsAGB);
    propsALayout->setSpacing(6);
    propsALayout->setMargin(margin);

    QGridLayout * propsBLayout = new QGridLayout(propsBGB);
    propsBLayout->setSpacing(6);
    propsBLayout->setMargin(margin);

    QStringList multiplicities;
    multiplicities << "1" << "*" << "1..*" << "0..1";

    // Properties
    //

    // Rolename A
    QLabel *pRoleAL = NULL;
    Dialog_Utils::makeLabeledEditField( propsAGB, propsALayout, 0,
                                    pRoleAL, i18n("Rolename:"),
                                    m_pRoleALE, nameA );

    // Multi A
    QLabel *pMultiAL = NULL;
    pMultiAL = new QLabel( i18n( "Multiplicity:" ), propsAGB );
    m_pMultiACB = new KComboBox(propsAGB);
    m_pMultiACB->addItems( multiplicities );
    m_pMultiACB->setDuplicatesEnabled(false);
    m_pMultiACB->setEditable(true);

    QString multiA =  m_pAssociationWidget->multiplicity(Uml::A );
    if ( !multiA.isEmpty() )
        m_pMultiACB->setEditText(multiA);

    propsALayout->addWidget(pMultiAL, 1, 0 );
    propsALayout->addWidget(m_pMultiACB, 1, 1 );

    // Visibility A
    QHBoxLayout * scopeALayout = new QHBoxLayout(scopeABG);
    scopeALayout->setMargin(margin);

    m_PublicARB = new QRadioButton(i18nc("scope for A is public", "Public"), scopeABG);
    scopeALayout->addWidget(m_PublicARB);

    m_PrivateARB = new QRadioButton(i18nc("scope for A is private", "Private"), scopeABG);
    scopeALayout->addWidget(m_PrivateARB);

    m_ProtectedARB = new QRadioButton(i18nc("scope for A is protected", "Protected"), scopeABG);
    scopeALayout->addWidget(m_ProtectedARB);

    m_ImplementationARB = new QRadioButton(i18nc("scope for A is implementation", "Implementation"), scopeABG);
    scopeALayout->addWidget(m_ImplementationARB);

    switch (m_pAssociationWidget->visibility(Uml::A)) {
    case Uml::Visibility::Public:
        m_PublicARB->setChecked( true );
        break;
    case Uml::Visibility::Private:
        m_PrivateARB->setChecked( true );
        break;
    case Uml::Visibility::Implementation:
        m_PrivateARB->setChecked( true );
        break;
    default:
        m_ProtectedARB->setChecked( true );
        break;
    }

    // Changeability A
    QHBoxLayout * changeALayout = new QHBoxLayout(changeABG);
    changeALayout->setMargin(margin);

    m_ChangeableARB = new QRadioButton(i18nc("changeability for A is changeable", "Changeable"), changeABG);
    changeALayout->addWidget(m_ChangeableARB);

    m_FrozenARB = new QRadioButton(i18nc("changeability for A is frozen", "Frozen"), changeABG);
    changeALayout->addWidget(m_FrozenARB);

    m_AddOnlyARB = new QRadioButton(i18nc("changeability for A is add only", "Add only"), changeABG);
    changeALayout->addWidget(m_AddOnlyARB);

    switch (m_pAssociationWidget->changeability(Uml::A)) {
    case Uml::Changeability::Changeable:
        m_ChangeableARB->setChecked( true );
        break;
    case Uml::Changeability::Frozen:
        m_FrozenARB->setChecked( true );
        break;
    default:
        m_AddOnlyARB->setChecked( true );
        break;
    }

    // Rolename B
    QLabel * pRoleBL = NULL;
    Dialog_Utils::makeLabeledEditField( propsBGB, propsBLayout, 0,
                                    pRoleBL, i18n("Rolename:"),
                                    m_pRoleBLE, nameB );

    // Multi B
    QLabel *pMultiBL = NULL;
    pMultiBL = new QLabel( i18n( "Multiplicity:" ), propsBGB );
    m_pMultiBCB = new KComboBox(propsBGB);
    m_pMultiBCB->addItems( multiplicities );
    m_pMultiBCB->setDuplicatesEnabled(false);
    m_pMultiBCB->setEditable(true);

    QString multiB =  m_pAssociationWidget->multiplicity(Uml::B );
    if ( !multiB.isEmpty() )
        m_pMultiBCB->setEditText(multiB);

    propsBLayout->addWidget(pMultiBL, 1, 0 );
    propsBLayout->addWidget(m_pMultiBCB, 1, 1 );

    // Visibility B
    QHBoxLayout * scopeBLayout = new QHBoxLayout(scopeBBG);
    scopeBLayout->setMargin(margin);

    m_PublicBRB = new QRadioButton(i18nc("scope for B is public", "Public"), scopeBBG);
    scopeBLayout->addWidget(m_PublicBRB);

    m_PrivateBRB = new QRadioButton(i18nc("scope for B is private", "Private"), scopeBBG);
    scopeBLayout->addWidget(m_PrivateBRB);

    m_ProtectedBRB = new QRadioButton(i18nc("scope for B is protected", "Protected"), scopeBBG);
    scopeBLayout->addWidget(m_ProtectedBRB);

    m_ImplementationBRB = new QRadioButton(i18nc("scope for B is implementation", "Implementation"), scopeBBG);
    scopeBLayout->addWidget(m_ImplementationBRB);

    switch (m_pAssociationWidget->visibility(Uml::B)) {
    case Uml::Visibility::Public:
        m_PublicBRB->setChecked( true );
        break;
    case Uml::Visibility::Private:
        m_PrivateBRB->setChecked( true );
        break;
    case Uml::Visibility::Protected:
          m_ProtectedBRB->setChecked( true );
        break;
    default:
        m_ImplementationBRB->setChecked( true );
        break;
    }

    // Changeability B
    QHBoxLayout * changeBLayout = new QHBoxLayout(changeBBG);
    changeBLayout->setMargin(margin);

    m_ChangeableBRB = new QRadioButton(i18nc("changeability for B is changeable", "Changeable"), changeBBG);
    changeBLayout->addWidget(m_ChangeableBRB);

    m_FrozenBRB = new QRadioButton(i18nc("changeability for B is frozen", "Frozen"), changeBBG);
    changeBLayout->addWidget(m_FrozenBRB);

    m_AddOnlyBRB = new QRadioButton(i18nc("changeability for B is add only", "Add only"), changeBBG);
    changeBLayout->addWidget(m_AddOnlyBRB);

    switch (m_pAssociationWidget->changeability(Uml::B)) {
    case Uml::Changeability::Changeable:
        m_ChangeableBRB->setChecked( true );
        break;
    case Uml::Changeability::Frozen:
        m_FrozenBRB->setChecked( true );
        break;
    default:
        m_AddOnlyBRB->setChecked( true );
        break;
    }

    // Documentation
    //

    // Document A
    QHBoxLayout * docALayout = new QHBoxLayout(docAGB);
    docALayout->setMargin(margin);
    m_docA = new KTextEdit(docAGB);
    docALayout->addWidget(m_docA);
    m_docA-> setText(m_pAssociationWidget->roleDocumentation(Uml::A));
    // m_docA->setText("<<not implemented yet>>");
    // m_docA->setEnabled(false);
    m_docA->setLineWrapMode(QTextEdit::WidgetWidth);

    // Document B
    QHBoxLayout * docBLayout = new QHBoxLayout(docBGB);
    docBLayout->setMargin(margin);
    m_docB = new KTextEdit(docBGB);
    docBLayout->addWidget(m_docB);
    m_docB->setText(m_pAssociationWidget->roleDocumentation(Uml::B));
    // m_docB->setEnabled(false);
    m_docB->setLineWrapMode(QTextEdit::WidgetWidth);

    // add group boxes to main layout
    mainLayout->addWidget( propsAGB, 0, 0);
    mainLayout->addWidget( scopeABG, 1, 0);
    mainLayout->addWidget(changeABG, 2, 0);
    mainLayout->addWidget(   docAGB, 3, 0);
    mainLayout->addWidget( propsBGB, 0, 1);
    mainLayout->addWidget( scopeBBG, 1, 1);
    mainLayout->addWidget(changeBBG, 2, 1);
    mainLayout->addWidget(   docBGB, 3, 1);
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void AssocRolePage::updateObject()
{
    if (m_pAssociationWidget) {

        // set props
        m_pAssociationWidget->setRoleName(m_pRoleALE->text(), Uml::A);
        m_pAssociationWidget->setRoleName(m_pRoleBLE->text(), Uml::B);
        m_pAssociationWidget->setMultiplicity(m_pMultiACB->currentText(), Uml::A);
        m_pAssociationWidget->setMultiplicity(m_pMultiBCB->currentText(), Uml::B);

        if (m_PrivateARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Private, Uml::A);
        else if (m_ProtectedARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Protected, Uml::A);
        else if (m_PublicARB->isChecked())
            m_pAssociationWidget->setVisibility(Uml::Visibility::Public, Uml::A);
        else if (m_ImplementationARB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Implementation, Uml::A);

        if (m_PrivateBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Private, Uml::B);
        else if (m_ProtectedBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Protected, Uml::B);
        else if (m_PublicBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Public, Uml::B);
        else if (m_ImplementationBRB->isChecked())
              m_pAssociationWidget->setVisibility(Uml::Visibility::Implementation, Uml::B);

        if (m_FrozenARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::Frozen, Uml::A);
        else if (m_AddOnlyARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::AddOnly, Uml::A);
        else
            m_pAssociationWidget->setChangeability(Uml::Changeability::Changeable, Uml::A);

        if (m_FrozenBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::Frozen, Uml::B);
        else if (m_AddOnlyBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::AddOnly, Uml::B);
        else
            m_pAssociationWidget->setChangeability(Uml::Changeability::Changeable, Uml::B);

        m_pAssociationWidget->setRoleDocumentation(m_docA->toPlainText(), Uml::A);
        m_pAssociationWidget->setRoleDocumentation(m_docB->toPlainText(), Uml::B);

    } //end if m_pAssociationWidget
}

#include "assocrolepage.moc"
