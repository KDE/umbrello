/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "associationrolepage.h"

// local includes
#include "associationwidget.h"
#include "dialog_utils.h"
#include "objectwidget.h"
#include "umldoc.h"
#include "umlobject.h"
#include "visibilityenumwidget.h"

// kde includes
#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <ktextedit.h>

// qt includes
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

/**
 *  Sets up the AssociationRolePage.
 *  @param  parent  The parent to the AssociationRolePage.
 *  @param  assoc   The AssociationWidget to display the properties of.
 */
AssociationRolePage::AssociationRolePage (QWidget *parent, AssociationWidget *assoc)
  : DialogPageBase(parent),
    m_pRoleALE(0),
    m_pRoleBLE(0),
    m_pMultiACB(0),
    m_pMultiBCB(0),
    m_pAssociationWidget(assoc),
    m_pWidget(0)
{
    constructWidget();
}

/**
 *  Standard destructor.
 */
AssociationRolePage::~AssociationRolePage()
{
}

void AssociationRolePage::constructWidget()
{
    // underlying roles and objects
    QString nameA = m_pAssociationWidget->roleName(Uml::RoleType::A);
    QString nameB = m_pAssociationWidget->roleName(Uml::RoleType::B);
    QString titleA = i18n("Role A Properties");
    QString titleB = i18n("Role B Properties");
    QString widgetNameA = m_pAssociationWidget->widgetForRole(Uml::RoleType::A)->name();
    QString widgetNameB = m_pAssociationWidget->widgetForRole(Uml::RoleType::B)->name();
    if(!widgetNameA.isEmpty())
        titleA.append(QLatin1String(" (") + widgetNameA + QLatin1Char(')'));
    if(!widgetNameB.isEmpty())
        titleB.append(QLatin1String(" (") + widgetNameB + QLatin1Char(')'));

    // general configuration of the GUI
    int margin = fontMetrics().height();

    QGridLayout * mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(6);

    // group boxes for role, documentation properties
    QGroupBox *propsAGB = new QGroupBox(this);
    QGroupBox *propsBGB = new QGroupBox(this);
    QGroupBox *changeABG = new QGroupBox(i18n("Role A Changeability"), this);
    QGroupBox *changeBBG = new QGroupBox(i18n("Role B Changeability"), this);
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
    multiplicities << QString()
                   << QLatin1String("1")
                   << QLatin1String("*")
                   << QLatin1String("1..*")
                   << QLatin1String("0..1");

    // Properties
    //

    // Rolename A
    QLabel *pRoleAL = 0;
    Dialog_Utils::makeLabeledEditField(propsALayout, 0,
                                    pRoleAL, i18n("Rolename:"),
                                    m_pRoleALE, nameA);

    // Multi A
    QLabel *pMultiAL = 0;
    pMultiAL = new QLabel(i18n("Multiplicity:"), propsAGB);
    m_pMultiACB = new KComboBox(propsAGB);
    m_pMultiACB->addItems(multiplicities);
    m_pMultiACB->setDuplicatesEnabled(false);
    m_pMultiACB->setEditable(true);

    QString multiA =  m_pAssociationWidget->multiplicity(Uml::RoleType::A);
    if (!multiA.isEmpty())
        m_pMultiACB->setEditText(multiA);

    propsALayout->addWidget(pMultiAL, 1, 0);
    propsALayout->addWidget(m_pMultiACB, 1, 1);

    m_visibilityWidgetA = new VisibilityEnumWidget(m_pAssociationWidget, Uml::RoleType::A, this);
    mainLayout->addWidget(m_visibilityWidgetA, 1, 0);

    // Changeability A
    QHBoxLayout * changeALayout = new QHBoxLayout(changeABG);
    changeALayout->setMargin(margin);

    m_ChangeableARB = new QRadioButton(i18nc("changeability for A is changeable", "Changeable"), changeABG);
    changeALayout->addWidget(m_ChangeableARB);

    m_FrozenARB = new QRadioButton(i18nc("changeability for A is frozen", "Frozen"), changeABG);
    changeALayout->addWidget(m_FrozenARB);

    m_AddOnlyARB = new QRadioButton(i18nc("changeability for A is add only", "Add only"), changeABG);
    changeALayout->addWidget(m_AddOnlyARB);

    switch (m_pAssociationWidget->changeability(Uml::RoleType::A)) {
    case Uml::Changeability::Changeable:
        m_ChangeableARB->setChecked(true);
        break;
    case Uml::Changeability::Frozen:
        m_FrozenARB->setChecked(true);
        break;
    default:
        m_AddOnlyARB->setChecked(true);
        break;
    }

    // Rolename B
    QLabel * pRoleBL = 0;
    Dialog_Utils::makeLabeledEditField(propsBLayout, 0,
                                    pRoleBL, i18n("Rolename:"),
                                    m_pRoleBLE, nameB);

    // Multi B
    QLabel *pMultiBL = 0;
    pMultiBL = new QLabel(i18n("Multiplicity:"), propsBGB);
    m_pMultiBCB = new KComboBox(propsBGB);
    m_pMultiBCB->addItems(multiplicities);
    m_pMultiBCB->setDuplicatesEnabled(false);
    m_pMultiBCB->setEditable(true);

    QString multiB =  m_pAssociationWidget->multiplicity(Uml::RoleType::B);
    if (!multiB.isEmpty())
        m_pMultiBCB->setEditText(multiB);

    propsBLayout->addWidget(pMultiBL, 1, 0);
    propsBLayout->addWidget(m_pMultiBCB, 1, 1);

    m_visibilityWidgetB = new VisibilityEnumWidget(m_pAssociationWidget, Uml::RoleType::B, this);
    mainLayout->addWidget(m_visibilityWidgetB, 1, 1);

    // Changeability B
    QHBoxLayout * changeBLayout = new QHBoxLayout(changeBBG);
    changeBLayout->setMargin(margin);

    m_ChangeableBRB = new QRadioButton(i18nc("changeability for B is changeable", "Changeable"), changeBBG);
    changeBLayout->addWidget(m_ChangeableBRB);

    m_FrozenBRB = new QRadioButton(i18nc("changeability for B is frozen", "Frozen"), changeBBG);
    changeBLayout->addWidget(m_FrozenBRB);

    m_AddOnlyBRB = new QRadioButton(i18nc("changeability for B is add only", "Add only"), changeBBG);
    changeBLayout->addWidget(m_AddOnlyBRB);

    switch (m_pAssociationWidget->changeability(Uml::RoleType::B)) {
    case Uml::Changeability::Changeable:
        m_ChangeableBRB->setChecked(true);
        break;
    case Uml::Changeability::Frozen:
        m_FrozenBRB->setChecked(true);
        break;
    default:
        m_AddOnlyBRB->setChecked(true);
        break;
    }

    // Documentation
    //

    // Document A
    QHBoxLayout * docALayout = new QHBoxLayout(docAGB);
    docALayout->setMargin(margin);
    m_docA = new KTextEdit(docAGB);
    docALayout->addWidget(m_docA);
    m_docA-> setText(m_pAssociationWidget->roleDocumentation(Uml::RoleType::A));
    // m_docA->setText("<<not implemented yet>>");
    // m_docA->setEnabled(false);
    m_docA->setLineWrapMode(QTextEdit::WidgetWidth);

    // Document B
    QHBoxLayout * docBLayout = new QHBoxLayout(docBGB);
    docBLayout->setMargin(margin);
    m_docB = new KTextEdit(docBGB);
    docBLayout->addWidget(m_docB);
    m_docB->setText(m_pAssociationWidget->roleDocumentation(Uml::RoleType::B));
    // m_docB->setEnabled(false);
    m_docB->setLineWrapMode(QTextEdit::WidgetWidth);

    // add group boxes to main layout
    mainLayout->addWidget(propsAGB, 0, 0);
    mainLayout->addWidget(changeABG, 2, 0);
    mainLayout->addWidget(docAGB, 3, 0);
    mainLayout->addWidget(propsBGB, 0, 1);
    mainLayout->addWidget(changeBBG, 2, 1);
    mainLayout->addWidget(docBGB, 3, 1);
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void AssociationRolePage::apply()
{
    if (m_pAssociationWidget) {

        // set props
        m_pAssociationWidget->setRoleName(m_pRoleALE->text(), Uml::RoleType::A);
        m_pAssociationWidget->setRoleName(m_pRoleBLE->text(), Uml::RoleType::B);
        m_pAssociationWidget->setMultiplicity(m_pMultiACB->currentText(), Uml::RoleType::A);
        m_pAssociationWidget->setMultiplicity(m_pMultiBCB->currentText(), Uml::RoleType::B);

        m_visibilityWidgetA->apply();
        m_visibilityWidgetB->apply();

        if (m_FrozenARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::Frozen, Uml::RoleType::A);
        else if (m_AddOnlyARB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::AddOnly, Uml::RoleType::A);
        else
            m_pAssociationWidget->setChangeability(Uml::Changeability::Changeable, Uml::RoleType::A);

        if (m_FrozenBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::Frozen, Uml::RoleType::B);
        else if (m_AddOnlyBRB->isChecked())
            m_pAssociationWidget->setChangeability(Uml::Changeability::AddOnly, Uml::RoleType::B);
        else
            m_pAssociationWidget->setChangeability(Uml::Changeability::Changeable, Uml::RoleType::B);

        m_pAssociationWidget->setRoleDocumentation(m_docA->toPlainText(), Uml::RoleType::A);
        m_pAssociationWidget->setRoleDocumentation(m_docB->toPlainText(), Uml::RoleType::B);

    } //end if m_pAssociationWidget
}

