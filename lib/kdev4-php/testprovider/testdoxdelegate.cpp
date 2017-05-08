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

#include "testdoxdelegate.h"

TestDoxDelegate::TestDoxDelegate(QObject* parent): QItemDelegate(parent),
failBrush(KColorScheme::View, KColorScheme::NegativeText),
passBrush(KColorScheme::View, KColorScheme::PositiveText)
{

}

TestDoxDelegate::~TestDoxDelegate()
{

}

void TestDoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString line = index.data().toString();
    QStyleOptionViewItem opt = option;
    if (line.startsWith(" [x]"))
    {
        highlight(opt, passBrush);
    }
    else if (line.startsWith(" [ ]"))
    {
        highlight(opt, failBrush);
    }
    QItemDelegate::paint(painter, opt, index);
}

void TestDoxDelegate::highlight(QStyleOptionViewItem& option, const KStatefulBrush& brush, bool bold) const
{
    option.font.setBold(bold);
    option.palette.setBrush(QPalette::Text, brush.brush(option.palette));
}

