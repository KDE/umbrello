/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DIAGRAMSELECTIONDIALOG_H
#define DIAGRAMSELECTIONDIALOG_H

#include "singlepagedialogbase.h"

class DiagramPrintPage;

class DiagramSelectionDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    explicit DiagramSelectionDialog(DiagramPrintPage *page, QWidget *parent = 0);
    ~DiagramSelectionDialog();
};

#endif // DIAGRAMSELECTIONDIALOG_H
