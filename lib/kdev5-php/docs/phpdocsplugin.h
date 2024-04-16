/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PHPDOCSPLUGIN_H
#define PHPDOCSPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationprovider.h>
#include <QVariantList>

class PhpDocsModel;

class PhpDocsPlugin : public KDevelop::IPlugin, public KDevelop::IDocumentationProvider
{
  Q_OBJECT

  Q_INTERFACES( KDevelop::IDocumentationProvider )

  public:
    explicit PhpDocsPlugin(QObject *parent, const QVariantList & args= QVariantList());
    ~PhpDocsPlugin();

    KDevelop::IDocumentation::Ptr documentationForDeclaration (KDevelop::Declaration* dec) const override;
    QAbstractListModel* indexModel() const override;
    KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& index) const override;
    QIcon icon() const override;
    QString name() const override;
    KDevelop::IDocumentation::Ptr homePage() const override;

    void addToHistory(const QUrl& url);

  Q_SIGNALS:
    void addHistory( const KDevelop::IDocumentation::Ptr& doc ) const override;

  public Q_SLOTS:
    void loadUrl(const QUrl &url) const;

  private:
    KDevelop::IDocumentation::Ptr documentationForUrl( const QUrl& url, const QString& name, const QByteArray& description = QByteArray() ) const;

    QString getDocumentationFilename(KDevelop::Declaration* dec, const bool& isLocal) const;
    PhpDocsModel* m_model;

  private Q_SLOTS:
    void readConfig();
};

#endif // PHPDOCSPLUGIN_H
