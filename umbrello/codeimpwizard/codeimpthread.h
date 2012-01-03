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
#ifndef CODEIMPTHREAD_H
#define CODEIMPTHREAD_H

#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

class ClassImport;

/**
 * Thread class that does the code import work for one file.
 * TODO: For a start it is only a QObject and is used to signals messages.
 * @author Andi Fischer
 */
class CodeImpThread : public QObject
{
    Q_OBJECT
public:
    explicit CodeImpThread(QFileInfo file, QObject* parent = 0);
    virtual ~CodeImpThread();

public slots:
    virtual void run();

    int emitAskQuestion(const QString& question);
    void emitMessageToLog(const QString& file, const QString& text);

signals:
    void askQuestion(const QString& question, int& answer);
    void messageToWiz(const QString& file, const QString& text);
    void messageToLog(const QString& file, const QString& text);
    void messageToApp(const QString& text);
    void finished();
    void aborted();

private slots:
    void questionAsked(const QString& question, int& answer);

private:
    QFileInfo         m_file;
    //QWaitCondition    m_waitCondition;
    //QMutex            m_mutex;
    ClassImport*      m_importer;
};

#endif
