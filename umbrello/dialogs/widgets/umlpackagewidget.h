/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
    explicit UMLPackageWidget(UMLObject *o, QWidget *parent = 0);
    ~UMLPackageWidget();

    void addToLayout(QGridLayout *layout, int row);
    void apply();

protected:
    QLabel *m_label;
    KComboBox *m_editField;
    UMLObject *m_object;
};

#endif // UMLPACKAGEWIDGET_H
