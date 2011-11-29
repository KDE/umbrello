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
#include "codeimpthread.h"

// app includes
#include "classimport.h"

// kde includes
#include <klocale.h>

/**
 * Constructor.
 * @param fileNames   name of imported files
 */
CodeImpThread::CodeImpThread(QFileInfo file, QObject* parent)
  : //QThread(parent),
    m_file(file)
{
    connect(this, SIGNAL(askQuestion(QString, QMessageBox::StandardButton*)),
            this, SLOT(questionAsked(QString, QMessageBox::StandardButton*)));
}

/**
 * Destructor.
 */
CodeImpThread::~CodeImpThread()
{
}

/**
 * Thread run method.
 */
void CodeImpThread::run()
{
    ClassImport *classImporter = ClassImport::createImporterByFileExt(m_file.fileName(), this);
    QString fileName = m_file.absoluteFilePath();
    if (classImporter) {
        emit messageToLog(m_file.fileName(), "start import...");
        emit messageToWiz(m_file.fileName(), "started");
        emit messageToApp(i18n("Importing file: %1", fileName));
        classImporter->importFile(fileName);
        delete classImporter;
        emit messageToApp(i18nc("show Ready on status bar", "Ready."));
        emit messageToWiz(m_file.fileName(), "finished");
        emit messageToLog(m_file.fileName(), "...stop import");
    }
    else {
        emit messageToWiz(m_file.fileName(), "aborted");
        emit messageToApp(i18n("No code importer for file: %1", fileName));
    }
}

/**
 * Emit a signal to the main gui thread to show a question box.
 * @param question   the text of the question
 * @return   the code of the answer button
 */
int CodeImpThread::emitAskQuestion(const QString& question)
{
    int buttonCode = 0;
    QMessageBox::StandardButton buttonCodeOld;
    //QMutexLocker locker(&m_mutex);
    emit askQuestion(question, &buttonCodeOld);
    //m_waitCondition.wait(&m_mutex);
    return buttonCode;
}

/**
 * Emit a signal to the main gui thread to write a log text to the log widget.
 * @param file   the file that is in work
 * @param text   the text which has to be added to the log widget
 */
void CodeImpThread::emitMessageToLog(const QString& file, const QString& text)
{
    emit messageToLog(file, text);
}

/**
 * Slot for signal askQuestion.
 * @param question   the question to ask
 * @param answer     the pressed answer button code
 */
void CodeImpThread::questionAsked(const QString& question, QMessageBox::StandardButton* answer)
{
    //QMutexLocker locker(&m_mutex);
    *answer = QMessageBox::question(0, "Question:", question, QMessageBox::Yes|QMessageBox::No);
    //m_waitCondition.wakeOne();
}
