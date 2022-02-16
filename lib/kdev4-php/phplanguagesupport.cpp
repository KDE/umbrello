/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "phplanguagesupport.h"

#include <QMutexLocker>
#include <QReadWriteLock>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KTextEditor/Document>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/editorcontext.h>

#include "phpparsejob.h"
#include "phphighlighting.h"
#include "kdevphpversion.h"
#include "codegen/refactoring.h"

#include <language/codecompletion/codecompletion.h>
#include <language/codecompletion/codecompletionmodel.h>

#include "completion/model.h"
#include "completion/worker.h"

#include "navigation/navigationwidget.h"
#include <language/duchain/parsingenvironment.h>

#include "duchain/helper.h"
#include <QTimer>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevPhpSupportFactory, registerPlugin<Php::LanguageSupport>();)
K_EXPORT_PLUGIN(KDevPhpSupportFactory(
    KAboutData("kdevphpsupport", "kdevphp", ki18n("PHP Support"),
               KDEVPHP_VERSION_STR, ki18n("Support for PHP Language"), KAboutData::License_GPL)
    .addAuthor(ki18n("Milian Wolff"), ki18n("Author"), "mail@milianw.de", "https://milianw.de")
    .addAuthor(ki18n("Niko Sams"), ki18n("Author"), "niko.sams@gmail.com", "https://nikosams.blogspot.com")
))

namespace Php
{

LanguageSupport::LanguageSupport(QObject* parent, const QVariantList& /*args*/)
        : KDevelop::IPlugin(KDevPhpSupportFactory::componentData(), parent),
        KDevelop::ILanguageSupport(), m_internalFunctionsLoaded(false)
{
    Q_ASSERT(internalFunctionFile().toUrl().isValid());
    m_internalFunctionsLock.lockForWrite();

    KDEV_USE_EXTENSION_INTERFACE(KDevelop::ILanguageSupport)

    m_highlighting = new Php::Highlighting(this);
    m_refactoring = new Php::Refactoring(this);

    CodeCompletionModel* ccModel = new CodeCompletionModel(this);
    new KDevelop::CodeCompletion(this, ccModel, name());

    QTimer::singleShot(0, this, SLOT(updateInternalFunctions()));
}

LanguageSupport::~LanguageSupport()
{
    ILanguage* lang = language();
    if ( lang ) {
        lang->parseLock()->lockForWrite();
        //By locking the parse-mutexes, we make sure that parse- and preprocess-jobs
        //get a chance to finish in a good state
        lang->parseLock()->unlock();
    }
}

void LanguageSupport::updateInternalFunctions()
{
    Q_ASSERT(core()->pluginController()->loadedPlugins().contains(this));
    kDebug() << "making sure that internal function file is up to date";
    DUChain::self()->updateContextForUrl(internalFunctionFile(), KDevelop::TopDUContext::AllDeclarationsAndContexts, this, -10);
}

void LanguageSupport::updateReady( const IndexedString& url, const ReferencedTopDUContext& topContext )
{
    Q_UNUSED(topContext);
    if (url == internalFunctionFile())
    {
        DUChain::self()->updateContextForUrl(internalTestFile(), KDevelop::TopDUContext::AllDeclarationsAndContexts, this, -10);
        m_internalFunctionsLoaded = true;
        m_internalFunctionsLock.unlock();
    }
}

bool LanguageSupport::internalFunctionsLoaded() const
{
    return m_internalFunctionsLoaded;
}

QReadWriteLock* LanguageSupport::internalFunctionsLock()
{
    return &m_internalFunctionsLock;
}

KDevelop::ParseJob *LanguageSupport::createParseJob(const IndexedString &url)
{
    return new ParseJob(url, this);
}

QString LanguageSupport::name() const
{
    return "Php";
}

KDevelop::ILanguage *LanguageSupport::language()
{
    return core()->languageController()->language(name());
}

KDevelop::ICodeHighlighting* LanguageSupport::codeHighlighting() const
{
    return m_highlighting;
}

KDevelop::ContextMenuExtension LanguageSupport::contextMenuExtension(Context* context)
{
    ContextMenuExtension cm;
    EditorContext *ed = dynamic_cast<KDevelop::EditorContext *>(context);

    if (ed && ICore::self()->languageController()->languagesForUrl(ed->url()).contains(language())) {
        // It's safe to add our own ContextMenuExtension.
        m_refactoring->fillContextMenu(cm, context);
    }
    return cm;
}

QPair<QString, SimpleRange> LanguageSupport::wordUnderCursor(const KUrl& url, const SimpleCursor& position)
{
    KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
    if(!doc || !doc->textDocument() || !doc->textDocument()->activeView())
        return qMakePair(QString(), SimpleRange::invalid());

    int lineNumber = position.line;
    int lineLength = doc->textDocument()->lineLength(lineNumber);

    QString line = doc->textDocument()->text(KTextEditor::Range(lineNumber, 0, lineNumber, lineLength));

    int startCol = position.column;
    for ( ; startCol >= 0; --startCol ) {
        if ( !line[startCol].isLetter() && line[startCol] != '_' ) {
            // don't include the wrong char
            if ( startCol != position.column ) {
                ++startCol;
            }
            break;
        }
    }
    int endCol = position.column;
    for ( ; endCol <= lineLength; ++endCol ) {
        if ( !line[endCol].isLetter() && line[endCol] != '_' ) {
            break;
        }
    }
    QString word = line.mid(startCol, endCol - startCol);
    SimpleRange range(lineNumber, startCol, lineNumber, endCol);
    return qMakePair(word, range);
}

bool isMagicConstant(QPair<QString, SimpleRange> word) {
    if ( word.second.isValid() && !word.second.isEmpty() ) {
        if ( word.first == "__FILE__" || word.first == "__LINE__" ||
             word.first == "__METHOD__" || word.first == "__CLASS__" ||
             word.first == "__FUNCTION__" || word.first == "__NAMESPACE__"
             ///TODO: php 5.3: __DIR__
           )
        {
            ///TODO: maybe we should use the tokenizer to really make sure this is such a token
            ///      and we are not inside a string, comment or similar
            ///      otoh, it doesn't hurt imo
            return true;
        }
    }
    return false;
}

QWidget* LanguageSupport::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position)
{
    QPair<QString, SimpleRange> word = wordUnderCursor(url, position);
    if ( isMagicConstant(word) ) {
        DUChainReadLocker lock;
        if (TopDUContext* top = standardContext(url)) {
            return new NavigationWidget(TopDUContextPointer(top), position, word.first);
        } else {
            return 0;
        }
    }
    return ILanguageSupport::specialLanguageObjectNavigationWidget(url, position);
}

SimpleRange LanguageSupport::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position)
{
    QPair<QString, SimpleRange> word = wordUnderCursor(url, position);
    if ( isMagicConstant(word) ) {
        return word.second;
    }
    return ILanguageSupport::specialLanguageObjectRange(url, position);
}

}

#include "phplanguagesupport.moc"
