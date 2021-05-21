/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H

#include "multipagedialogbase.h"

class NoteWidget;

/**
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class NoteDialog : public MultiPageDialogBase
{
   Q_OBJECT
public:
    NoteDialog(QWidget *parent, NoteWidget *widget);

    bool apply();

public slots:
    void slotOk();
    void slotApply();

protected:
    void setupPages();
    NoteWidget *m_widget;
};

#endif
