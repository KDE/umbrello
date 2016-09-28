/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2015                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "stereotypeswindow.h"

// app includes
#include "stereotype.h"
#include "models/stereotypesmodel.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QHeaderView>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QtDebug>

StereotypesWindow::StereotypesWindow(const QString &title, QWidget *parent)
  : QDockWidget(title, parent)
{
    setObjectName(QLatin1String("StereotypesWindow"));

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(UMLApp::app()->document()->stereotypesModel());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_stereotypesTree = new QTableView;
    m_stereotypesTree->setModel(proxy);
    m_stereotypesTree->setSortingEnabled(true);
    m_stereotypesTree->verticalHeader()->setDefaultSectionSize(20);
    m_stereotypesTree->verticalHeader()->setVisible(false);
    m_stereotypesTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWidget(m_stereotypesTree);

    connect(m_stereotypesTree, &QTableView::doubleClicked, this, &StereotypesWindow::slotStereotypesDoubleClicked);
}

StereotypesWindow::~StereotypesWindow()
{
    delete m_stereotypesTree;
}

void StereotypesWindow::modified()
{
    UMLStereotype *o = dynamic_cast<UMLStereotype*>(QObject::sender());
    if (!o)
        return;
    int index = UMLApp::app()->document()->stereotypes().indexOf(o);
    UMLApp::app()->document()->stereotypesModel()->emitDataChanged(index);
}

void StereotypesWindow::slotStereotypesDoubleClicked(QModelIndex index)
{
    QVariant v = m_stereotypesTree->model()->data(index, Qt::UserRole);
    if (v.canConvert<UMLStereotype*>()) {
        UMLStereotype *s = v.value<UMLStereotype*>();
        s->showPropertiesDialog(this);
    }
}
