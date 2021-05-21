/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#include "phpunitprovider.h"
#include "phpunittestsuite.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/itestcontroller.h>
#include <project/projectmodel.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/duchainutils.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>
#include <KStandardDirs>
#include <QVariant>
#include <QTimer>

K_PLUGIN_FACTORY(PhpUnitProviderFactory, registerPlugin<PhpUnitProvider>(); )
K_EXPORT_PLUGIN(PhpUnitProviderFactory(KAboutData("kdevphpunitprovider","kdevphpunitprovider", ki18n("PHPUnit Integration"), "0.1", ki18n("Finds and runs PHPUnit tests"), KAboutData::License_GPL)))


using namespace KDevelop;

PhpUnitProvider::PhpUnitProvider(QObject* parent, const QList< QVariant >& args): IPlugin(PhpUnitProviderFactory::componentData(), parent)
{
    Q_UNUSED(args);

    QString file = KStandardDirs::locate("data", "kdevphpsupport/phpunitdeclarations.php");
    DUChain::self()->updateContextForUrl(IndexedString(file), KDevelop::TopDUContext::AllDeclarationsContextsAndUses, this, -10);

    connect (core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

void PhpUnitProvider::updateReady(const IndexedString& document, const ReferencedTopDUContext& context)
{
    Q_UNUSED(document);

    DUChainReadLocker lock(DUChain::lock());
    QVector<Declaration*> declarations = context.data()->localDeclarations();
    if (declarations.isEmpty())
    {
        kDebug() << "Update of the internal test file found no suitable declarations";
        return;
    }
    m_testCaseDeclaration = IndexedDeclaration(declarations.first());

    kDebug() << "Found declaration" << declarations.first()->toString();
    lock.unlock();


    foreach (const ReferencedTopDUContext& context, m_pendingContexts)
    {
        processContext(context);
    }
}

void PhpUnitProvider::parseJobFinished(KDevelop::ParseJob* job)
{
    ReferencedTopDUContext topContext = job->duChain();
    DUChainReadLocker lock;
    if (!m_testCaseDeclaration.isValid())
    {
        m_pendingContexts << topContext;
    }
    else
    {
        processContext(topContext);
    }
}


void PhpUnitProvider::processContext(ReferencedTopDUContext referencedContext)
{
    kDebug();

    DUChainReadLocker locker;
    TopDUContext* context = referencedContext.data();

    if (!context) {
        kDebug() << "context went away";
        return;
    }

    Declaration* testCase = m_testCaseDeclaration.data();
    if (!testCase) {
        kDebug() << "test case declaration went away";
        return;
    }

    kDebug() << "Number of declarations" << context->localDeclarations().size();

    foreach (Declaration* declaration, context->localDeclarations())
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
    KUrl url = d->url().toUrl();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    kDebug() << name << url << (project ? project->name() : "No project");
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
        foreach (Declaration* member, classDeclaration->internalContext()->localDeclarations())
        {
            kDebug() << "Trying test case declaration" << member;
            if (member->isFunctionDeclaration() && member->identifier().toString().startsWith("test"))
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
    foreach (Declaration* inheriter, DUChainUtils::getInheriters(d, steps))
    {
        processTestCaseDeclaration(inheriter);
    }
}

