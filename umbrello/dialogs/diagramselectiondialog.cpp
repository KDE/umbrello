/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "diagramselectiondialog.h"

#include "diagramprintpage.h"

DiagramSelectionDialog::DiagramSelectionDialog(DiagramPrintPage *page, QWidget *parent) :
    SinglePageDialogBase(parent)
{
    setMainWidget(page);
}

DiagramSelectionDialog::~DiagramSelectionDialog()
{
    // keep settings
    setMainWidget(0);
}
