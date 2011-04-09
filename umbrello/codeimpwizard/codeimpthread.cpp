/*
    Copyright 2010  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>

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
CodeImpThread::CodeImpThread(QList<QFileInfo> files, QObject* parent)
  : QThread(parent),
    m_files(files)
{
    connect(this, SIGNAL(askQuestion( const QString&, QMessageBox::StandardButton*)),
            this, SLOT(questionAsked( const QString&, QMessageBox::StandardButton*)));
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
    foreach (const QFileInfo& file, m_files) {
        ClassImport *classImporter = ClassImport::createImporterByFileExt(file.fileName());
        QString fileName = file.absoluteFilePath();
        emit messageToWiz(file, "started");
        emit messageToApp(i18n("Importing file: %1", fileName));
        classImporter->importFile(fileName);
        delete classImporter;
        emit messageToApp(i18nc("show Ready on status bar", "Ready."));
        emit messageToWiz(file, "finished");
    }
}

int CodeImpThread::emitAskQuestion(const QString& question)
{
    int buttonCode = 0;
    QMessageBox::StandardButton buttonCodeOld;
    QMutexLocker locker(&m_mutex);
    emit askQuestion(question, &buttonCodeOld);
    m_waitCondition.wait(&m_mutex);
    return buttonCode;
}

void CodeImpThread::questionAsked(const QString& question, QMessageBox::StandardButton* answer)
{
    QMutexLocker locker(&m_mutex);
    *answer = QMessageBox::question(0, "Question:", question, QMessageBox::Yes|QMessageBox::No);
    m_waitCondition.wakeOne();
}
