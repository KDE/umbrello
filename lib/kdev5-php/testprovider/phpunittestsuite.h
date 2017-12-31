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
