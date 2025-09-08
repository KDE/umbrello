/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "stereotypesmodel.h"

// app includes
#include "umlstereotype.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QtDebug>

StereotypesModel::StereotypesModel(UMLStereotypeList& stereotypes)
  : m_count(0),
    m_stereotypes(stereotypes)
{
}

int StereotypesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int count = m_stereotypes.count();
    return count;
}

int StereotypesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant StereotypesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section + 1;
    if (section == 0)
        return QVariant(i18n("Name"));
    else if (section == 1)
        return QVariant(i18n("Usage"));
    else return QVariant();
}

QVariant StereotypesModel::data(const QModelIndex & index, int role) const
{
    if (role == Qt::UserRole && index.column() == 0) {
        QVariant v;
        v.setValue(m_stereotypes.at(index.row()));
        return v;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    int cCount = columnCount(index);
    if (index.column() >= cCount)
        return QVariant();

    UMLStereotype *s = m_stereotypes.at(index.row());
    if (cCount == 1) {
        QString a = s->name() + QString(QStringLiteral(" (%1)")).arg(s->refCount());
        return a;
      }

    // table view
    if (index.column() == 0)
        return s->name();
    else
        return s->refCount();
}

bool StereotypesModel::addStereotype(UMLStereotype *stereotype)
{
    if (m_stereotypes.contains(stereotype))
        return false;
    for(UMLStereotype *s : m_stereotypes) {
        if (s->name() == stereotype->name()) {
            return false;
        }
    }
    int index = m_stereotypes.count();
    beginInsertRows(QModelIndex(), index, index);
    m_stereotypes.append(stereotype);
    endInsertRows();
    return true;
}

bool StereotypesModel::removeStereotype(UMLStereotype *stereotype)
{
    if (!m_stereotypes.contains(stereotype)) {
        UMLStereotype *stFound = nullptr;
        for(UMLStereotype *s : m_stereotypes) {
            if (s->name() == stereotype->name()) {
                stFound = s;
                break;
            }
        }
        if (stFound == nullptr)
            return false;
        stereotype = stFound;
    }
    int index = m_stereotypes.indexOf(stereotype);
    beginRemoveRows(QModelIndex(), index, index);
    m_stereotypes.removeAll(stereotype);
    endRemoveRows();
    return true;
}

void StereotypesModel::emitDataChanged(const QModelIndex &index)
{
    Q_EMIT dataChanged(index, index);
}

void StereotypesModel::emitDataChanged(int index)
{
    QModelIndex mi = createIndex(index,0);
    Q_EMIT dataChanged(mi, mi);
}
