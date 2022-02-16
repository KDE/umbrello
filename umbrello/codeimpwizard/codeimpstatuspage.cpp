/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// do not work because there are runtime errors reporting that
// objects derived from QObject could not be called in a different thread
// #define ENABLE_IMPORT_THREAD
#include "codeimpstatuspage.h"

// app includes
#include "classimport.h"  //:TODO: remove it later
#include "codeimpthread.h"
#include "codeimportingwizard.h"  //:TODO: circular reference
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"

//kde includes
#if QT_VERSION < 0x050000
#include <kfiledialog.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>

//qt includes
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#endif
#include <QListWidget>
#include <QTimer>
#include <QScrollBar>

DEBUG_REGISTER(CodeImpStatusPage)

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeImpStatusPage::CodeImpStatusPage(QWidget *parent)
  : QWizardPage(parent),
    m_workDone(false),
    m_savedUndoEnabled(false),
    m_index(0),
    m_savedlistViewVisible(false)
#ifdef ENABLE_IMPORT_THREAD
  , m_thread(0)
#endif
{
    setTitle(i18n("Status of Code Importing Progress"));
    setSubTitle(i18n("Press the button 'Start import' to start the code import.\nCheck the success state for every class."));

    setupUi(this);

    ui_tableWidgetStatus->setColumnCount(3);
    ui_tableWidgetStatus->setColumnWidth(0, 200);
    ui_tableWidgetStatus->setColumnWidth(1, 200);

    connect(ui_pushButtonStart, SIGNAL(clicked()), this, SLOT(importCode()));
    ui_pushButtonStop->setEnabled(false);
    connect(ui_pushButtonStop, SIGNAL(clicked()), this, SLOT(importCodeStop()));
    connect(ui_pushButtonClear, SIGNAL(clicked()), this, SLOT(loggerClear()));
    connect(ui_pushButtonExport, SIGNAL(clicked()), this, SLOT(loggerExport()));
}

/**
 * Destructor.
 */
CodeImpStatusPage::~CodeImpStatusPage()
{
#ifdef ENABLE_IMPORT_THREAD
    delete m_thread;
#endif
}

/**
 * Reimplemented QWizardPage method to initialize page after clicking next button.
 */
void CodeImpStatusPage::initializePage()
{
    ui_tableWidgetStatus->clearContents();
    m_workDone = false;
    populateStatusList();
}

/**
 * Fills the status list with the selected classes for generation.
 */
void CodeImpStatusPage::populateStatusList()
{
    CodeImportingWizard* wiz = (CodeImportingWizard*)wizard();
    m_files = wiz->selectedFiles();

    ui_tableWidgetStatus->setRowCount(m_files.count());
    for (int index = 0; index < m_files.count(); ++index) {
        QFileInfo file = m_files.at(index);
        logDebug1("CodeImpStatusPage::populateStatusList: file %1", file.fileName());
        ui_tableWidgetStatus->setItem(index, 0, new QTableWidgetItem(file.fileName()));
        ui_tableWidgetStatus->setItem(index, 1, new QTableWidgetItem(i18n("Not Yet Generated")));
        CodeImport::LedStatus* led = new CodeImport::LedStatus(70, 70);
        ui_tableWidgetStatus->setCellWidget(index, 2, led);
    }

    if (m_files.count() > 0) {
        ui_pushButtonStart->setEnabled(true);
    }
    else {
        ui_pushButtonStart->setEnabled(false);
    }
}

/**
 * Slot for the start button. Starts the code import.
 */
void CodeImpStatusPage::importCode()
{
    ui_tabWidget->setCurrentIndex(1);  //show the logger tab

    ui_pushButtonStart->setEnabled(false);
    ui_pushButtonStop->setEnabled(true);
    setCommitPage(true);  //:TODO: disable back and cancel button ?

    UMLDoc* doc = UMLApp::app()->document();
    doc->setLoading(true);
    // hide list view dockwindow to avoid time consuming update on every insert
    m_savedlistViewVisible = UMLApp::app()->listView()->parentWidget()->isVisible();
    UMLApp::app()->listView()->parentWidget()->setVisible(false);

    ui_textEditLogger->setHtml(i18np("<b>Code import of 1 file:</b><br>", "<b>Code import of %1 files:</b><br>", m_files.size()));
    
    ui_textEditLogger->insertHtml(QLatin1String("\n") + QLatin1String("<br>"));
    ui_textEditLogger->moveCursor (QTextCursor::End);
    ui_textEditLogger->verticalScrollBar()->setValue(ui_textEditLogger->verticalScrollBar()->maximum()); // move Cursor to the end

    m_index = 0;
    m_workDone = false;
    m_savedUndoEnabled = UMLApp::app()->isUndoEnabled();
    UMLApp::app()->enableUndo(false);

#ifdef ENABLE_IMPORT_THREAD
    m_thread = new QThread;
    //connect(thread, SIGNAL(started()), this, SLOT(importCodeFile()));
    connect(m_thread, SIGNAL(finished(bool)), this, SLOT(importCodeFile(bool)));
    connect(m_thread, SIGNAL(terminated()), this, SLOT(importCodeStop()));
#endif
    importCodeFile();
}

void CodeImpStatusPage::importCodeFile(bool noError)
{
    if (m_index > 0) {
        if (noError) {
            messageToLog(m_file.fileName(), i18n("importing file ... DONE<br>"));
            updateStatus(m_file.fileName(), i18n("Import Done"));
        }
        else {
            messageToLog(m_file.fileName(), i18n("importing file ... FAILED<br>"));
            updateStatus(m_file.fileName(), i18n("Import Failed"));
        }
    }

    // all files done
    if (m_index >= m_files.size()) {
        importCodeFinish();
        return;
    }

    m_file = m_files.at(m_index++);
    messageToLog(m_file.fileName(), i18n("importing file ..."));
    CodeImpThread* worker = new CodeImpThread(m_file);
    connect(worker, SIGNAL(messageToWiz(QString,QString)), this, SLOT(updateStatus(QString,QString)));
    connect(worker, SIGNAL(messageToLog(QString,QString)), this, SLOT(messageToLog(QString,QString)));
    connect(worker, SIGNAL(messageToApp(QString)), this, SLOT(messageToApp(QString)));
#ifndef ENABLE_IMPORT_THREAD
    connect(worker, SIGNAL(finished(bool)), this, SLOT(importNextFile(bool)));
    connect(worker, SIGNAL(aborted()), this, SLOT(importCodeStop()));
    worker->run();
    worker->deleteLater();
#else
    worker->moveToThread(m_thread);
    m_thread->start();
    QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection);
    // FIXME: when to delete worker and m_thread
#endif
    logDebug1("****** CodeImpStatusPage::importCodeFile starting task for %1", m_file.fileName());
}

void CodeImpStatusPage::importNextFile(bool noError)
{
    Q_UNUSED(noError);

    QTimer::singleShot(10, this, SLOT(importCodeFile()));
}

void CodeImpStatusPage::importCodeFinish()
{
    UMLDoc* doc = UMLApp::app()->document();

    UMLApp::app()->enableUndo(m_savedUndoEnabled);

    doc->setLoading(false);
    // Modification is set after the import is made, because the file was modified when adding the classes.
    // Allowing undo of the whole class importing. I think it eats a lot of memory.
    // Setting the modification, but without allowing undo.
    doc->setModified(true);
    UMLApp::app()->listView()->parentWidget()->setVisible(m_savedlistViewVisible);

    m_workDone = true;
    setFinalPage(true);
    emit completeChanged();
#ifdef ENABLE_IMPORT_THREAD
    delete m_thread;
    m_thread = 0;
#endif
}

/**
 * Slot for messageToApp events generated by the code import worker.
 * @param text   the message to write to the status bar of the main window
 */
void CodeImpStatusPage::messageToApp(const QString& text)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->writeToStatusBar(text);
}

/**
 * Slot for the stop button. Stops the code import.
 */
void CodeImpStatusPage::importCodeStop()
{
    messageToLog(m_file.fileName(), i18n("importing file ... stopped<br>"));
    updateStatus(m_file.fileName(), i18n("Import stopped"));

    UMLApp::app()->enableUndo(m_savedUndoEnabled);

    UMLDoc* doc = UMLApp::app()->document();
    doc->setLoading(false);
    // Modification is set after the import is made, because the file was modified when adding the classes.
    // Allowing undo of the whole class importing. I think it eats a lot of memory.
    // Setting the modification, but without allowing undo.
    doc->setModified(true);
    UMLApp::app()->listView()->parentWidget()->setVisible(m_savedlistViewVisible);

    m_workDone = true;
    setFinalPage(true);
    emit completeChanged();
#ifdef ENABLE_IMPORT_THREAD
    delete m_thread;
    m_thread = 0;
#endif
}

/**
 * Reimplemented QWizardPage method the enable / disable the next button.
 * @return   complete state 
 */
bool CodeImpStatusPage::isComplete() const
{
    return m_workDone;
}

/**
 * Writes messages of the code import to a log device.
 * @param file   the file for which the code was imported
 * @param text   the text to display
 */
void CodeImpStatusPage::messageToLog(const QString& file, const QString& text)
{
    if (file.isEmpty()) {
        ui_textEditLogger->insertHtml(QLatin1String("\n    ") + text + QLatin1String("<br>"));
    }
    else {
        ui_textEditLogger->insertHtml(QLatin1String("\n<b>") + file + QLatin1String(":</b> ") + text + QLatin1String("<br>"));
    }
    // move Cursor to the end
    ui_textEditLogger->verticalScrollBar()->setValue(ui_textEditLogger->verticalScrollBar()->maximum());
}

/**
 * Updates the status of the code import in the status table.
 * @param file   the file for which the code was imported
 * @param text   the text to display
 */
void CodeImpStatusPage::updateStatus(const QString& file, const QString& text)
{
    logDebug2("CodeImpStatusPage::updateStatus %1 : %2", file, text);
    QList<QTableWidgetItem*> items = ui_tableWidgetStatus->findItems(file, Qt::MatchFixedString);
    if (items.count() > 0) {
        QTableWidgetItem* item = items.at(0);
        if (!item) {
            logError1("CodeImpStatusPage::updateStatus(%1): Error finding class in list view", file);
        }
        else {
            int row = ui_tableWidgetStatus->row(item);
            QTableWidgetItem* status = ui_tableWidgetStatus->item(row, 1);
            CodeImport::LedStatus* led =
                (CodeImport::LedStatus*)ui_tableWidgetStatus->cellWidget(row, 2);
            if (text.isEmpty()) {
                status->setText(i18n("Not Imported"));
                led->setColor(Qt::red);
                led->setOn(true);
            }
            else {
                status->setText(text);
                led->setOn(true);
            }
        }
    }
}

/**
 * Slot for clicked events generated by the clear button of the logger.
 * Clears the logger widget.
 */
void CodeImpStatusPage::loggerClear()
{
    ui_textEditLogger->setHtml(QString());
}

/**
 * Slot for clicked events generated by the export button of the logger.
 * Writes the content of the logger widget to a file.
 */
void CodeImpStatusPage::loggerExport()
{
    const QString caption = i18n("Umbrello Code Import - Logger Export");
#if QT_VERSION >= 0x050000
    QString fileName = QFileDialog::getSaveFileName(wizard(), caption);
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

