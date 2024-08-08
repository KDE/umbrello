/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014,2019 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
  : ComboBoxWidgetBase(i18n("Stereotype &name:"), QString(), parent)
  , m_object(object)
{
    Q_ASSERT(m_object);
    insertItems(m_object->umlStereotype());
}

/**
 * Set state if stereotypes could be edited. By default stereotypes could be edited.
 * @param state edit state
 */
void UMLStereotypeWidget::setEditable(bool state)
{
    m_editField->setEditable(state);
}

/**
 * Apply changes to the related UMLObject.
 */
void UMLStereotypeWidget::apply()
{
    if (m_editField->currentText().isEmpty()) {
        m_object->setUMLStereotype(nullptr);
        return;
    }

    QVariant v = m_editField->itemData(m_editField->currentIndex());
    if (v.canConvert<UMLStereotype*>()) {
        UMLStereotype *selected = v.value<UMLStereotype*>();
        if (m_object->umlStereotype()) {
            if (m_object->umlStereotype()->name() != m_editField->currentText())
                m_object->setUMLStereotype(selected);
        }
        else
            m_object->setUMLStereotype(selected);
    } else {
        UMLStereotype *stereotype = new UMLStereotype(m_editField->currentText());
        UMLApp::app()->document()->addStereotype(stereotype);
        m_object->setUMLStereotype(stereotype);
    }
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

    m_editField->clear();
    m_editField->addItem(QStringLiteral(""), QVariant(0));
    foreach(const QString &key, types.keys()) { // krazy:exclude=foreach
        m_editField->addItem(key, QVariant::fromValue((types[key])));
    }

    // select the given parameter
    if (type) {
        int currentIndex = m_editField->findText(type->name());
        if (currentIndex > -1) {
            m_editField->setCurrentIndex(currentIndex);
        }
        m_editField->completionObject()->addItem(type->name());
    }
}
