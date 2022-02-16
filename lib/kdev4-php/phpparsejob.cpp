/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "phpparsejob.h"
#include <QFile>
#include <QReadWriteLock>

#include <ktexteditor/document.h>

#include <kdebug.h>
#include <klocale.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/dumpchain.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/urlparselock.h>

#include "editorintegrator.h"
#include "parsesession.h"
#include "phplanguagesupport.h"
#include "phpdebugvisitor.h"
#include "duchain/builders/declarationbuilder.h"
#include "duchain/builders/usebuilder.h"
#include "duchain/helper.h"
#include "phpducontext.h"

#include <QtCore/QReadLocker>
#include <QtCore/QThread>
#include <language/duchain/duchainutils.h>

using namespace KDevelop;

namespace Php
{

ParseJob::ParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: KDevelop::ParseJob(url, languageSupport)
, m_parentJob(0)
{
}

ParseJob::~ParseJob()
{
}

LanguageSupport* ParseJob::php() const
{
    return dynamic_cast<LanguageSupport*>(languageSupport());
}

void ParseJob::run()
{
    /// Indexed string for 'Php', identifies environment files from this language plugin
    static const IndexedString phpLangString("Php");

    // make sure we loaded the internal file already
    if ( !php()->internalFunctionsLoaded() && !m_parentJob && document() != internalFunctionFile() ) {
        kDebug() << "waiting for internal function file to finish parsing";
        QReadLocker(php()->internalFunctionsLock());
    }

    UrlParseLock urlLock(document());

    if (!(minimumFeatures() & Resheduled) && !isUpdateRequired(phpLangString)) {
        return;
    }
    kDebug() << "parsing" << document().str();

    KDevelop::ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return abortJob();;
    }

    ParseSession session;
    //TODO: support different charsets
    session.setContents(QString::fromUtf8(contents().contents));
    session.setCurrentDocument(document());

    // 2) parse
    StartAst* ast = 0;
    bool matched = session.parse(&ast);

    if (abortRequested() || ICore::self()->shuttingDown()) {
        return abortJob();
    }

    KDevelop::ReferencedTopDUContext toUpdate;
    {
        KDevelop::DUChainReadLocker duchainlock(KDevelop::DUChain::lock());
        toUpdate = KDevelop::DUChainUtils::standardContextForUrl(document().toUrl());
    }

    KDevelop::TopDUContext::Features newFeatures = minimumFeatures();
    if (toUpdate)
        newFeatures = (KDevelop::TopDUContext::Features)(newFeatures | toUpdate->features());

    //Remove update-flags like 'Recursive' or 'ForceUpdate'
    newFeatures = static_cast<KDevelop::TopDUContext::Features>(newFeatures & KDevelop::TopDUContext::AllDeclarationsContextsUsesAndAST);

    if (matched) {
        if (abortRequested()) {
            return abortJob();
        }

        EditorIntegrator editor(&session);

        QReadLocker parseLock(php()->language()->parseLock());

        DeclarationBuilder builder(&editor);
        KDevelop::ReferencedTopDUContext chain = builder.build(document(), ast, toUpdate);

        if (abortRequested()) {
            return abortJob();
        }

        setDuChain(chain);

        bool hadUnresolvedIdentifiers = builder.hadUnresolvedIdentifiers();

        if ( newFeatures & TopDUContext::AllDeclarationsContextsAndUses
                && document() != internalFunctionFile() )
        {
            UseBuilder useBuilder(&editor);
            useBuilder.buildUses(ast);

            if (useBuilder.hadUnresolvedIdentifiers())
                hadUnresolvedIdentifiers = true;
        }

        if (hadUnresolvedIdentifiers) {
            if (!(minimumFeatures() & Resheduled) && KDevelop::ICore::self()->languageController()->backgroundParser()->queuedCount()) {
                // Need to create new parse job with lower priority
                kDebug() << "Reschedule file " << document().str() << "for parsing";
                KDevelop::TopDUContext::Features feat = static_cast<KDevelop::TopDUContext::Features>(
                        minimumFeatures() | KDevelop::TopDUContext::VisibleDeclarationsAndContexts | Resheduled
                    );
                int priority = qMin(parsePriority()+100, (int)KDevelop::BackgroundParser::WorstPriority);
                KDevelop::ICore::self()->languageController()->backgroundParser()
                    ->addDocument(document(), feat, priority);

            } else {
                // We haven't resolved all identifiers, but by now, we don't expect to
                kDebug() << "Builder found unresolved identifiers when they should have been resolved! (if there was no coding error)";
            }
        }

        if (abortRequested()) {
            return abortJob();
        }

        if (abortRequested()) {
            return abortJob();
        }

        {
            DUChainWriteLocker lock(DUChain::lock());

            foreach(const ProblemPointer &p, session.problems()) {
                chain->addProblem(p);
            }

            chain->setFeatures(newFeatures);
            ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
            file->setModificationRevision(contents().modification);
            DUChain::self()->updateContextEnvironment( chain->topContext(), file.data() );
        }

        highlightDUChain();
    } else {
        ReferencedTopDUContext top;
        DUChainWriteLocker lock;
        {
            top = DUChain::self()->chainForDocument(document());
        }
        if (top) {
            ///NOTE: if we clear the imported parent contexts, autocompletion of built-in PHP stuff won't work!
            //top->clearImportedParentContexts();
            top->parsingEnvironmentFile()->clearModificationRevisions();
            top->clearProblems();
        } else {
            ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
            file->setLanguage(phpLangString);
            top = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
            DUChain::self()->addDocumentChain(top);
        }
        foreach(const ProblemPointer &p, session.problems()) {
            top->addProblem(p);
        }
        setDuChain(top);
        kDebug() << "===Failed===" << document().str();
    }
}

void ParseJob::setParentJob(ParseJob *job)
{
    m_parentJob = job;
}


bool ParseJob::hasParentDocument(const IndexedString &doc)
{
    if (document() == doc) return true;
    if (!m_parentJob) return false;
    if (m_parentJob->document() == doc) return true;
    return m_parentJob->hasParentDocument(doc);
}

ProblemPointer ParseJob::createProblem(const QString &description, AstNode* node,
                                       EditorIntegrator * editor, ProblemData::Source source,
                                       ProblemData::Severity severity)
{
    ProblemPointer p(new Problem());
    p->setSource(source);
    p->setSeverity(severity);
    p->setDescription(description);
    p->setFinalLocation(DocumentRange(document(), editor->findRange(node).castToSimpleRange()));
    kDebug() << p->description();
    return p;
}

}

#include "phpparsejob.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
