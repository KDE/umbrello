/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPUNITRUNJOB_H
#define PHPUNITRUNJOB_H

#include <QProcess>

#include <outputview/outputjob.h>
#include <interfaces/itestsuite.h>
#include <interfaces/itestcontroller.h>
#include <util/executecompositejob.h>

class PhpUnitTestSuite;
class KProcess;

class PhpUnitRunJob : public KJob
{
    Q_OBJECT
public:
    PhpUnitRunJob(PhpUnitTestSuite *suite, const QStringList& cases, KDevelop::OutputJob::OutputJobVerbosity verbosity, QObject* parent = nullptr);
    virtual void start();

protected:
    virtual bool doKill();

private Q_SLOTS:
    void processFinished(KJob* job);
    void rowsInserted(const QModelIndex &parent, int startRow, int endRow);

private:
    KProcess* m_process;
    PhpUnitTestSuite* m_suite;
    QStringList m_cases;
    KDevelop::TestResult m_result;
    KJob* m_job;
    KDevelop::OutputJob* m_outputJob;
    KDevelop::OutputJob::OutputJobVerbosity m_verbosity;
};

class UnprotectedExecuteCompositeJob : public KDevelop::ExecuteCompositeJob
{
    Q_OBJECT
public:
    using ExecuteCompositeJob::subjobs;
};

#endif // PHPUNITRUNJOB_H
