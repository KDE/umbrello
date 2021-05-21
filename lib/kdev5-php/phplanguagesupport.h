/*
    SPDX-FileCopyrightText: 2007 Piyush verma <piyush.verma@gmail.com>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPHPLANGUAGESUPPORT_H
#define KDEVPHPLANGUAGESUPPORT_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <QVariant>

namespace KDevelop
{
class IDocument;
class IProject;
class CodeHighlighting;
class ReferencedTopDUContext;
class ParseJob;
class IndexedString;
}

namespace Php
{

class Highlighting;
class Refactoring;

/**
 * \brief Language Support plugin for PHP
 *
 * All internal PHP declarations can be found in a central document, hitherto called the
 * internal function file. See \p internalFunctionsFile().
 *
 * To access the DUContext, include duchain/helper.h and use:
 * \code
 *  DUChainWriteLocker lock(DUChain::lock());
 *  TopDUContext* ctx = DUChain::self()->chainForDocument(internalFunctionsFile());
 * \endcode
 *
 * To access the destination of the internal function file without linking against the LanguageSupport,
 * like is done in e.g. the PHP-Docs plugin, use:
 * \code
 *  IndexedString url(KStandardDirs::locate("data", "kdevphpsupport/phpfunctions.php"));
 * \endcode
 */
class LanguageSupport : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ILanguageSupport)

public:
    explicit LanguageSupport(QObject *parent, const QVariantList& args = QVariantList());
    virtual ~LanguageSupport();
    /*Name Of the Language*/
    QString name() const override;
    /*Parsejob used by background parser to parse given Url*/
    KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString& url) override;

    static LanguageSupport* self();
    /*the code highlighter*/
    KDevelop::ICodeHighlighting* codeHighlighting() const override;

    /**
     * @returns the ContextMenuExtension for the Php plugin.
     */
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context *context) override;

    QWidget* specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) override;
    KTextEditor::Range specialLanguageObjectRange(const QUrl& url, const KTextEditor::Cursor& position) override;

private:
    KDevelop::CodeHighlighting* m_highlighting;
    Refactoring *m_refactoring;

    QPair<QString, KTextEditor::Range>  wordUnderCursor(const QUrl& url, const KTextEditor::Cursor& position);
};

}

#endif

