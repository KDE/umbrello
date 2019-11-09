/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2019                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "associationwidgetrole.h"

#include "floatingtextwidget.h"
#include "umlwidget.h"

AssociationWidgetRole::AssociationWidgetRole()
  : multiplicityWidget(nullptr)
  , changeabilityWidget(nullptr)
  , roleWidget(nullptr)
  , umlWidget(nullptr)
  , m_WidgetRegion(Uml::Region::Error)
  , m_nIndex(0)
  , m_nTotalCount(0)
  , visibility(Uml::Visibility::Public)
  , changeability(Uml::Changeability::Changeable)
{
}

void AssociationWidgetRole::setFont(const QFont &font)
{
    if (roleWidget)
        roleWidget->setFont(font);
    if (multiplicityWidget)
        multiplicityWidget->setFont(font);
    if (changeabilityWidget)
        changeabilityWidget->setFont(font);
}
