/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "umltemplatedialog.h"

// app includes
#include "template.h"
#include "classifier.h"
#include "documentationwidget.h"
#include "umldoc.h"
#include "uml.h"
#include "dialog_utils.h"
#include "umldatatypewidget.h"
#include "umlstereotypewidget.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
  : SinglePageDialogBase(pParent)
  , ui(new Ui::UMLTemplateDialog)
{
    m_pTemplate = pTemplate;
    setCaption(i18n("Template Properties"));
    ui->setupUi(mainWidget());
    setupDialog();
}

UMLTemplateDialog::~UMLTemplateDialog()
{
}

/**
 *   Sets up the dialog
 */
void UMLTemplateDialog::setupDialog()
{
    ui->dataTypeWidget->setTemplate(m_pTemplate);
    ui->stereotypeWidget->setUMLObject(m_pTemplate);
    ui->documentationWidget->setUMLObject(m_pTemplate);
    ui->nameLE->setFocus();
    ui->nameLE->setText(m_pTemplate->name());
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLTemplateDialog::apply()
{
    ui->dataTypeWidget->apply();

    QString name = ui->nameLE->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid template name."),
                           i18n("Template Name Invalid"), 0);
        ui->nameLE->setText(m_pTemplate->name());
        return false;
    }

    UMLClassifier * pClass = dynamic_cast<UMLClassifier *>(m_pTemplate->parent());
    if (pClass) {
        UMLObject *o = pClass->findChildObject(name);
        if (o && o != m_pTemplate) {
            KMessageBox::error(this, i18n("The template parameter name you have chosen is already being used in this operation."),
                               i18n("Template Name Not Unique"), 0);
            ui->nameLE->setText(m_pTemplate->name());
            return false;
        }
    }
    m_pTemplate->setName(name);
    ui->stereotypeWidget->apply();
    ui->documentationWidget->apply();

    return true;
}

