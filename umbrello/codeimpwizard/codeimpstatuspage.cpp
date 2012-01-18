/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

//kde includes
#include <klocale.h>

//qt includes
#include <QtGui/QListWidget>

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeImpStatusPage::CodeImpStatusPage(QWidget *parent)
  : QWizardPage(parent),
    m_workDone(false),
    m_index(0)
{
    setTitle(i18n("Status of Code Importing Progress"));
    setSubTitle(i18n("Press the button 'Start import' to start the code import.\nCheck the success state for every class."));

    setupUi(this);

    ui_tableWidgetStatus->setColumnCount(3);
    ui_tableWidgetStatus->setColumnWidth(0, 200);
    ui_tableWidgetStatus->setColumnWidth(1, 200);

    connect(ui_pushButtonStart, SIGNAL(clicked()), this, SLOT(importCode()));
    ui_pushButtonStop->setEnabled(false);
    connect(ui_pushButtonStop, SIGNAL(clicked()), this, SLOT(importStop()));
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
        uDebug() << file.fileName();
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

    ui_textEditLogger->setHtml(i18np("<b>Code import of 1 file:</b><br>", "<b>Code import of %1 files:</b><br>", m_files.size()));
    m_index = 0;
    m_workDone = false;
#ifdef ENABLE_IMPORT_THREAD
    m_thread = new QThread;
    //connect(thread, SIGNAL(started()), this, SLOT(importCodeFile()));
    connect(m_thread, SIGNAL(finished()), this, SLOT(importCodeFile()));
    connect(m_thread, SIGNAL(terminated()), this, SLOT(importCodeStop()));
#endif
    importCodeFile();
}

void CodeImpStatusPage::importCodeFile()
{
    // all files done
    if (m_index >= m_files.size()) {
        messageToLog(m_file.fileName(), i18n("importing file ... DONE<br>"));
        updateStatus(m_file.fileName(), i18n("Import Done"));
        importCodeFinish();
        return;
    }
    // at least one file done
    else if (m_index > 0) {
        messageToLog(m_file.fileName(), i18n("importing file ... DONE<br>"));
        updateStatus(m_file.fileName(), i18n("Import Done"));
    }

    m_file = m_files.at(m_index++);
    messageToLog(m_file.fileName(), i18n("importing file ..."));
    CodeImpThread* worker = new CodeImpThread(m_file);
    connect(worker, SIGNAL(messageToWiz(QString,QString)), this, SLOT(updateStatus(QString,QString)));
    connect(worker, SIGNAL(messageToLog(QString,QString)), this, SLOT(messageToLog(QString,QString)));
    connect(worker, SIGNAL(messageToApp(QString)), this, SLOT(messageToApp(QString)));
#ifndef ENABLE_IMPORT_THREAD
    connect(worker, SIGNAL(finished()), this, SLOT(importCodeFile()));
    connect(worker, SIGNAL(aborted()), this, SLOT(importCodeStop()));
    worker->run();
    worker->deleteLater();
#else
    worker->moveToThread(m_thread);
    m_thread->start();
    QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection);
    // FIXME: when to delete worker and m_thread
#endif
    uDebug() << "****** starting task for " << m_file.fileName();
}

void CodeImpStatusPage::importCodeFinish()
{
    UMLDoc* doc = UMLApp::app()->document();
    doc->setLoading(false);
    // Modification is set after the import is made, because the file was modified when adding the classes.
    // Allowing undo of the whole class importing. I think it eats a lot of memory.
    // Setting the modification, but without allowing undo.
    doc->setModified(true);

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

    UMLDoc* doc = UMLApp::app()->document();
    doc->setLoading(false);
    // Modification is set after the import is made, because the file was modified when adding the classes.
    // Allowing undo of the whole class importing. I think it eats a lot of memory.
    // Setting the modification, but without allowing undo.
    doc->setModified(true);

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
    QString oldText = ui_textEditLogger->toHtml();
    QString newText('\n');
    if (file.isEmpty()) {
        newText.append("    " + text);
    }
    else {
        newText.append("<b>" + file + ":</b> " + text);
    }
    oldText.append(newText);
    ui_textEditLogger->setHtml(oldText);
}

/**
 * Updates the status of the code import in the status table.
 * @param file   the file for which the code was imported
 * @param text   the text to display
 */
void CodeImpStatusPage::updateStatus(const QString& file, const QString& text)
{
    uDebug() << file << " : " << text;
    QList<QTableWidgetItem*> items = ui_tableWidgetStatus->findItems(file, Qt::MatchFixedString);
    if (items.count() > 0) {
        QTableWidgetItem* item = items.at(0);
        if ( !item ) {
            uError() << "Code Importing Status Page::Error finding class <" << file << "> in list view!";
        }
        else {
            int row = ui_tableWidgetStatus->row(item);
            QTableWidgetItem* status = ui_tableWidgetStatus->item(row, 1);
            CodeImport::LedStatus* led =
                (CodeImport::LedStatus*)ui_tableWidgetStatus->cellWidget(row, 2);
            if (text.isEmpty()) {
                status->setText( i18n("Not Imported") );
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
    ui_textEditLogger->setHtml("");
}

/**
 * Slot for clicked events generated by the export button of the logger.
 * Writes the content of the logger widget to a file.
 */
void CodeImpStatusPage::loggerExport()
{
    uDebug() << "TODO: Not yet implemented!";
}

#include "codeimpstatuspage.moc"
