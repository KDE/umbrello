/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SELECTLAYOUTTYPEWIDGET_H
#define SELECTLAYOUTTYPEWIDGET_H

#include "basictypes.h"

#include "comboboxwidgetbase.h"

class SelectLayoutTypeWidget : public ComboBoxWidgetBase
{
public:
    explicit SelectLayoutTypeWidget(const QString &title, Uml::LayoutType::Enum selected, QWidget *parent = nullptr);

    void setCurrentLayout(Uml::LayoutType::Enum layout);
    Uml::LayoutType::Enum currentLayout();
};

#endif // SELECTLAYOUTTYPEWIDGET_H
