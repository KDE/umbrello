/*  This file is part of the KDevelop PHP Documentation Plugin

    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PHPDOCSMODEL_H
#define PHPDOCSMODEL_H

#include <QStringListModel>

#include <serialization/indexedstring.h>
#include <language/duchain/declaration.h>

namespace KDevelop
{
    class Declaration;
    class ParseJob;
    class ReferencedTopDUContext;
}

class PhpDocsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PhpDocsModel(QObject* parent = nullptr);
    ~PhpDocsModel();

    enum CustomDataRoles {
        /// returns the Declaration that a given index in the model represents
        DeclarationRole = Qt::UserRole
    };

    /**
     * You can use @p DeclarationRole to get the Declaration for a given index.
     * NOTE: If you use that, don't forget to lock the DUChain if you access the declaration!
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent) const override;

    /// Returns the Declaration for a given index
    /// NOTE: Don't forget to lock the DUChain if you access the declaration!
    KDevelop::DeclarationPointer declarationForIndex(const QModelIndex& index) const;

    /// Returns the destination of the internal PHP function file
    /// @see PhpLanguageSupport
    KDevelop::IndexedString internalFunctionFile() const;

    Q_SLOT void updateReady( const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext );

private:
    /// fills model with all declarations from the internal PHP functions file
    void fillModel(const KDevelop::ReferencedTopDUContext& topContext);

    /// List of pointers to _all_ PHP internal declarations
    QList<KDevelop::DeclarationPointer> m_declarations;

    /// internal function file
    const KDevelop::IndexedString m_internalFunctionsFile;


};

#endif // PHPDOCSMODEL_H
