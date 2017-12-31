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

#include "phpdocsmodel.h"

#include "phpdocsdebug.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <language/duchain/types/structuretype.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include <KLocalizedString>

#include <QStandardPaths>

using namespace KDevelop;

PhpDocsModel::PhpDocsModel(QObject* parent)
    : QAbstractListModel(parent), m_internalFunctionsFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevphpsupport/phpfunctions.php")))
{
    // make sure the php plugin is loaded
    auto phpLangPlugin = ICore::self()->languageController()->language(QStringLiteral("Php"));
    if ( !phpLangPlugin ) {
        qCWarning(DOCS) << "could not load PHP language support plugin";
        return;
    }

    DUChain::self()->updateContextForUrl(internalFunctionFile(), TopDUContext::AllDeclarationsAndContexts, this, -10);
}

PhpDocsModel::~PhpDocsModel()
{
}

IndexedString PhpDocsModel::internalFunctionFile() const
{
    return m_internalFunctionsFile;
}

void PhpDocsModel::updateReady(const IndexedString& file, const ReferencedTopDUContext& top)
{
    if ( file == m_internalFunctionsFile && top ) {
        fillModel(top);
    }
}

void PhpDocsModel::fillModel(const ReferencedTopDUContext& top)
{
    DUChainReadLocker lock;
    if (!top) {
        return;
    }

    qCDebug(DOCS) << "filling model";
    typedef QPair<Declaration*, int> DeclDepthPair;
    foreach ( DeclDepthPair declpair, top->allDeclarations(top->range().end, top) ) {
        if ( declpair.first->abstractType() && declpair.first->abstractType()->modifiers() & AbstractType::ConstModifier ) {
            // filter global constants, since they are hard to find in the documentation
            continue;
        }
        m_declarations << DeclarationPointer(declpair.first);

        if ( StructureType::Ptr type = declpair.first->type<StructureType>() ) {
            foreach ( Declaration* dec, type->internalContext(top)->localDeclarations() ) {
                m_declarations << DeclarationPointer(dec);
            }
        }
    }
}

bool PhpDocsModel::hasChildren(const QModelIndex& parent) const
{
    // only the top-level has children
    return parent == QModelIndex();
}

QVariant PhpDocsModel::data(const QModelIndex& index, int role) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole: {
            DUChainReadLocker lock;
            DeclarationPointer dec = declarationForIndex(index);
            if (!dec.data()) {
                return i18n("<lost declaration>");
            }
            QString ret = dec->toString();
            if ( dec->isFunctionDeclaration() ) {
                // remove function arguments
                ret = ret.replace(QRegExp("\\(.+\\)"), QStringLiteral("()"));
                // remove return type
                ret = ret.remove(QRegExp("^[^ ]+ "));
            }
            return ret;
        }
        case DeclarationRole: {
            DeclarationPointer dec = declarationForIndex(index);
            return QVariant::fromValue<DeclarationPointer>(dec);
        }
        /*
        case Qt::ToolTipRole:
        case Qt::AccessibleTextRole:
        case Qt::AccessibleDescriptionRole:
        */
        default:
            return QVariant();
    }
}

int PhpDocsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_declarations.count();
}

DeclarationPointer PhpDocsModel::declarationForIndex(const QModelIndex& index) const
{
    Q_ASSERT(m_declarations.size() > index.row());

    return m_declarations[index.row()];
}
