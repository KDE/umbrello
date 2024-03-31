/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CODEIMPTHREAD_H
#define CODEIMPTHREAD_H

#include <QFileInfo>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

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
    explicit CodeImpThread(QFileInfo& file, QObject* parent = nullptr);
    virtual ~CodeImpThread();

    Q_SLOT virtual void run();

    Q_SLOT int emitAskQuestion(const QString& question);
    Q_SLOT void emitMessageToLog(const QString& file, const QString& text);

    Q_SIGNAL void askQuestion(const QString& question, int& answer);
    Q_SIGNAL void messageToWiz(const QString& file, const QString& text);
    Q_SIGNAL void messageToLog(const QString& file, const QString& text);
    Q_SIGNAL void messageToApp(const QString& text);
    Q_SIGNAL void aborted();
    Q_SIGNAL void failed();
    Q_SIGNAL void finished(bool noError);

private:
    Q_SLOT void questionAsked(const QString& question, int& answer);

private:
    QFileInfo&        m_file;
    //QWaitCondition    m_waitCondition;
    //QMutex            m_mutex;
};

#endif
