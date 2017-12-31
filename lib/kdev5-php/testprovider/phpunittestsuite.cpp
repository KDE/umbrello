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



