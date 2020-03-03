/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "selectlayouttypewidget.h"

#include <KComboBox>

SelectLayoutTypeWidget::SelectLayoutTypeWidget(const QString &title, Uml::LayoutType::Enum selected, QWidget *parent)
  : ComboBoxWidgetBase(title, QString(), parent)
{
    for (int layoutTypeNo = Uml::LayoutType::Undefined + 1; layoutTypeNo < Uml::LayoutType::N_LAYOUTTYPES; ++layoutTypeNo) {
        Uml::LayoutType::Enum lt = Uml::LayoutType::fromInt(layoutTypeNo);
        const QString type = Uml::LayoutType::toString(lt);
        m_editField->insertItem(layoutTypeNo-1, type);
        m_editField->completionObject()->addItem(type);
    }
    m_editField->setCurrentIndex(selected - 1);
}

void SelectLayoutTypeWidget::setCurrentLayout(Uml::LayoutType::Enum layout)
{
    m_editField->setCurrentIndex(layout - 1);
}

Uml::LayoutType::Enum SelectLayoutTypeWidget::currentLayout()
{
    return Uml::LayoutType::fromInt(m_editField->currentIndex()+1);
}
