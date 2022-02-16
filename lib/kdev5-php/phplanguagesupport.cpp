/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "phplanguagesupport.h"

#include <QMutexLocker>
#include <QReadWriteLock>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KTextEditor/Document>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
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
#include "phpdebug.h"
#include "codegen/refactoring.h"

#include <language/codecompletion/codecompletion.h>
#include <language/codecompletion/codecompletionmodel.h>

#include "completion/model.h"
#include "completion/worker.h"

#include "navigation/navigationwidget.h"
#include <language/duchain/parsingenvironment.h>

#include "duchain/helper.h"
#include <QTimer>

using namespace KTextEditor;
using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevPhpSupportFactory, "kdevphpsupport.json", registerPlugin<Php::LanguageSupport>(); )

namespace Php
{

LanguageSupport::LanguageSupport(QObject* parent, const QVariantList& /*args*/)
        : KDevelop::IPlugin(QStringLiteral("kdevphpsupport"), parent),
        KDevelop::ILanguageSupport()
{
    Q_ASSERT(internalFunctionFile().toUrl().isValid());

    m_highlighting = new Php::Highlighting(this);
    m_refactoring = new Php::Refactoring(this);

    CodeCompletionModel* ccModel = new CodeCompletionModel(this);
    new KDevelop::CodeCompletion(this, ccModel, name());
}

LanguageSupport::~LanguageSupport()
{
    parseLock()->lockForWrite();
    //By locking the parse-mutexes, we make sure that parse- and preprocess-jobs
    //get a chance to finish in a good state
    parseLock()->unlock();
}

KDevelop::ParseJob *LanguageSupport::createParseJob(const IndexedString &url)
{
    return new ParseJob(url, this);
}

QString LanguageSupport::name() const
{
    return QStringLiteral("Php");
}

KDevelop::ICodeHighlighting* LanguageSupport::codeHighlighting() const
{
    return m_highlighting;
}

KDevelop::ContextMenuExtension LanguageSupport::contextMenuExtension(Context* context)
{
    ContextMenuExtension cm;
    EditorContext *ed = dynamic_cast<KDevelop::EditorContext *>(context);

    if (ed && ICore::self()->languageController()->languagesForUrl(ed->url()).contains(this)) {
        // It's safe to add our own ContextMenuExtension.
        m_refactoring->fillContextMenu(cm, context);
    }
    return cm;
}

QPair<QString, Range> LanguageSupport::wordUnderCursor(const QUrl& url, const Cursor& position)
{
    KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
    if(!doc || !doc->textDocument())
        return {};

    int lineNumber = position.line();
    int lineLength = doc->textDocument()->lineLength(lineNumber);

    QString line = doc->textDocument()->text(Range(lineNumber, 0, lineNumber, lineLength));

    int startCol = position.column();
    for ( ; startCol >= 0; --startCol ) {
        if ( !line[startCol].isLetter() && line[startCol] != '_' ) {
            // don't include the wrong char
            if ( startCol != position.column() ) {
                ++startCol;
            }
            break;
        }
    }
    int endCol = position.column();
    for ( ; endCol <= lineLength; ++endCol ) {
        if ( !line[endCol].isLetter() && line[endCol] != '_' ) {
            break;
        }
    }
    QString word = line.mid(startCol, endCol - startCol);
    Range range(lineNumber, startCol, lineNumber, endCol);
    return qMakePair(word, range);
}

bool isMagicConstant(QPair<QString, Range> word) {
    if ( word.second.isValid() && !word.second.isEmpty() ) {
        if ( word.first == QLatin1String("__FILE__") || word.first == QLatin1String("__LINE__") ||
             word.first == QLatin1String("__METHOD__") || word.first == QLatin1String("__CLASS__") ||
             word.first == QLatin1String("__FUNCTION__") || word.first == QLatin1String("__NAMESPACE__")
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

QWidget* LanguageSupport::specialLanguageObjectNavigationWidget(const QUrl& url, const Cursor& position)
{
    QPair<QString, Range> word = wordUnderCursor(url, position);
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

Range LanguageSupport::specialLanguageObjectRange(const QUrl& url, const Cursor& position)
{
    QPair<QString, Range> word = wordUnderCursor(url, position);
    if ( isMagicConstant(word) ) {
        return word.second;
    }
    return ILanguageSupport::specialLanguageObjectRange(url, position);
}

}

#include "phplanguagesupport.moc"
