/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//local includes
#include "umlinstanceattributedialog.h"
#include "ui_umlinstanceattributedialog.h"
#include "attribute.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstanceAttributeDialog::UMLInstanceAttributeDialog(QWidget *parent, UMLInstanceAttribute *pInstanceAttr)
    : SinglePageDialogBase(parent),
    ui(new Ui::UMLInstanceAttributeDialog),
    m_pInstanceAttr(pInstanceAttr)
{
    setCaption(i18n("Object Attribute Properties"));
    ui->setupUi(mainWidget());
    ui->nameLE->setReadOnly(true);
    ui->nameLE->setText(pInstanceAttr->getAttribute()->name());
    QString initValue = pInstanceAttr->getValue();
    if (initValue.isEmpty())
        initValue = pInstanceAttr->getAttribute()->getInitialValue();
    ui->valueLE->setText(initValue);
}

UMLInstanceAttributeDialog::~UMLInstanceAttributeDialog()
{
    delete ui;
}

bool UMLInstanceAttributeDialog::apply()
{
    QString value = ui->valueLE->text();
    if (value.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute value."),
                           i18n("Value Invalid"));
        return false;
    }
    m_pInstanceAttr->setValue(value);
    m_pInstanceAttr->emitModified();
    return true;
}

