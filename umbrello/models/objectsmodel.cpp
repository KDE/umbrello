/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "objectsmodel.h"

// app includes
#include "attribute.h"
#include "folder.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QtDebug>

ObjectsModel::ObjectsModel()
{
}

bool ObjectsModel::add(UMLObject *o)
{
    if (m_allObjects.contains(o))
        return false;
    int index = m_allObjects.size();
    beginInsertRows(QModelIndex(), index, index);
    m_allObjects.append(o);
    endInsertRows();
    return true;
}

bool ObjectsModel::remove(UMLObject *o)
{
    int index = m_allObjects.indexOf(o);
    if (index == -1)
        return false;
    beginRemoveRows(QModelIndex(), index, index);
    m_allObjects.removeAll(o);
    endRemoveRows();
    return true;
}

int ObjectsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int count = m_allObjects.size();
    return count;
}

int ObjectsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 4;
}

QVariant ObjectsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        return QVariant(i18n("Type"));
    else if (section == 2)
        return QVariant(i18n("Folder"));
    else if (section == 3)
        return QVariant(i18n("ID"));
    else return QVariant();
}

QVariant ObjectsModel::data(const QModelIndex & index, int role) const
{
    if (role == Qt::UserRole && index.column() == 0) {
        QVariant v;
        v.setValue(m_allObjects.at(index.row()).data());
        return v;
    }
    else if (role != Qt::DisplayRole)
        return QVariant();

    int cCount = columnCount(index);
    if (index.column() >= cCount)
        return QVariant();

    UMLObject *o  = m_allObjects.at(index.row());
    if (index.column() == 0)
        return o->name();
    else if (index.column() == 1)
        return o->baseTypeStr();
    else if (index.column() == 2) {
        QVariant v;
        if (o->umlPackage())
            v.setValue(o->umlPackage()->name());
        else if (o->parent()) {
            UMLObject *p = dynamic_cast<UMLObject*>(o->parent());
            if (p)
                v.setValue(p->name());
        }
        return v;
    } else if (index.column() == 3)
        return Uml::ID::toString(o->id());
    return QVariant();
}

void ObjectsModel::emitDataChanged(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

void ObjectsModel::emitDataChanged(int index)
{
    QModelIndex mi = createIndex(index,0);
    emit dataChanged(mi, mi);
}

void ObjectsModel::emitDataChanged(UMLObject *o)
{
#if QT_VERSION < 0x050000
    emit layoutAboutToBeChanged();
#endif
    int index  = m_allObjects.indexOf(o);
    emitDataChanged(index);
#if QT_VERSION < 0x050000
    emit layoutChanged();
#endif
}
