/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef COMBOBOXWIDGETBASE_H
#define COMBOBOXWIDGETBASE_H

#include <QWidget>

class KComboBox;
class QGridLayout;
class QLabel;

class ComboBoxWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit ComboBoxWidgetBase(const QString &title, const QString &postLabel = QString(), QWidget *parent = nullptr);
    void addToLayout(QGridLayout *layout, int row, int startColumn = 0);

    KComboBox *editField();

protected:
    QLabel *m_label;
    QLabel *m_postLabel;
    KComboBox *m_editField;
};

#endif // COMBOBOXWIDGETBASE_H
