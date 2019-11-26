/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "selectdiagramdialog.h"

// app includes
#include "selectdiagramwidget.h"

// kde includes
#include <KLocalizedString>

SelectDiagramDialog::SelectDiagramDialog(QWidget *parent, Uml::DiagramType::Enum type, const QString &currentName, const QString excludeName)
  : SinglePageDialogBase(parent)
{
    setCaption(i18n("Select diagram"));
    m_widget = new SelectDiagramWidget(i18n("Diagram"), this);
    m_widget->setupWidget(type, currentName, excludeName, false);
    setMainWidget(m_widget);
}

SelectDiagramDialog::~SelectDiagramDialog()
{
}

Uml::ID::Type SelectDiagramDialog::currentID()
{
    return m_widget->currentID();
}

