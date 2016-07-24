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

#include <KLocalizedString>

Q_DECLARE_METATYPE(UMLStereotype*)

UMLStereotypeWidget::UMLStereotypeWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::UMLStereotypeWidget)
{    
    ui->setupUi(this);
    ui->stereotypeCB->setDuplicatesEnabled(false);
}

UMLStereotypeWidget::~UMLStereotypeWidget()
{
}

void UMLStereotypeWidget::setUMLObject(UMLObject *o)
{
    Q_ASSERT(o);
    m_object = o;
    insertItems(m_object->umlStereotype());
}

/**
 * Set state if stereotypes could be edited. By default stereotypes could be edited.
 * @param state edit state
 */
void UMLStereotypeWidget::setEditable(bool state)
{
    ui->stereotypeCB->setEditable(state);
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void UMLStereotypeWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(this, row, 0);
}

/**
 * Apply changes to the related UMLObject.
 */
void UMLStereotypeWidget::apply()
{
    if (ui->stereotypeCB->currentText().isEmpty()) {
        m_object->setUMLStereotype(0);
        return;
    }

    QVariant v = ui->stereotypeCB->itemData(ui->stereotypeCB->currentIndex());
    if (v.canConvert<UMLStereotype*>()) {
        UMLStereotype *selected = v.value<UMLStereotype*>();
        if (m_object->umlStereotype()) {
            if (m_object->umlStereotype()->name() != ui->stereotypeCB->currentText())
                m_object->setUMLStereotype(selected);
        }
        else
            m_object->setUMLStereotype(selected);
    } else {
        UMLStereotype *stereotype = new UMLStereotype(ui->stereotypeCB->currentText());
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

    ui->stereotypeCB->clear();
    ui->stereotypeCB->addItem(QLatin1String(""), QVariant(0));
    foreach(const QString &key, types.keys()) { // krazy:exclude=foreach
        ui->stereotypeCB->addItem(key, QVariant::fromValue((types[key])));
    }

    // select the given parameter
    if (type) {
        int currentIndex = ui->stereotypeCB->findText(type->name());
        if (currentIndex > -1) {
            ui->stereotypeCB->setCurrentIndex(currentIndex);
        }
        //ui->stereotypeCB->completionObject()->addItem(type->name());
    }
}
