/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIAGRAMSWINDOW_H
#define DIAGRAMSWINDOW_H

#include <QDockWidget>
#include <QModelIndex>

class QTableView;

class DiagramsWindow : public QDockWidget
{
    Q_OBJECT
public:
    explicit DiagramsWindow(const QString &title, QWidget *parent = 0);
    ~DiagramsWindow();

    Q_SLOT void modified();

protected:
    Q_SLOT void slotDiagramsDoubleClicked(QModelIndex index);
    Q_SLOT void slotDiagramsClicked(QModelIndex index);

protected:
    QTableView *m_diagramsTree;
};

#endif // DIAGRAMSWINDOW_H
