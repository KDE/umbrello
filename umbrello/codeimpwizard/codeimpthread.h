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
    explicit CodeImpThread(QFileInfo& file, QObject *parent = nullptr);
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
    void aborted();
    void failed();
    void finished(bool noError);

private slots:
    void questionAsked(const QString& question, int& answer);

private:
    QFileInfo&        m_file;
    //QWaitCondition    m_waitCondition;
    //QMutex            m_mutex;
};

#endif
