/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OBJECTSMODEL_H
#define OBJECTSMODEL_H

// qt includes
#include <QAbstractTableModel>
#include <QPointer>

class UMLObject;

class ObjectsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ObjectsModel();

    bool add(UMLObject *o);
    bool remove(UMLObject *o);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    void emitDataChanged(const QModelIndex &index);
    void emitDataChanged(int index);
    void emitDataChanged(UMLObject *o);

protected:
    QList<QPointer<UMLObject>> m_allObjects;
};

#endif // OBJECTSMODEL_H
