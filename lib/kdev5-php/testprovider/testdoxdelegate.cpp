/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
    if (line.startsWith(QLatin1String(" [x]")))
    {
        highlight(opt, passBrush);
    }
    else if (line.startsWith(QLatin1String(" [ ]")))
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

