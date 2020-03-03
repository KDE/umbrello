/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

protected:
    QLabel *m_label;
    QLabel *m_postLabel;
    KComboBox *m_editField;
};

#endif // COMBOBOXWIDGETBASE_H
