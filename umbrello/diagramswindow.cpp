/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "diagramswindow.h"

// app includes
#include "models/diagramsmodel.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QHeaderView>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QtDebug>

DiagramsWindow::DiagramsWindow(const QString &title, QWidget *parent)
  : QDockWidget(title, parent)
{
    setObjectName(QStringLiteral("DiagramsWindow"));

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(UMLApp::app()->document()->diagramsModel());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_diagramsTree = new QTableView;
    m_diagramsTree->setModel(proxy);
    m_diagramsTree->setSortingEnabled(true);
    m_diagramsTree->verticalHeader()->setDefaultSectionSize(20);
    m_diagramsTree->verticalHeader()->setVisible(false);
    m_diagramsTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWidget(m_diagramsTree);

    connect(m_diagramsTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDiagramsDoubleClicked(QModelIndex)));
    connect(m_diagramsTree, SIGNAL(clicked(QModelIndex)), this, SLOT(slotDiagramsClicked(QModelIndex)));
}

DiagramsWindow::~DiagramsWindow()
{
    QAbstractItemModel *proxy = m_diagramsTree->model();
    delete m_diagramsTree;
    delete proxy;
}

void DiagramsWindow::modified()
{
    UMLView *v = dynamic_cast<UMLView*>(QObject::sender());
    if (!v)
        return;
    UMLApp::app()->document()->diagramsModel()->emitDataChanged(v);
}

void DiagramsWindow::slotDiagramsDoubleClicked(QModelIndex index)
{
    QVariant v = m_diagramsTree->model()->data(index, Qt::UserRole);
    if (v.canConvert<UMLView*>()) {
        UMLView *view = v.value<UMLView*>();
        view->showPropertiesDialog(this);
    }
}

void DiagramsWindow::slotDiagramsClicked(QModelIndex index)
{
    QVariant v = m_diagramsTree->model()->data(index, Qt::UserRole);
    if (v.canConvert<UMLView*>()) {
        UMLView *view = v.value<UMLView*>();
        UMLApp::app()->setCurrentView(view, true);
    }
}
