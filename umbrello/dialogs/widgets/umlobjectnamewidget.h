/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOBJECTNAMEWIDGET_H
#define UMLOBJECTNAMEWIDGET_H

#include <QWidget>

class QLineEdit;

class QGridLayout;
class QLabel;

class UMLObjectNameWidget : public QWidget
{
public:
    UMLObjectNameWidget(const QString &label, const QString &text, QWidget *parent=0);
    ~UMLObjectNameWidget();

    void addToLayout(QGridLayout *layout, int row);
    QString text();
    void reset();

protected:
    QLabel *m_label;
    QLineEdit *m_editField;
    QString m_text;
};

#endif // UMLOBJECTNAMEWIDGET_H
