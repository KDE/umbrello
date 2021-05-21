/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef STEREOTYPESMODEL_H
#define STEREOTYPESMODEL_H

// app includes
#include "umlstereotypelist.h"

// qt includes
#include <QAbstractTableModel>

class UMLStereotype;

Q_DECLARE_METATYPE(UMLStereotype*);

class StereotypesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StereotypesModel(UMLStereotypeList& stereotypes);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool addStereotype(UMLStereotype *stereotype);
    bool removeStereotype(UMLStereotype *stereotype);

    void emitDataChanged(const QModelIndex &index);
    void emitDataChanged(int index);

protected:
    int m_count;
    UMLStereotypeList& m_stereotypes;
};

#endif // STEREOTYPESMODEL_H
