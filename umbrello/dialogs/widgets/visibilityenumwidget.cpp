/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "visibilityenumwidget.h"

#include "associationwidget.h"
#include "uml.h"
#include "umlobject.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>

VisibilityEnumWidget::VisibilityEnumWidget(QWidget *parent)
    : QWidget(parent)
    , m_widget(0)
    , ui(new Ui::visibilityEnumWidget)
    , m_role(Uml::RoleType::A)
{
    ui->setupUi(this);
    initMaps();
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
    } else {
        m_texts[Uml::Visibility::Public] = i18nc("scope for B is public", "Public");
        m_texts[Uml::Visibility::Protected] = i18nc("scope for B is protected", "Protected");
        m_texts[Uml::Visibility::Private] = i18nc("scope for B is private", "Private");
        m_texts[Uml::Visibility::Implementation] = i18nc("scope for B is implementation", "Implementation");
    }
    m_widgets[a->visibility(role)]->setChecked(true);
}

VisibilityEnumWidget::~VisibilityEnumWidget()
{
    // nothing here, parenting makes sure that all objects are destroyed
}

void VisibilityEnumWidget::setUMLObject(UMLObject *o)
{
    m_object = o;
    switch (m_object->visibility()) {
    case Uml::Visibility::Public:
            ui->rb_public->setChecked(true);
        break;
    case Uml::Visibility::Protected:
        ui->rb_protected->setChecked(true);
        break;
    case Uml::Visibility::Private:
        ui->rb_protected->setChecked(true);
        break;
    case Uml::Visibility::Implementation:
        ui->rb_implementation->setChecked(true);
        break;
    }
}

/**
 * Add this widget to a given layout.
 * @param layout The layout to which the widget should be added
 */
void VisibilityEnumWidget::addToLayout(QVBoxLayout *layout)
{
    layout->addWidget(this);
}

/**
 * Apply changes to the related UMLObject.
 */
void VisibilityEnumWidget::apply()
{
    if(ui->rb_public->isChecked())
        m_object->setVisibility(Uml::Visibility::Public);
    else if(ui->rb_protected->isChecked())
        m_object->setVisibility(Uml::Visibility::Protected);
    else if(ui->rb_private->isChecked())
        m_object->setVisibility(Uml::Visibility::Private);
    else
        m_object->setVisibility(Uml::Visibility::Implementation);
}

void VisibilityEnumWidget::initMaps()
{
    m_texts[Uml::Visibility::Public] = i18nc("public visibility", "P&ublic");
    m_texts[Uml::Visibility::Protected] = i18nc("protected visibility", "Pro&tected");
    m_texts[Uml::Visibility::Private] = i18nc("private visibility", "P&rivate");
    m_texts[Uml::Visibility::Implementation] = i18n("Imple&mentation");

    m_widgets[Uml::Visibility::Public]= ui->rb_public;
    m_widgets[Uml::Visibility::Protected]= ui->rb_protected;
    m_widgets[Uml::Visibility::Private]= ui->rb_private;
    m_widgets[Uml::Visibility::Implementation]= ui->rb_implementation;

}

