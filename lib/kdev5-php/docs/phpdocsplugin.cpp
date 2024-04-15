/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "phpdocsplugin.h"
#include "phpdocsmodel.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <ksettings/Dispatcher>

#include <interfaces/idocumentation.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <language/duchain/classdeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <QFile>

#include "phpdocsdebug.h"
#include "phpdocumentation.h"
#include "phpdocssettings.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(PhpDocsFactory, "kdevphpdocs.json", registerPlugin<PhpDocsPlugin>();)

PhpDocsPlugin::PhpDocsPlugin(QObject* parent, const QVariantList& args)
    : IPlugin(QStringLiteral("kdevphpdocs"), parent)
    , m_model(new PhpDocsModel(this))
{
    Q_UNUSED(args);

    readConfig();

    KSettings::Dispatcher::registerComponent( QStringLiteral("kdevphpdocs_config"), this, "readConfig" );
}

PhpDocsPlugin::~PhpDocsPlugin()
{
}

QString PhpDocsPlugin::name() const
{
    return QStringLiteral("PHP");
}

QIcon PhpDocsPlugin::icon() const
{
    static QIcon icon = QIcon::fromTheme(QStringLiteral("application-x-php"));
    return icon;
}

void PhpDocsPlugin::readConfig()
{
    // since PhpDocsSettings pointer in this plugin is distinct from the one in the KCM
    // we have to trigger reading manually
    PhpDocsSettings::self()->load();
}

///TODO: possibly return multiple filenames (i.e. fallbacks) when doing local lookups
QString PhpDocsPlugin::getDocumentationFilename( KDevelop::Declaration* dec, const bool& isLocal ) const
{
    QString fname;

    //TODO: most of the SPL stuff is not found for me in the deb package php-doc
    //      => check newer documentation or give a fallback to ref.spl.html
    if ( ClassFunctionDeclaration* fdec = dynamic_cast<ClassFunctionDeclaration*>( dec ) ) {
        // class methods -> php.net/CLASS.METHOD
        // local: either CLASS.METHOD.html or function.CLASS-METHOD.html... really sucks :-/
        //        for now, use the latter...
        if ( dec->context() && dec->context()->type() == DUContext::Class && dec->context()->owner() ) {
            QString className = dec->context()->owner()->identifier().toString();

            if ( !isLocal ) {
                fname = className + '.' + fdec->identifier().toString();
            } else {
                if ( fdec->isConstructor() ) {
                    fname = QStringLiteral("construct");
                } else if ( fdec->isDestructor() ) {
                    fname = QStringLiteral("destruct");
                } else {
                    fname = fdec->identifier().toString();
                }
                //TODO: CLASS.METHOD.html e.g. for xmlreader etc. pp.
                fname = "function." + className + '-' + fname;
            }
        }
    } else if ( dynamic_cast<ClassDeclaration*>(dec) ) {
        fname = "class." + dec->identifier().toString();
    } else if ( dynamic_cast<FunctionDeclaration*>(dec) ) {
        fname = "function." + dec->identifier().toString();
    }
    // check for superglobals / reserved variables
    else if ( dec->identifier() == Identifier(QStringLiteral("GLOBALS")) ||
                dec->identifier() == Identifier(QStringLiteral("php_errormsg")) ||
                dec->identifier() == Identifier(QStringLiteral("HTTP_RAW_POST_DATA")) ||
                dec->identifier() == Identifier(QStringLiteral("http_response_header")) ||
                dec->identifier() == Identifier(QStringLiteral("argc")) ||
                dec->identifier() == Identifier(QStringLiteral("argv")) ||
                dec->identifier() == Identifier(QStringLiteral("_GET")) ||
                dec->identifier() == Identifier(QStringLiteral("_POST")) ||
                dec->identifier() == Identifier(QStringLiteral("_FILES")) ||
                dec->identifier() == Identifier(QStringLiteral("_REQUEST")) ||
                dec->identifier() == Identifier(QStringLiteral("_SESSION")) ||
                dec->identifier() == Identifier(QStringLiteral("_ENV")) ||
                dec->identifier() == Identifier(QStringLiteral("_COOKIE")) ) {
        if ( isLocal ) {
            fname = QStringLiteral("reserved.variables.") + dec->identifier().toString().remove('_');
        } else {
            fname = dec->identifier().toString();
        }
    }

    qCDebug(DOCS) << fname;

    if ( !fname.isEmpty() && isLocal ) {
        fname = fname.toLower();
        fname.replace('_', '-');
        fname.append(".html");
    }

    return fname;
}

IDocumentation::Ptr PhpDocsPlugin::documentationForDeclaration( Declaration* dec ) const
{
    if ( dec ) {
        DUChainReadLocker lock( DUChain::lock() );

        // filter non global or non-php declarations
        if ( dec->topContext()->url() != m_model->internalFunctionFile() ) {
            return {};
        }

        QUrl url = PhpDocsSettings::phpDocLocation();
        qCDebug(DOCS) << url;

        QString file = getDocumentationFilename( dec, url.isLocalFile() );
        if ( file.isEmpty() ) {
            qCDebug(DOCS) << "no documentation pattern found for" << dec->toString();
            return {};
        }

        url.setPath( url.path() + '/' + file);
        if ( url.isLocalFile() && !QFile::exists( url.toLocalFile() ) ) {
            qCDebug(DOCS) << "bad path" << url << "for documentation of" << dec->toString() << " - aborting";
            return {};
        }

        qCDebug(DOCS) << "php documentation located at " << url << "for" << dec->toString();
        return documentationForUrl(url, dec->qualifiedIdentifier().toString(), dec->comment());
    }

    return {};
}

QAbstractListModel* PhpDocsPlugin::indexModel() const
{
    return m_model;
}

IDocumentation::Ptr PhpDocsPlugin::documentationForIndex(const QModelIndex& index) const
{
    return documentationForDeclaration(static_cast<Declaration*>(
        index.data(PhpDocsModel::DeclarationRole).value<DeclarationPointer>().data()
    ));
}

void PhpDocsPlugin::loadUrl(const QUrl& url) const
{
    qCDebug(DOCS) << "loading URL" << url;
    auto doc = documentationForUrl(url, QString());
    ICore::self()->documentationController()->showDocumentation(doc);
}

void PhpDocsPlugin::addToHistory(const QUrl& url)
{
    auto doc = documentationForUrl(url, url.toString());
    Q_EMIT addHistory(doc);
}

IDocumentation::Ptr PhpDocsPlugin::documentationForUrl(const QUrl& url, const QString& name, const QByteArray& description) const
{
    return IDocumentation::Ptr(new PhpDocumentation( url, name, description, const_cast<PhpDocsPlugin*>(this)));
}

IDocumentation::Ptr PhpDocsPlugin::homePage() const
{
    QUrl url = PhpDocsSettings::phpDocLocation();
    if ( url.isLocalFile() ) {
        url.setPath(url.path() + "/index.html");
    } else {
        url.setPath(url.path() + "/manual");
    }
    return documentationForUrl(url, i18n("PHP Documentation"));
}

#include "phpdocsplugin.moc"
