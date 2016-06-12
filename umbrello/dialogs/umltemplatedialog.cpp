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
#include <klineedit.h>
#include <kcombobox.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>

UMLTemplateDialog::UMLTemplateDialog(QWidget* pParent, UMLTemplate* pTemplate)
  : SinglePageDialogBase(pParent)
{
    m_pTemplate = pTemplate;
    setCaption(i18n("Template Properties"));
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
    int margin = fontMetrics().height();

    QFrame *frame = new QFrame(this);
    setMainWidget(frame);
    QVBoxLayout* mainLayout = new QVBoxLayout(frame);

    m_pValuesGB = new QGroupBox(i18n("General Properties"), frame);
    QGridLayout* valuesLayout = new QGridLayout(m_pValuesGB);
    valuesLayout->setMargin(margin);
    valuesLayout->setSpacing(10);

    m_datatypeWidget = new UMLDatatypeWidget();
    m_datatypeWidget->setTemplate(m_pTemplate);
    m_datatypeWidget->addToLayout(valuesLayout, 0);

    Dialog_Utils::makeLabeledEditField(valuesLayout, 1,
                                    m_pNameL, i18nc("template name", "&Name:"),
                                    m_pNameLE, m_pTemplate->name());
    m_stereotypeWidget = new UMLStereotypeWidget(m_pTemplate);
    m_stereotypeWidget->addToLayout(valuesLayout, 2);

    mainLayout->addWidget(m_pValuesGB);

    m_docWidget = new DocumentationWidget(m_pTemplate, this);
    mainLayout->addWidget(m_docWidget);

    m_pNameLE->setFocus();
}

/**
 * Checks if changes are valid and applies them if they are,
 * else returns false
 */
bool UMLTemplateDialog::apply()
{
    m_datatypeWidget->apply();

    QString name = m_pNameLE->text();
    if(name.length() == 0) {
        KMessageBox::error(this, i18n("You have entered an invalid template name."),
                           i18n("Template Name Invalid"), 0);
        m_pNameLE->setText(m_pTemplate->name());
        return false;
    }

    UMLClassifier * pClass = dynamic_cast<UMLClassifier *>(m_pTemplate->parent());
    if (pClass) {
        UMLObject *o = pClass->findChildObject(name);
        if (o && o != m_pTemplate) {
            KMessageBox::error(this, i18n("The template parameter name you have chosen is already being used in this operation."),
                               i18n("Template Name Not Unique"), 0);
            m_pNameLE->setText(m_pTemplate->name());
            return false;
        }
    }
    m_pTemplate->setName(name);
    m_stereotypeWidget->apply();
    m_docWidget->apply();

    return true;
}

