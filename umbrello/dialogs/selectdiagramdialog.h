/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
