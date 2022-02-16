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

signals:

public slots:
    void modified();

protected slots:
    void slotDiagramsDoubleClicked(QModelIndex index);
    void slotDiagramsClicked(QModelIndex index);

protected:
    QTableView *m_diagramsTree;
};

#endif // DIAGRAMSWINDOW_H
