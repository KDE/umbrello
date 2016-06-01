/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "diagramsmodel.h"

// app includes
#include "umlscene.h"
#include "umlview.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QtDebug>

DiagramsModel::DiagramsModel()
{
}

int DiagramsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int count = m_views.size();
    return count;
}

int DiagramsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QVariant DiagramsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        return QVariant(i18n("Widgets/Associations"));
    else return QVariant();
}

QVariant DiagramsModel::data(const QModelIndex & index, int role) const
{
    if (role == Qt::UserRole && index.column() == 0) {
        QVariant v;
        v.setValue(m_views.at(index.row()).data());
        return v;
    }
    else if (role == Qt::DecorationRole && index.column() == 0) {
        UMLView *v = m_views.at(index.row());
        return QVariant(Icon_Utils::smallIcon(v->umlScene()->type()));
    }
    else if (role != Qt::DisplayRole)
        return QVariant();

    int cCount = columnCount(index);
    if (index.column() >= cCount)
        return QVariant();

    UMLView *v = m_views.at(index.row());
    if (index.column() == 0)
        return v->umlScene()->name();
    else if (index.column() == 1)
        return Uml::DiagramType::toStringI18n(v->umlScene()->type());
    else
        return QVariant(QString::number(v->umlScene()->widgetList().size())
                        + QLatin1String("/")
                        + QString::number(v->umlScene()->associationList().size()));
}

bool DiagramsModel::addDiagram(UMLView *view)
{
    if (m_views.contains(view))
        return false;
    int index = m_views.size();
    beginInsertRows(QModelIndex(), index, index);
    m_views.append(view);
    endInsertRows();
    return true;
}

bool DiagramsModel::removeDiagram(UMLView *view)
{
    if (!m_views.contains(view))
        return false;
    int index = m_views.indexOf(view);
    beginRemoveRows(QModelIndex(), index, index);
    m_views.removeAll(view);
    endRemoveRows();
    return true;
}

void DiagramsModel::emitDataChanged(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

void DiagramsModel::emitDataChanged(int index)
{
    QModelIndex mi = createIndex(index,0);
    emit dataChanged(mi, mi);
}

void DiagramsModel::emitDataChanged(UMLView *view)
{
   int index = m_views.indexOf(view);
   emitDataChanged(index);
}
