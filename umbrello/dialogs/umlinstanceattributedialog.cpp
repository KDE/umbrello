/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

//local includes
#include "umlinstanceattributedialog.h"
#include "ui_umlinstanceattributedialog.h"
#include "classifier.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstanceAttributeDialog::UMLInstanceAttributeDialog(QWidget *parent, UMLAttribute *pAttribute)
    : SinglePageDialogBase(parent),
    ui(new Ui::UMLInstanceAttributeDialog),
    m_pAttribute(pAttribute)
{
    setCaption(i18n("Object Attribute Properties"));
    ui->setupUi(mainWidget());
    connect(ui->nameLE, SIGNAL(textChanged()), this, SLOT(slotNameChanged()));
    connect(ui->valueLE, SIGNAL(textChanged()), this, SLOT(slotNameChanged()));
}

UMLInstanceAttributeDialog::~UMLInstanceAttributeDialog()
{
    delete ui;
}

bool UMLInstanceAttributeDialog::apply()
{
    QString name = ui->nameLE->text();
    QString value = ui->valueLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Class Name Invalid"), 0);
        ui->nameLE->setText(m_pAttribute->name());
        return false;
    } else if(value.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute value."),
                           i18n("Value Invalid"), 0);
        return false;
    }
    UMLClassifier * pConcept = m_pAttribute->umlParent()->asUMLClassifier();
    UMLObject *o = pConcept ? pConcept->findChildObject(name) : 0;
        if (o && o != m_pAttribute) {
            KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                               i18n("Attribute Name Not Unique"), 0);
            ui->nameLE->setText(m_pAttribute->name());
            return false;
        }
        m_pAttribute->setName(name);
        m_pAttribute->setInitialValue(value);
        return true;
}

void UMLInstanceAttributeDialog::slotNameChanged(const QString &text)
{
    enableButtonOk(!text.isEmpty());
}
