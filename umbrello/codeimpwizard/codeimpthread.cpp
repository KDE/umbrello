/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "codeimpthread.h"

// app includes
#include "classimport.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

/**
 * Constructor.
 * @param parent  QObject which acts as parent to this CodeImpThread
 * @param file  File to import for which the thread shall be spawned
 */
CodeImpThread::CodeImpThread(QFileInfo& file, QObject* parent)
  : QObject(parent),
    m_file(file)
{
    connect(this, SIGNAL(askQuestion(QString,int&)),
            this, SLOT(questionAsked(QString,int&)));
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
        emit messageToLog(m_file.fileName(), QStringLiteral("start import..."));
        emit messageToWiz(m_file.fileName(), QStringLiteral("started"));
        emit messageToApp(i18n("Importing file: %1", fileName));
        // FIXME: ClassImport still uses umldoc->writeToStatusBar for log writing

        if (!classImporter->importFile(fileName)) {
            emit messageToApp(i18nc("show failed on status bar", "Failed."));
            emit messageToWiz(m_file.fileName(), QString());
            emit messageToLog(m_file.fileName(), QStringLiteral("...import failed"));
            emit finished(false);
        }
        else {
            emit messageToApp(i18nc("show Ready on status bar", "Ready."));
            emit messageToWiz(m_file.fileName(), QStringLiteral("finished"));
            emit messageToLog(m_file.fileName(), QStringLiteral("...import finished"));
            emit finished(true);
        }
        delete classImporter;
    }
    else {
        emit messageToWiz(m_file.fileName(), QStringLiteral("aborted"));
        emit messageToApp(i18n("No code importer for file: %1", fileName));
        emit aborted();
    }
}

/**
 * Emit a signal to the main gui thread to show a question box.
 * @param question   the text of the question
 * @return   the code of the answer button KMessageBox::ButtonCode
 */
int CodeImpThread::emitAskQuestion(const QString& question)
{
    int buttonCode = 0;
    //QMutexLocker locker(&m_mutex);
    emit askQuestion(question, buttonCode);
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
    if (file.isEmpty()) {
        emit messageToLog(m_file.fileName(), text);
    } else {
        emit messageToLog(file, text);
    }
}

/**
 * Slot for signal askQuestion.
 * @param question   the question to ask
 * @param answer     the pressed answer button code @see KMessageBox::ButtonCode
 */
void CodeImpThread::questionAsked(const QString& question, int& answer)
{
    //QMutexLocker locker(&m_mutex);
    answer = KMessageBox::questionYesNo(0, question, QStringLiteral("Question code import:")); // @todo i18n
    //m_waitCondition.wakeOne();
}
