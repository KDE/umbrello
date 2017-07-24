/***************************************************************************
 *   This file is part of KDevelop PHP support                             *
 *   Copyright 2012 Miha Čančula <miha@noughmad.eu>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef TESTDOXDELEGATE_H
#define TESTDOXDELEGATE_H

#include <QtGui/QItemDelegate>
#include <KColorScheme>

class TestDoxDelegate : public QItemDelegate
{

public:
    explicit TestDoxDelegate(QObject* parent = 0);
    virtual ~TestDoxDelegate();
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    void highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold = true) const;

    KStatefulBrush failBrush;
    KStatefulBrush passBrush;
};

#endif // TESTDOXDELEGATE_H
