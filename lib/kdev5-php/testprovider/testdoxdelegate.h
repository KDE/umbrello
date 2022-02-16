/*
    This file is part of KDevelop PHP support
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTDOXDELEGATE_H
#define TESTDOXDELEGATE_H

#include <QItemDelegate>
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
