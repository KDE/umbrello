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
#include "ui_umlstereotypewidget.h"

class UMLObject;
class UMLStereotype;

class UMLStereotypeWidget : public QWidget
{
public:
    explicit UMLStereotypeWidget(QWidget *parent=0);
    ~UMLStereotypeWidget();

    void setUMLObject(UMLObject *o);
    void setEditable(bool state);
    void addToLayout(QGridLayout *layout, int row);
    void apply();

private:
    Ui::UMLStereotypeWidget *ui;

protected:
    UMLObject *m_object;
    void insertItems(UMLStereotype *type);
};

#endif // UMLSTEREOTYPEWIDGET_H
