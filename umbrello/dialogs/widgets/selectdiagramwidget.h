/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SELECTDIAGRAMWIDGET_H
#define SELECTDIAGRAMWIDGET_H

#include "basictypes.h"

#include "comboboxwidgetbase.h"

/**
 * This widget provides selecting a diagram from the list of currently available diagrams
 * @author Ralf Habacker
 */
class SelectDiagramWidget : public ComboBoxWidgetBase
{
    Q_OBJECT
public:
    explicit SelectDiagramWidget(const QString &title, QWidget *parent = nullptr);

    void setupWidget(Uml::DiagramType::Enum type, const QString &currentName, const QString &excludeName, bool withNewEntry = true);
    QString currentText();
    Uml::ID::Type currentID();
};

#endif // SELECTDIAGRAMWIDGET_H
