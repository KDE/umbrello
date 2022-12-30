/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

Q_DECLARE_METATYPE(UMLObject*);

ObjectsWindow::ObjectsWindow(const QString &title, QWidget *parent)
  : QDockWidget(title, parent)
{
    setObjectName(QStringLiteral("ObjectsWindow"));

    m_proxyModel = new QSortFilterProxyModel;
    m_proxyModel->setSourceModel(UMLApp::app()->document()->objectsModel());
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_objectsTree = new QTableView;
    m_objectsTree->setModel(m_proxyModel);
    m_objectsTree->setSortingEnabled(true);
    m_objectsTree->verticalHeader()->setDefaultSectionSize(20);
    m_objectsTree->verticalHeader()->setVisible(false);
    m_objectsTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWidget(m_objectsTree);

    connect(m_objectsTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotObjectsDoubleClicked(QModelIndex)));
    connect(m_objectsTree, SIGNAL(clicked(QModelIndex)), this, SLOT(slotObjectsClicked(QModelIndex)));
}

ObjectsWindow::~ObjectsWindow()
{
    delete m_objectsTree;
    delete m_proxyModel;
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
