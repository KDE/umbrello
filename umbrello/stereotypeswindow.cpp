/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "stereotypeswindow.h"

// app includes
#include "dialog_utils.h"
#include "debug_utils.h"
#include "stereotype.h"
#include "models/stereotypesmodel.h"
#include "dialogs/stereoattributedialog.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QHeaderView>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QContextMenuEvent>
#include <QtDebug>

DEBUG_REGISTER(StereotypesWindow)

StereotypesWindow::StereotypesWindow(const QString &title, QWidget *parent)
  : QDockWidget(title, parent)
{
    setObjectName(QStringLiteral("StereotypesWindow"));

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(UMLApp::app()->document()->stereotypesModel());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_stereotypesTree = new QTableView;
    m_stereotypesTree->setModel(proxy);
    m_stereotypesTree->setSortingEnabled(true);
    m_stereotypesTree->verticalHeader()->setDefaultSectionSize(20);
    m_stereotypesTree->verticalHeader()->setVisible(false);
#if QT_VERSION >= 0x050000
    m_stereotypesTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    m_stereotypesTree->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    setWidget(m_stereotypesTree);

    connect(m_stereotypesTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotStereotypesDoubleClicked(QModelIndex)));
}

StereotypesWindow::~StereotypesWindow()
{
    QAbstractItemModel *proxy = m_stereotypesTree->model();
    delete m_stereotypesTree;
    delete proxy;
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

void StereotypesWindow::contextMenuEvent(QContextMenuEvent *event)
{
    const QPoint& pos = event->pos();
    int row = m_stereotypesTree->rowAt(pos.y() - 60);
    // Apparently we need the "- 60" to subtract height of title lines "Stereotypes", "Name / Usage"
    logDebug3("StereotypesWindow::contextMenuEvent: pos (%1, %2) row %3",
              event->pos().x(), event->pos().y(), row);
    if (row >= 0) {
        QModelIndex index = m_stereotypesTree->model()->index(row, 0);  // first column
        // DEBUG() << "StereotypesWindow::contextMenuEvent: QModelIndex " << index;
        QVariant v = m_stereotypesTree->model()->data(index, Qt::UserRole);
        if (v.canConvert<UMLStereotype*>()) {
            UMLStereotype *s = v.value<UMLStereotype*>();
            StereoAttributeDialog *dialog = new StereoAttributeDialog(this, s);
            dialog->exec();
            delete dialog;
        } else {
            logDebug0("StereotypesWindow::contextMenuEvent: QVariant::canConvert returns false");
        }
        return;
    }
    QString name;
    if (!Dialog_Utils::askDefaultNewName(UMLObject::ot_Stereotype, name))
        return;
    if (UMLApp::app()->document()->findStereotype(name))
        return;
    UMLStereotype *s = new UMLStereotype(name);
    UMLApp::app()->document()->stereotypesModel()->addStereotype(s);
}
