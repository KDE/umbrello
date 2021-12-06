/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Luis De la Parra <luis@delaparra.org>
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#if QT_VERSION < 0x050000
#include <kfiledialog.h>
#include <KIntSpinBox>
#endif
#include <KLocalizedString>
#include <KMessageBox>

//qt includes
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#endif
#include <QListWidget>

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeGenStatusPage::CodeGenStatusPage(QWidget *parent)
  : QWizardPage(parent),
    m_generationDone(false)
{
    setTitle(i18n("Status of Code Generation Progress"));
    setSubTitle(i18n("Press the button Generate to start the code generation.\nCheck the success state for every class."));

    setupUi(this);

    ui_tableWidgetStatus->setColumnCount(3);
    ui_textEditLogger->setReadOnly(true);

    connect(ui_pushButtonGenerate, SIGNAL(clicked()), this, SLOT(generateCode()));
    connect(ui_pushButtonClear, SIGNAL(clicked()), this, SLOT(loggerClear()));
    connect(ui_pushButtonExport, SIGNAL(clicked()), this, SLOT(loggerExport()));
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
        connect(codeGenerator, SIGNAL(codeGenerated(UMLClassifier*,bool)),
                this, SLOT(classGenerated(UMLClassifier*,bool)));
        connect(codeGenerator, SIGNAL(codeGenerated(UMLClassifier*, CodeGenerator::GenerationState)),
                this, SLOT(classGenerated(UMLClassifier*, CodeGenerator::GenerationState)));
        connect(codeGenerator, SIGNAL(showGeneratedFile(QString)),
                this, SLOT(showFileGenerated(QString)));

        UMLClassifierList cList;

        for (int row = 0; row < ui_tableWidgetStatus->rowCount(); ++row) {
            QTableWidgetItem* item = ui_tableWidgetStatus->item(row, 0);
            UMLClassifier *concept = doc->findUMLClassifier(item->text());
            if (concept == 0) {
                uError() << "Could not find classifier " << item->text()
                         << " - not included in generated code.";
                continue;
            }
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
    classGenerated(concept, generated ? CodeGenerator::Generated : CodeGenerator::Failed);
}

/**
 * Updates the status of the code generation in the status table.
 * @param concept     the class for which the code was generated
 * @param state   the state of the generation
 */
void CodeGenStatusPage::classGenerated(UMLClassifier* classifier, CodeGenerator::GenerationState state)
{
    QList<QTableWidgetItem*> items = ui_tableWidgetStatus->findItems(classifier->fullyQualifiedName(), Qt::MatchFixedString);
    if (items.count() > 0) {
        QTableWidgetItem* item = items.at(0);
        if (!item) {
            uError() << "Code Generation Status Page::Error finding class in list view!";
        }
        else {
            int row = ui_tableWidgetStatus->row(item);
            QTableWidgetItem* status = ui_tableWidgetStatus->item(row, 1);
            LedStatus* led = (LedStatus*)ui_tableWidgetStatus->cellWidget(row, 2);
            if (state == CodeGenerator::Generated) {
                status->setText(i18n("Code Generated"));
                led->setOn(true);
            }
            else if (state == CodeGenerator::Failed) {
                status->setText(i18n("Not Generated"));
                led->setColor(Qt::red);
                led->setOn(true);
            }
            else if (state == CodeGenerator::Skipped) {
                status->setText(i18n("Skipped"));
                led->setColor(Qt::gray);
                led->setOn(true);
            }
        }
    }
}

/**
 * Writes the content of the just generated file to the logger text widget.
 */
void CodeGenStatusPage::showFileGenerated(const QString& filename)
{
    ui_textEditLogger->insertHtml(QLatin1String("<b>") + filename + QLatin1String(":</b><br>"));

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            ui_textEditLogger->insertHtml(line + QLatin1String("<br>"));
        }
        file.close();
    }
    else {
        ui_textEditLogger->insertHtml(i18n("Cannot open file!") + QLatin1String("<br>"));
    }
    ui_textEditLogger->insertHtml(QLatin1String("<br><HR><br>"));
}

/**
 * Slot for clicked events generated by the clear button of the logger.
 * Clears the logger widget.
 */
void CodeGenStatusPage::loggerClear()
{
    ui_textEditLogger->setHtml(QString());
}

/**
 * Slot for clicked events generated by the export button of the logger.
 * Writes the content of the logger widget to a file.
 */
void CodeGenStatusPage::loggerExport()
{
    const QString caption = i18n("Umbrello Code Generation - Logger Export");
#if QT_VERSION >= 0x050000
    QString fileName = QFileDialog::getSaveFileName(this, caption, QLatin1String("UmbrelloCodeGenerationLogger.html"));
#else
    QString fileName = KFileDialog::getSaveFileName(KUrl(), QString(), 0, caption);
#endif
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui_textEditLogger->toHtml();
            file.close();
        }
        else {
            KMessageBox::error(this, i18n("Cannot open file!"), caption);
        }
    }
}

