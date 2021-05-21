/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPUNITTESTSUITE_H
#define PHPUNITTESTSUITE_H

#include <interfaces/itestsuite.h>
#include <language/duchain/indexeddeclaration.h>
#include <KUrl>

class KJob;
namespace KDevelop {
class IProject;
}


class PhpUnitTestSuite : public KDevelop::ITestSuite
{

public:
    PhpUnitTestSuite(const QString& name, const KUrl& url, const KDevelop::IndexedDeclaration& suiteDeclaration, const QStringList& cases, const QHash< QString, KDevelop::IndexedDeclaration >& caseDeclarations, KDevelop::IProject* project);
    virtual ~PhpUnitTestSuite();

    virtual KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity);
    virtual KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity);
    virtual KJob* launchAllCases(TestJobVerbosity verbosity);

    virtual KDevelop::IProject* project() const;
    virtual KUrl url() const;
    virtual QStringList cases() const;
    virtual QString name() const;

    virtual KDevelop::IndexedDeclaration declaration() const;
    virtual KDevelop::IndexedDeclaration caseDeclaration(const QString& testCase) const;

private:
    QString m_name;
    KUrl m_url;
    KDevelop::IndexedDeclaration m_declaration;
    QStringList m_cases;
    QHash<QString, KDevelop::IndexedDeclaration> m_caseDeclarations;
    KDevelop::IProject* m_project;
};

#endif // PHPUNITTESTSUITE_H
