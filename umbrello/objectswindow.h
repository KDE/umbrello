/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ObjectsWindow_H
#define ObjectsWindow_H

#include <QDockWidget>
#include <QModelIndex>

class QSortFilterProxyModel;
class QTableView;

class ObjectsWindow : public QDockWidget
{
    Q_OBJECT
public:
    explicit ObjectsWindow(const QString &title, QWidget *parent = 0);
    ~ObjectsWindow();

signals:

public slots:
    void modified();

protected slots:
    void slotObjectsDoubleClicked(QModelIndex index);
    void slotObjectsClicked(QModelIndex index);

protected:
    QTableView *m_objectsTree;
    QSortFilterProxyModel *m_proxyModel;
};

#endif // ObjectsWindow_H
