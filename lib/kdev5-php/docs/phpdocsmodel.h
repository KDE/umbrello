/*  This file is part of the KDevelop PHP Documentation Plugin

    Copyright 2012 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    explicit PhpDocsModel(QObject* parent = 0);
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

private:
    /// fills model with all declarations from the internal PHP functions file
    void fillModel(const KDevelop::ReferencedTopDUContext& topContext);

    /// List of pointers to _all_ PHP internal declarations
    QList<KDevelop::DeclarationPointer> m_declarations;

    /// internal function file
    const KDevelop::IndexedString m_internalFunctionsFile;

public slots:
    void updateReady( const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext& topContext );

};

#endif // PHPDOCSMODEL_H
