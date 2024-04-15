/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "visibilityenumwidget.h"

#include "associationwidget.h"
#include "uml.h"
#include "umlobject.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

VisibilityEnumWidget::VisibilityEnumWidget(UMLObject *o, QWidget *parent)
    : QWidget(parent),
      m_object(o),
      m_widget(0),
      m_role(Uml::RoleType::A)
{
    Q_ASSERT(o);

    m_texts[Uml::Visibility::Public] = i18nc("public visibility", "P&ublic");
    m_texts[Uml::Visibility::Protected] = i18nc("protected visibility", "Pro&tected");
    m_texts[Uml::Visibility::Private] = i18nc("private visibility", "P&rivate");
    m_texts[Uml::Visibility::Implementation] = i18n("Imple&mentation");
    init(i18n("Visibility"));
    m_buttons[m_object->visibility()]->setChecked(true);
}

VisibilityEnumWidget::VisibilityEnumWidget(AssociationWidget *a, Uml::RoleType::Enum role, QWidget *parent)
    : QWidget(parent),
      m_object(0),
      m_widget(a),
      m_role(role)
{
    if (role == Uml::RoleType::A) {
        m_texts[Uml::Visibility::Public] = i18nc("scope for A is public", "Public");
        m_texts[Uml::Visibility::Protected] = i18nc("scope for A is protected", "Protected");
        m_texts[Uml::Visibility::Private] = i18nc("scope for A is private", "Private");
        m_texts[Uml::Visibility::Implementation] = i18nc("scope for A is implementation", "Implementation");
        init(i18n("Role A Visibility"));
    } else {
        m_texts[Uml::Visibility::Public] = i18nc("scope for B is public", "Public");
        m_texts[Uml::Visibility::Protected] = i18nc("scope for B is protected", "Protected");
        m_texts[Uml::Visibility::Private] = i18nc("scope for B is private", "Private");
        m_texts[Uml::Visibility::Implementation] = i18nc("scope for B is implementation", "Implementation");
        init(i18n("Role B Visibility"));
    }
    m_buttons[a->visibility(role)]->setChecked(true);
}

VisibilityEnumWidget::~VisibilityEnumWidget()
{
    // nothing here, parenting makes sure that all objects are destroyed
}

/**
 * Add this widget to a given layout.
 * @param layout The layout to which the widget should be added
 */
void VisibilityEnumWidget::addToLayout(QVBoxLayout *layout)
{
    layout->addWidget(m_box);
}

/**
 * Apply changes to the related UMLObject.
 */
void VisibilityEnumWidget::apply()
{
    for(ButtonMap::const_iterator i = m_buttons.constBegin(); i != m_buttons.constEnd(); ++i) {
        if (i.value()->isChecked()) {
            if (m_object)
                m_object->setVisibility(i.key());
            else
                m_widget->setVisibility(i.key(), m_role);
        }
    }
}

void VisibilityEnumWidget::init(const QString &title)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_box = new QGroupBox(title, this);
    QHBoxLayout* boxlayout = new QHBoxLayout(m_box);
    int margin = fontMetrics().height();
    QList<Uml::Visibility::Enum> orders;
    orders << Uml::Visibility::Public << Uml::Visibility::Protected << Uml::Visibility::Private << Uml::Visibility::Implementation;

    for(QList<Uml::Visibility::Enum>::const_iterator i = orders.constBegin(); i != orders.constEnd(); ++i) {
        Uml::Visibility::Enum key = *i;
        QRadioButton *button = new QRadioButton(m_texts[key], m_box);
        m_buttons[key] = button;
        boxlayout->addWidget(button);
    }
    layout->addWidget(m_box);
    setLayout(layout);
}
