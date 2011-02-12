/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra <luis@delaparra.org>                                 *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenstatuspage.h"

// app includes
#include "codegenerationwizard.h"  //:TODO: circular reference
#include "codegenerator.h"
#include "codegenerationpolicypage.h"
#include "codegenfactory.h"
#include "codegenpolicyext.h"
#include "debug_utils.h"
#include "defaultcodegenpolicypage.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"

//kde includes
#include <knuminput.h>
#include <kfiledialog.h>
#include <klocale.h>

//qt includes
#include <QtGui/QListWidget>

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeGenStatusPage::CodeGenStatusPage(QWidget *parent)
    : QWizardPage(parent), m_generationDone(false)
{
    setTitle(i18n("Status of Code Generation Progress"));
    setSubTitle(i18n("Press the button Generate to start the code generation.\nCheck the success state for every class."));

    setupUi(this);

    ui_tableWidgetStatus->setColumnCount(3);

    connect( ui_pushButtonGenerate, SIGNAL(clicked()),
             this, SLOT(generateCode()) );
}

/**
 * Destructor.
 */
CodeGenStatusPage::~CodeGenStatusPage()
{
}

/**
 * Reimplemented QWizardPage method to initialize page after clicking next button.
 */
void CodeGenStatusPage::initializePage()
{
    ui_tableWidgetStatus->clearContents();
    m_generationDone = false;
    populateStatusList();
}

/**
 * Fills the status list with the selected classes for generation.
 */
void CodeGenStatusPage::populateStatusList()
{
    CodeGenerationWizard* wiz = (CodeGenerationWizard*)wizard();
    QListWidget* classListWidget = wiz->getSelectionListWidget();

    ui_tableWidgetStatus->setRowCount(classListWidget->count());
    for (int index = 0; index < classListWidget->count(); ++index) {
        QListWidgetItem* item = classListWidget->item(index);
        ui_tableWidgetStatus->setItem(index, 0, new QTableWidgetItem(item->text()));
        ui_tableWidgetStatus->setItem(index, 1, new QTableWidgetItem(i18n("Not Yet Generated")));
        LedStatus* led = new LedStatus(70, 70);
        ui_tableWidgetStatus->setCellWidget(index, 2, led);
    }

    if (classListWidget->count() > 0) {
        ui_pushButtonGenerate->setEnabled(true);
    }
    else {
        ui_pushButtonGenerate->setEnabled(false);
    }
}

/**
 * Slot for the generate button. Starts the code generation.
 */
void CodeGenStatusPage::generateCode()
{
    ui_pushButtonGenerate->setEnabled(false);
    setCommitPage(true);  //:TODO: disable back and cancel button ?

    CodeGenerator* codeGenerator = UMLApp::app()->generator();
    UMLDoc* doc = UMLApp::app()->document();

    if (codeGenerator) {
        connect( codeGenerator, SIGNAL(codeGenerated(UMLClassifier*, bool)),
                 this, SLOT(classGenerated(UMLClassifier*, bool)) );

        UMLClassifierList cList;

        for (int row = 0; row < ui_tableWidgetStatus->rowCount(); ++row) {
            QTableWidgetItem* item = ui_tableWidgetStatus->item(row, 0);
            UMLClassifier *concept =  doc->findUMLClassifier(item->text());
            cList.append(concept);
        }
        codeGenerator->writeCodeToFile(cList);

        m_generationDone = true;
        setFinalPage(true);
        emit completeChanged();
    }
}

/**
 * Reimplemented QWizardPage method the enable / disable the next button.
 * @return   complete state 
 */
bool CodeGenStatusPage::isComplete() const
{
    return m_generationDone;
}

/**
 * Updates the status of the code generation in the status table.
 * @param concept     the class for which the code was generated
 * @param generated   the status of the generation
 */
void CodeGenStatusPage::classGenerated(UMLClassifier* concept, bool generated)
{
    QList<QTableWidgetItem*> items = ui_tableWidgetStatus->findItems( concept->fullyQualifiedName(), Qt::MatchFixedString );
    if (items.count() > 0) {
        QTableWidgetItem* item = items.at(0);
        if ( !item ) {
            uError() << "Code Generation Status Page::Error finding class in list view!";
        }
        else {
            int row = ui_tableWidgetStatus->row(item);
            QTableWidgetItem* status = ui_tableWidgetStatus->item(row, 1);
            LedStatus* led = (LedStatus*)ui_tableWidgetStatus->cellWidget(row, 2);
            if (generated) {
                status->setText( i18n("Code Generated") );
                led->setOn(true);
            }
            else {
                status->setText( i18n("Not Generated") );
                led->setColor(Qt::red);
                led->setOn(true);
            }
        }
    }
}


#include "codegenstatuspage.moc"
