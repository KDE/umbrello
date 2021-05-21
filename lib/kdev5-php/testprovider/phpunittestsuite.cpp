/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phpunittestsuite.h"
#include "phpunitrunjob.h"

using namespace KDevelop;

PhpUnitTestSuite::PhpUnitTestSuite(const QString& name, const QUrl& url, const IndexedDeclaration& suiteDeclaration,
                                   const QStringList& cases, const QHash< QString, IndexedDeclaration >& caseDeclarations,
                                   IProject* project)
    : m_name(name)
    , m_url(url)
    , m_declaration(suiteDeclaration)
    , m_cases(cases)
    , m_caseDeclarations(caseDeclarations)
    , m_project(project)
{
}

PhpUnitTestSuite::~PhpUnitTestSuite()
{

}

KJob* PhpUnitTestSuite::launchCase(const QString& testCase, TestJobVerbosity verbosity)
{
    return launchCases(QStringList() << testCase, verbosity);
}

KJob* PhpUnitTestSuite::launchCases(const QStringList& testCases, TestJobVerbosity verbosity)
{
    OutputJob::OutputJobVerbosity outputVerbosity = (verbosity == Verbose) ? OutputJob::Verbose : OutputJob::Silent;
    return new PhpUnitRunJob(this, testCases, outputVerbosity);
}

KJob* PhpUnitTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    return launchCases(m_cases, verbosity);
}

KDevelop::IProject* PhpUnitTestSuite::project() const
{
    return m_project;
}

QUrl PhpUnitTestSuite::url() const
{
    return m_url;
}

QStringList PhpUnitTestSuite::cases() const
{
    return QStringList(m_cases);
}

QString PhpUnitTestSuite::name() const
{
    return m_name;
}

IndexedDeclaration PhpUnitTestSuite::declaration() const
{
    return m_declaration;
}

IndexedDeclaration PhpUnitTestSuite::caseDeclaration(const QString& testCase) const
{
    return m_caseDeclarations.value(testCase, IndexedDeclaration(0));
}



