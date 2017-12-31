/***************************************************************************
 *   This file is part of KDevelop PHP support                             *
 *   Copyright 2012 Miha Čančula <miha@noughmad.eu>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
    PhpUnitRunJob(PhpUnitTestSuite* suite, const QStringList& cases, KDevelop::OutputJob::OutputJobVerbosity verbosity, QObject* parent = 0);
    virtual void start();

protected:
    virtual bool doKill();

private slots:
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
