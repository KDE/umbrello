/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2015-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIAGRAMSMODEL_H
#define DIAGRAMSMODEL_H

// app includes
#include "umlviewlist.h"

// qt includes
#include <QAbstractTableModel>
#include <QPointer>

class UMLView;

Q_DECLARE_METATYPE(UMLView*);

class DiagramsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DiagramsModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool addDiagram(UMLView *view);
    bool removeDiagram(UMLView *view);
    bool removeAllDiagrams();

    void emitDataChanged(const QModelIndex &index);
    void emitDataChanged(int index);
    void emitDataChanged(UMLView *view);

protected:
    int m_count;
    UMLViewList m_views;
};

#endif // STEREOTYPESMODEL_H
