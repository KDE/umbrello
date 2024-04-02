/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#include "phpunitprovider.h"

#include "phpunittestsuite.h"
#include "testproviderdebug.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/itestcontroller.h>
#include <project/projectmodel.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/duchainutils.h>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KLocalizedString>
#include <QVariant>
#include <QTimer>
#include <QStandardPaths>

using namespace KDevelop;

K_PLUGIN_FACTORY(PhpUnitProviderFactory, registerPlugin<PhpUnitProvider>(); )

PhpUnitProvider::PhpUnitProvider(QObject* parent, const QList< QVariant >& args)
    : IPlugin(QStringLiteral("kdevphpunitprovider"), parent)
{
    Q_UNUSED(args);

    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevphpsupport/phpunitdeclarations.php"));
    m_phpUnitDeclarationsFile = IndexedString(file);
    DUChain::self()->updateContextForUrl(m_phpUnitDeclarationsFile, KDevelop::TopDUContext::AllDeclarationsContextsAndUses, this, -10);

    connect(DUChain::self(), &DUChain::updateReady,
            this, &PhpUnitProvider::updateReady);
}

void PhpUnitProvider::updateReady(const IndexedString& document, const ReferencedTopDUContext& context)
{
    Q_UNUSED(document);

    DUChainReadLocker lock;
    if (!context) {
        qCDebug(TESTPROVIDER) << "Received null context for file: " << document;
        return;
    }

    if (document == m_phpUnitDeclarationsFile) {
        QVector<Declaration*> declarations = context.data()->localDeclarations();
        if (declarations.isEmpty()) {
            qCDebug(TESTPROVIDER) << "Update of the internal test file found no suitable declarations";
            return;
        }
        m_testCaseDeclaration = IndexedDeclaration(declarations.first());

        qCDebug(TESTPROVIDER) << "Found declaration" << declarations.first()->toString();

        Q_FOREACH(const ReferencedTopDUContext& context, m_pendingContexts) {
            processContext(context);
        }
    } else {
        if (!m_testCaseDeclaration.isValid()) {
            m_pendingContexts << context;
        } else {
            processContext(context);
        }
    }
}


void PhpUnitProvider::processContext(ReferencedTopDUContext referencedContext)
{
    qCDebug(TESTPROVIDER);

    TopDUContext* context = referencedContext.data();

    if (!context) {
        qCDebug(TESTPROVIDER) << "context went away";
        return;
    }

    Declaration* testCase = m_testCaseDeclaration.data();
    if (!testCase) {
        qCDebug(TESTPROVIDER) << "test case declaration went away";
        return;
    }

    qCDebug(TESTPROVIDER) << "Number of declarations" << context->localDeclarations().size();

    Q_FOREACH(Declaration* declaration, context->localDeclarations())
    {
        ClassDeclaration* classDeclaration = dynamic_cast<ClassDeclaration*>(declaration);
        if (!classDeclaration || classDeclaration->isAbstract() || !classDeclaration->internalContext())
        {
            continue;
        }

        if (classDeclaration->isPublicBaseClass(static_cast<ClassDeclaration*>(m_testCaseDeclaration.data()), context)) {
            processTestCaseDeclaration(declaration);
        }
    }
}

void PhpUnitProvider::processTestCaseDeclaration(Declaration* d)
{
    QString name = d->identifier().toString();
    QUrl url = d->url().toUrl();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    qCDebug(TESTPROVIDER) << name << url << (project ? project->name() : QStringLiteral("No project"));
    if (!project)
    {
        return;
    }
    QStringList testCases;
    QHash<QString, IndexedDeclaration> testCaseDeclarations;
    ClassDeclaration* classDeclaration = dynamic_cast<ClassDeclaration*>(d);

    if (!classDeclaration)
    {
        return;
    }

    if (!classDeclaration->isAbstract())
    {
        Q_FOREACH(Declaration* member, classDeclaration->internalContext()->localDeclarations())
        {
            qCDebug(TESTPROVIDER) << "Trying test case declaration" << member;
            if (member->isFunctionDeclaration() && member->identifier().toString().startsWith(QLatin1String("test")))
            {
                const QString caseName = member->identifier().toString();
                testCases << caseName;
                testCaseDeclarations.insert(caseName, IndexedDeclaration(member));
            }
        }

        if (!testCaseDeclarations.isEmpty())
        {
            // NOTE: No declarations usually means the class in abstract
            // This should be resolved by the classDeclaration->isAbstract() check
            // But that always returns false.
            ICore::self()->testController()->addTestSuite(new PhpUnitTestSuite(name, url, IndexedDeclaration(classDeclaration), testCases, testCaseDeclarations, project));
            return;
        }
    }

    uint steps = 100;
    Q_FOREACH(Declaration* inheriter, DUChainUtils::getInheriters(d, steps))
    {
        processTestCaseDeclaration(inheriter);
    }
}

#include "phpunitprovider.moc"
