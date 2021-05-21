/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
