/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "objectsmodel.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "folder.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"

#include "umlobjectprivate.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QtDebug>

Q_DECLARE_METATYPE(UMLObject*);

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

    return 7;
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
    else if (section == 4)
        return QVariant(i18n("Saved"));
    else if (section == 5)
        return QVariant(i18n("Parent"));
    else if (section == 6)
        return QVariant(i18n("Pointer"));
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

    // each case needs to return
    switch (index.column()) {
    case 0:
        return o->name();
    case 1:
        return o->baseTypeStr();
    case 2:
        if (o->umlPackage())
            return o->umlPackage()->name();
        else if (o->parent()) {
            UMLObject *p = dynamic_cast<UMLObject*>(o->parent());
            if (p)
                return p->name();
        }
        return QVariant();
   case 3:
        return Uml::ID::toString(o->id());
   case 4:
        return o->m_d->isSaved;
   case 5:
        if (o->umlPackage()) {
            const UMLFolder *f = o->umlPackage()->asUMLFolder();
            if (f) {
                UMLObjectList content = f->containedObjects();
                if (content.contains(o))
                    return QLatin1String("package +");
                content = f->subordinates();
                if (content.contains(o))
                    return QLatin1String("list +");
            }
            else
                return QLatin1String("package -");
        } else if (o->umlParent()) {
            if (o->isUMLAttribute()) {
                const UMLOperation *op = o->umlParent()->asUMLOperation();
                if (op && op->getParmList().contains(o->asUMLAttribute()))
                    return QLatin1String("parent +");
                else
                    return QLatin1String("parent -");
            } else if (o->isUMLOperation()) {
                const UMLClassifier *c = o->umlParent()->asUMLClassifier();
                if (c && c->getOpList().contains(o->asUMLOperation()))
                    return QLatin1String("parent +");
                else
                    return QLatin1String("parent -");
            }
            return QLatin1String("not implemented");
        } else
            return QLatin1String("no parent");
        return QVariant();
    case 6:
        return QString::number((quintptr)o, 16);
    default:
        return QVariant();
    }
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
