/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLSTEREOTYPEWIDGET_H
#define UMLSTEREOTYPEWIDGET_H

#include <QWidget>

class UMLObject;
class UMLStereotype;

class KComboBox;

class QGridLayout;
class QLabel;

class UMLStereotypeWidget : public QWidget
{
public:
    UMLStereotypeWidget(UMLObject *object, QWidget *parent=0);
    ~UMLStereotypeWidget();

    void setEditable(bool state);
    void addToLayout(QGridLayout *layout, int row);
    void apply();

protected:
    QLabel *m_label;
    KComboBox *m_comboBox;
    UMLObject *m_object;

    void init();
    void insertItems(UMLStereotype *type);
};

#endif // UMLSTEREOTYPEWIDGET_H
