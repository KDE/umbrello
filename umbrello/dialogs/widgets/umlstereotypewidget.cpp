/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umlstereotypewidget.h"

#include "basictypes.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"

#include <KComboBox>
#include <KLocalizedString>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

Q_DECLARE_METATYPE(UMLStereotype*);

UMLStereotypeWidget::UMLStereotypeWidget(UMLObject *object, QWidget *parent)
    : QWidget(parent),
      m_object(object)
{
    Q_ASSERT(m_object);
    init();
}

UMLStereotypeWidget::~UMLStereotypeWidget()
{
    delete m_comboBox;
    delete m_label;
}

/**
 * Set state if stereotypes could be edited. By default stereotypes could be edited.
 * @param state edit state
 */
void UMLStereotypeWidget::setEditable(bool state)
{
    m_comboBox->setEditable(state);
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void UMLStereotypeWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_label, row, 0);
    layout->addWidget(m_comboBox, row, 1);
}

/**
 * Apply changes to the related UMLObject.
 */
void UMLStereotypeWidget::apply()
{
    QVariant v = m_comboBox->itemData(m_comboBox->currentIndex());
    if (v.canConvert<UMLStereotype*>()) {
        UMLStereotype *selected = v.value<UMLStereotype*>();
        if (m_object->umlStereotype()) {
            if (m_object->umlStereotype()->name() != m_comboBox->currentText())
                m_object->setUMLStereotype(selected);
        }
        else
            m_object->setUMLStereotype(selected);
    }
    else if (!m_comboBox->currentText().isEmpty()) {
        UMLStereotype *stereotype = new UMLStereotype(m_comboBox->currentText());
        UMLApp::app()->document()->addStereotype(stereotype);
        m_object->setUMLStereotype(stereotype);
    }
    else
        m_object->setUMLStereotype(0);
}

/**
 * setup widgets
 */
void UMLStereotypeWidget::init()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(i18n("&Stereotype name:"), this);
    layout->addWidget(m_label);

    m_comboBox = new KComboBox(true, this);
    layout->addWidget(m_comboBox, 2);

    m_label->setBuddy(m_comboBox);

    m_comboBox->setDuplicatesEnabled(false);  // only allow one of each type in box

    insertItems(m_object->umlStereotype());
    setLayout(layout);
}

/**
 * Insert stereotypes into combo box and select the currently used stereotype.
 * @param type currently used stereotype
 */
void UMLStereotypeWidget::insertItems(UMLStereotype *type)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    QMap<QString, UMLStereotype*> types;

    foreach(UMLStereotype* ust, umldoc->stereotypes()) {
        types[ust->name()] = ust;
    }
    // add the given parameter
    if (type && !types.keys().contains(type->name())) {
        types[type->name()] = type;
    }

    m_comboBox->clear();
    m_comboBox->addItem(QLatin1String(""), QVariant(0));
    foreach(const QString &key, types.keys()) {
        m_comboBox->addItem(key, QVariant::fromValue((types[key])));
    }

    // select the given parameter
    if (type) {
        int currentIndex = m_comboBox->findText(type->name());
        if (currentIndex > -1) {
            m_comboBox->setCurrentIndex(currentIndex);
        }
        m_comboBox->completionObject()->addItem(type->name());
    }
}
