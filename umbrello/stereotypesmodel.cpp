/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2015                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "stereotypesmodel.h"

// app includes
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QtDebug>

// TODO: remove in 2.18
#define I18N(a) QLatin1String(a)

StereotypesModel::StereotypesModel(UMLStereotypeList *stereotypes)
  : m_count(0),
    m_stereotypes(stereotypes)
{
}

int StereotypesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int count = m_stereotypes->count();
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
        return QVariant(I18N("Name"));
    else if (section == 1)
        return QVariant(I18N("Usage"));
    else return QVariant();
}

QVariant StereotypesModel::data(const QModelIndex & index, int role) const
{
    if (role == Qt::UserRole && index.column() == 0) {
        QVariant v;
        v.setValue(m_stereotypes->at(index.row()));
        return v;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    int cCount = columnCount(index);
    if (index.column() >= cCount)
        return QVariant();

    UMLStereotype *s = m_stereotypes->at(index.row());
    if (cCount == 1) {
        QString a = s->name() + QString(QLatin1String(" (%1)")).arg(s->refCount());
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
    if (m_stereotypes->contains(stereotype))
        return false;
    int index = m_stereotypes->count();
    beginInsertRows(QModelIndex(), index, index);
    m_stereotypes->append(stereotype);
    endInsertRows();
    return true;
}

bool StereotypesModel::removeStereotype(UMLStereotype *stereotype)
{
    if (!m_stereotypes->contains(stereotype))
        return false;
    int index = m_stereotypes->indexOf(stereotype);
    beginRemoveRows(QModelIndex(), index, index);
    m_stereotypes->removeAll(stereotype);
    endRemoveRows();
    return true;
}

void StereotypesModel::emitDataChanged(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

void StereotypesModel::emitDataChanged(int index)
{
    QModelIndex mi = createIndex(index,0);
    emit dataChanged(mi, mi);
}
