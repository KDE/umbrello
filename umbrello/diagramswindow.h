/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2015                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIAGRAMSWINDOW_H
#define DIAGRAMSWINDOW_H

#include <QDockWidget>
#include <QModelIndex>

class QTableView;

class DiagramsWindow : public QDockWidget
{
    Q_OBJECT
public:
    explicit DiagramsWindow(QWidget *parent = 0);
    ~DiagramsWindow();

signals:

public slots:
    void modified();

protected slots:
    void slotDiagramsDoubleClicked(QModelIndex index);

protected:
    QTableView *m_diagramsTree;
};

#endif // DIAGRAMSWINDOW_H
