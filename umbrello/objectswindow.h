/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ObjectsWindow_H
#define ObjectsWindow_H

#include <QDockWidget>
#include <QModelIndex>

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
};

#endif // ObjectsWindow_H
