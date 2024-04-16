/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014,2019 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLSTEREOTYPEWIDGET_H
#define UMLSTEREOTYPEWIDGET_H

#include "comboboxwidgetbase.h"

class UMLObject;
class UMLStereotype;

class UMLStereotypeWidget : public ComboBoxWidgetBase
{
public:
    explicit UMLStereotypeWidget(UMLObject  *object, QWidget *parent= nullptr);

    void setEditable(bool state);
    void apply();

protected:
    UMLObject *m_object;

    void insertItems(UMLStereotype *type);
};

#endif // UMLSTEREOTYPEWIDGET_H
