/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLVIEWIMAGEEXPORTERALL_H
#define UMLVIEWIMAGEEXPORTERALL_H

#include "umlviewlist.h"

class DiagramPrintPage;
class ExportAllViewsDialog;

/**
 * Exports all the views in the UML document as images.
 * This class takes care of asking the user the needed parameters and
 * then exports the views using UMLViewImageExporterModel.
 */
class UMLViewImageExporterAll
{
public:

    UMLViewImageExporterAll();
    virtual ~UMLViewImageExporterAll();

    void exportViews(DiagramPrintPage *selectPage);
    void exportViews(const UMLViewList &views);

private:

    ExportAllViewsDialog* m_dialog;  ///< Dialog to get the needed parameters from the user.

};

#endif
