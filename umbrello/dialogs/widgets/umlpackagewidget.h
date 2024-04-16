/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLPACKAGEWIDGET_H
#define UMLPACKAGEWIDGET_H

#include <QWidget>

class UMLObject;

class KComboBox;

class QGridLayout;
class QLabel;

class UMLPackageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UMLPackageWidget(UMLObject  *o, QWidget *parent = nullptr);
    ~UMLPackageWidget();

    void addToLayout(QGridLayout *layout, int row);
    void apply();

protected:
    QLabel *m_label;
    KComboBox *m_editField;
    UMLObject *m_object;
};

#endif // UMLPACKAGEWIDGET_H
