/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phpunitrunjob.h"
#include "phpunittestsuite.h"
#include "testdoxdelegate.h"

#include <util/processlinemaker.h>
#include <util/executecompositejob.h>
#include <outputview/outputmodel.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/ilaunchmode.h>

#include <KProcess>
#include <KStandardDirs>
#include <KDebug>
#include <KLocalizedString>
#include <KConfigGroup>

PhpUnitRunJob::PhpUnitRunJob(PhpUnitTestSuite* suite, const QStringList& cases, KDevelop::OutputJob::OutputJobVerbosity verbosity, QObject* parent)
: KJob(parent)
, m_process(0)
, m_suite(suite)
, m_cases(cases)
, m_job(0)
, m_outputJob(0)
, m_verbosity(verbosity)
{
}

KJob* createTestJob(QString launchModeId, QStringList arguments )
{
    KDevelop::LaunchConfigurationType* type = KDevelop::ICore::self()->runController()->launchConfigurationTypeForId( "Script Application" );
    KDevelop::ILaunchMode* mode = KDevelop::ICore::self()->runController()->launchModeForId( launchModeId );

    kDebug() << "got mode and type:" << type << type->id() << mode << mode->id();
    Q_ASSERT(type && mode);

    KDevelop::ILauncher* launcher = 0;
    foreach (KDevelop::ILauncher *l, type->launchers())
    {
        //kDebug() << "available launcher" << l << l->id() << l->supportedModes();
        if (l->supportedModes().contains(mode->id())) {
            launcher = l;
            break;
        }
    }
    Q_ASSERT(launcher);

    KDevelop::ILaunchConfiguration* ilaunch = 0;
    QList<KDevelop::ILaunchConfiguration*> launchConfigurations = KDevelop::ICore::self()->runController()->launchConfigurations();
    foreach (KDevelop::ILaunchConfiguration *l, launchConfigurations) {
        if (l->type() == type && l->config().readEntry("ConfiguredByPhpUnit", false)) {
            ilaunch = l;
            break;
        }
    }
    if (!ilaunch) {
        ilaunch = KDevelop::ICore::self()->runController()->createLaunchConfiguration( type,
                                                qMakePair( mode->id(), launcher->id() ),
                                                0, //TODO add project
                                                i18n("PHPUnit") );
        ilaunch->config().writeEntry("ConfiguredByPhpUnit", true);
        //kDebug() << "created config, launching";
    } else {
        //kDebug() << "reusing generated config, launching";
    }
    type->configureLaunchFromCmdLineArguments( ilaunch->config(), arguments );
    return KDevelop::ICore::self()->runController()->execute(launchModeId, ilaunch);
}

void PhpUnitRunJob::start()
{
    m_process = new KProcess(this);
    // TODO: Arguments from test cases

    QStringList args;

    if (m_cases != m_suite->cases())
    {
        args << "--filter";
        args << '"' + m_cases.join("|") + '"';
    }

    args << "--testdox" << m_suite->name() << m_suite->url().toLocalFile();

    const QString exe = KStandardDirs::findExe("phpunit");
    if (exe.isEmpty()) {
        KDevelop::ITestController* tc = KDevelop::ICore::self()->testController();
        tc->notifyTestRunFinished(m_suite, m_result);
        emitResult();
        return;
    }

    args.prepend(exe);
    args.prepend("php");

    m_job = createTestJob("execute", args);

    m_outputJob = qobject_cast<KDevelop::OutputJob*>(m_job);
    if (!m_outputJob) {
        if (KDevelop::ExecuteCompositeJob* cjob = qobject_cast<KDevelop::ExecuteCompositeJob*>(m_job)) {
            m_outputJob = qobject_cast<KDevelop::OutputJob*>(cjob->subjobs().last());
        }
    }
    Q_ASSERT(m_outputJob);
    if (m_outputJob) {
        m_outputJob->setVerbosity(m_verbosity);
        connect(m_outputJob->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)));
    }

    connect(m_job, SIGNAL(finished(KJob*)), SLOT(processFinished(KJob*)));
}

bool PhpUnitRunJob::doKill()
{
    if (m_job)
    {
        m_job->kill();
    }
    return true;
}

void PhpUnitRunJob::processFinished(KJob* job)
{
    if (job->error() == 1) {
        m_result.suiteResult = KDevelop::TestResult::Failed;
    } else if (job->error() == 0) {
        m_result.suiteResult = KDevelop::TestResult::Passed;
        foreach (KDevelop::TestResult::TestCaseResult result, m_result.testCaseResults)
        {
            if (result == KDevelop::TestResult::Failed)
            {
                m_result.suiteResult = KDevelop::TestResult::Failed;
                break;
            }
        }
    } else {
        m_result.suiteResult = KDevelop::TestResult::Error;
    }

    kDebug() << m_result.suiteResult << m_result.testCaseResults;
    KDevelop::ICore::self()->testController()->notifyTestRunFinished(m_suite, m_result);
    emitResult();
}

void PhpUnitRunJob::rowsInserted(const QModelIndex &parent, int startRow, int endRow)
{
    Q_ASSERT(m_outputJob);
    static QRegExp testResultLineExp = QRegExp("\\[([x\\s])\\]");
    for (int row = startRow; row <= endRow; ++row)
    {
        QString line = m_outputJob->model()->data(m_outputJob->model()->index(row, 0, parent), Qt::DisplayRole).toString();

        int i = testResultLineExp.indexIn(line);
        if (i > -1)
        {
            bool passed = testResultLineExp.cap(1) == "x";
            QString testCase = "test" + line.mid(i+4).toLower().remove(' ');
            kDebug() << "Got result in " << line << " for " << testCase;
            if (m_cases.contains(testCase, Qt::CaseInsensitive))
            {
                foreach (const QString& realCaseName, m_cases)
                {
                    if (QString::compare(testCase, realCaseName, Qt::CaseInsensitive) == 0)
                    {
                        m_result.testCaseResults[testCase] = (passed ? KDevelop::TestResult::Passed : KDevelop::TestResult::Failed);
                        break;
                    }
                }
            }
        }
        else
        {
            kDebug() << line << testResultLineExp.pattern() << i;
        }
    }
}
