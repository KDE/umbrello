/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "objectswindow.h"

// app includes
#include "models/objectsmodel.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QHeaderView>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QtDebug>

ObjectsWindow::ObjectsWindow(const QString &title, QWidget *parent)
  : QDockWidget(title, parent)
{
    setObjectName(QLatin1String("ObjectsWindow"));

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(UMLApp::app()->document()->objectsModel());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_objectsTree = new QTableView;
    m_objectsTree->setModel(proxy);
    m_objectsTree->setSortingEnabled(true);
    m_objectsTree->verticalHeader()->setDefaultSectionSize(20);
    m_objectsTree->verticalHeader()->setVisible(false);
#if QT_VERSION >= 0x050000
    m_objectsTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    m_objectsTree->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    setWidget(m_objectsTree);

    connect(m_objectsTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotObjectsDoubleClicked(QModelIndex)));
    connect(m_objectsTree, SIGNAL(clicked(QModelIndex)), this, SLOT(slotObjectsClicked(QModelIndex)));
}

ObjectsWindow::~ObjectsWindow()
{
    delete m_objectsTree;
}

void ObjectsWindow::modified()
{
    UMLObject *o = dynamic_cast<UMLObject*>(QObject::sender());
    if (!o)
        return;
    UMLApp::app()->document()->objectsModel()->emitDataChanged(o);
}

void ObjectsWindow::slotObjectsDoubleClicked(QModelIndex index)
{
    QVariant v = m_objectsTree->model()->data(index, Qt::UserRole);
    if (v.canConvert<UMLObject*>()) {
        UMLObject *o = v.value<UMLObject*>();
        o->showPropertiesDialog(this);
    }
}

void ObjectsWindow::slotObjectsClicked(QModelIndex index)
{
#if 1
    Q_UNUSED(index)
#else
    QVariant v = m_objectsTree->model()->data(index, Qt::UserRole);
    if (v.canConvert<UMLObject*>()) {
        UMLObject *o = v.value<UMLObject*>();
        //o->showPropertiesDialog(this);
    }
#endif
}
