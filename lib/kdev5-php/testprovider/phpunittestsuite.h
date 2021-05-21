/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPUNITTESTSUITE_H
#define PHPUNITTESTSUITE_H

#include <interfaces/itestsuite.h>
#include <language/duchain/indexeddeclaration.h>

#include <QUrl>
#include <QHash>

class KJob;
namespace KDevelop {
class IProject;
}


class PhpUnitTestSuite : public KDevelop::ITestSuite
{

public:
    PhpUnitTestSuite(const QString& name, const QUrl& url, const KDevelop::IndexedDeclaration& suiteDeclaration, const QStringList& cases, const QHash< QString, KDevelop::IndexedDeclaration >& caseDeclarations, KDevelop::IProject* project);
    virtual ~PhpUnitTestSuite();

    KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity) override;
    KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity) override;
    KJob* launchAllCases(TestJobVerbosity verbosity) override;

    KDevelop::IProject* project() const override;
    QUrl url() const;
    QStringList cases() const override;
    QString name() const override;

    KDevelop::IndexedDeclaration declaration() const override;
    KDevelop::IndexedDeclaration caseDeclaration(const QString& testCase) const override;

private:
    QString m_name;
    QUrl m_url;
    KDevelop::IndexedDeclaration m_declaration;
    QStringList m_cases;
    QHash<QString, KDevelop::IndexedDeclaration> m_caseDeclarations;
    KDevelop::IProject* m_project;
};

#endif // PHPUNITTESTSUITE_H
