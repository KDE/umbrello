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
    explicit DiagramsWindow(const QString &title, QWidget *parent = nullptr);
    ~DiagramsWindow();

Q_SIGNALS:

public Q_SLOTS:
    void modified();

protected Q_SLOTS:
    void slotDiagramsDoubleClicked(QModelIndex index);
    void slotDiagramsClicked(QModelIndex index);

protected:
    QTableView *m_diagramsTree;
};

#endif // DIAGRAMSWINDOW_H
