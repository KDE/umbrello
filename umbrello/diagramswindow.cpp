/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "diagramswindow.h"

// app includes
#include "models/diagramsmodel.h"
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

DiagramsWindow::DiagramsWindow(QWidget *parent)
    : QDockWidget(i18n("&Diagrams"), parent)
{
    setObjectName(QLatin1String("DiagramsWindow"));

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(UMLApp::app()->document()->diagramsModel());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_diagramsTree = new QTableView;
    m_diagramsTree->setModel(proxy);
    m_diagramsTree->setSortingEnabled(true);
    m_diagramsTree->verticalHeader()->setDefaultSectionSize(20);
    m_diagramsTree->verticalHeader()->setVisible(false);
#if QT_VERSION >= 0x050000
    m_diagramsTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    m_diagramsTree->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    setWidget(m_diagramsTree);

    connect(m_diagramsTree, &QTableView::doubleClicked, this, &DiagramsWindow::slotDiagramsDoubleClicked);
    connect(m_diagramsTree, &QTableView::clicked, this, &DiagramsWindow::slotDiagramsClicked);
}

DiagramsWindow::~DiagramsWindow()
{
    delete m_diagramsTree;
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
    QVariant v = UMLApp::app()->document()->diagramsModel()->data(index, Qt::UserRole);
    if (v.canConvert<UMLView*>()) {
        UMLView *view = v.value<UMLView*>();
        view->showPropertiesDialog(this);
    }
}

void DiagramsWindow::slotDiagramsClicked(QModelIndex index)
{
    QVariant v = UMLApp::app()->document()->diagramsModel()->data(index, Qt::UserRole);
    if (v.canConvert<UMLView*>()) {
        UMLView *view = v.value<UMLView*>();
        UMLApp::app()->setCurrentView(view, true);
    }
}
