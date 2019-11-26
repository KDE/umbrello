/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SELECTDIAGRAMDIALOG_H
#define SELECTDIAGRAMDIALOG_H

//app includes
#include "singlepagedialogbase.h"
#include "basictypes.h"

class SelectDiagramWidget;

/**
 * Provides dialog to select a diagram
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class SelectDiagramDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    SelectDiagramDialog(QWidget * parent, Uml::DiagramType::Enum type, const QString &currentName, const QString excludeName);
    ~SelectDiagramDialog();

    Uml::ID::Type currentID();

protected:
    SelectDiagramWidget *m_widget;
};

#endif
