/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phpunitrunjob.h"
#include "phpunittestsuite.h"
#include "testdoxdelegate.h"
#include "testproviderdebug.h"

#include <QStandardPaths>

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
    KDevelop::LaunchConfigurationType* type = KDevelop::ICore::self()->runController()->launchConfigurationTypeForId( QStringLiteral("Script Application") );
    KDevelop::ILaunchMode* mode = KDevelop::ICore::self()->runController()->launchModeForId( launchModeId );

    qCDebug(TESTPROVIDER) << "got mode and type:" << type << type->id() << mode << mode->id();
    Q_ASSERT(type && mode);

    KDevelop::ILauncher *launcher = nullptr;
    for(KDevelop::ILauncher  *l : type->launchers())
    {
        //qCDebug(TESTPROVIDER) << "available launcher" << l << l->id() << l->supportedModes();
        if (l->supportedModes().contains(mode->id())) {
            launcher = l;
            break;
        }
    }
    Q_ASSERT(launcher);

    KDevelop::ILaunchConfiguration *ilaunch = nullptr;
    QList<KDevelop::ILaunchConfiguration*> launchConfigurations = KDevelop::ICore::self()->runController()->launchConfigurations();
    for(KDevelop::ILaunchConfiguration  *l : launchConfigurations) {
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
        //qCDebug(TESTPROVIDER) << "created config, launching";
    } else {
        //qCDebug(TESTPROVIDER) << "reusing generated config, launching";
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
        args << QStringLiteral("--filter");
        args << '"' + m_cases.join(QStringLiteral("|")) + '"';
    }

    args << QStringLiteral("--testdox") << m_suite->name() << m_suite->url().toLocalFile();

    const QString exe = QStandardPaths::findExecutable(QStringLiteral("phpunit"));
    if (exe.isEmpty()) {
        KDevelop::ITestController* tc = KDevelop::ICore::self()->testController();
        tc->notifyTestRunFinished(m_suite, m_result);
        emitResult();
        return;
    }

    args.prepend(exe);
    args.prepend(QStringLiteral("php"));

    m_job = createTestJob(QStringLiteral("execute"), args);

    m_outputJob = qobject_cast<KDevelop::OutputJob*>(m_job);
    if (!m_outputJob) {
        if (UnprotectedExecuteCompositeJob* cjob = qobject_cast<UnprotectedExecuteCompositeJob*>(m_job)) {
            m_outputJob = qobject_cast<KDevelop::OutputJob*>(cjob->subjobs().last());
        }
    }
    Q_ASSERT(m_outputJob);
    if (m_outputJob) {
        m_outputJob->setVerbosity(m_verbosity);
        connect(m_outputJob->model(), &QAbstractItemModel::rowsInserted, this, &PhpUnitRunJob::rowsInserted);
    }

    connect(m_job, &KJob::finished, this, &PhpUnitRunJob::processFinished);
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
        for(KDevelop::TestResult::TestCaseResult result: m_result.testCaseResults)
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

    qCDebug(TESTPROVIDER) << m_result.suiteResult << m_result.testCaseResults;
    KDevelop::ICore::self()->testController()->notifyTestRunFinished(m_suite, m_result);
    emitResult();
}

void PhpUnitRunJob::rowsInserted(const QModelIndex &parent, int startRow, int endRow)
{
    Q_ASSERT(m_outputJob);
    static QRegularExpression testResultLineExp = QRegularExpression(QStringLiteral("\\[([x\\s])\\]"));
    for (int row = startRow; row <= endRow; ++row)
    {
        QString line = m_outputJob->model()->data(m_outputJob->model()->index(row, 0, parent), Qt::DisplayRole).toString();

        int i = testResultLineExp.indexIn(line);
        if (i > -1)
        {
            bool passed = testResultLineExp.cap(1) == QLatin1String("x");
            QString testCase = "test" + line.mid(i+4).toLower().remove(' ');
            qCDebug(TESTPROVIDER) << "Got result in " << line << " for " << testCase;
            if (m_cases.contains(testCase, Qt::CaseInsensitive))
            {
                for(const QString& realCaseName: m_cases)
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
            qCDebug(TESTPROVIDER) << line << testResultLineExp.pattern() << i;
        }
    }
}
