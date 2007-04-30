/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLVIEWIMAGEEXPORTERALL_H
#define UMLVIEWIMAGEEXPORTERALL_H

class ExportAllViewsDialog;

/**
 * Exports all the views in the UML document as images.
 * This class takes care of asking the user the needed parameters and
 * then exports the views using UMLViewImageExporterModel.
 */
class UMLViewImageExporterAll {
public:

    /**
     * Constructor for UMLViewImageExporterAll
     */
    UMLViewImageExporterAll();

    /**
     * Destructor for UMLViewImageExporterAll
     */
    virtual ~UMLViewImageExporterAll();

    /**
     * Shows a dialog to the user to get the needed parameters and then exports
     * the views.
     * The dialog remembers values between calls (in the same application instance,
     * although it's not persistent between Umbrello executions).
     *
     * Once the export begins, it can't be stopped until it ends itself. The status
     * bar shows an information message until the export finishes.
     *
     * If something went wrong while exporting, an error dialog is shown to the
     * user with the error messages explaining the problems occurred.
     */
    void exportAllViews();

private:

    /**
     * The dialog to get the needed parameters from the user.
     */
    ExportAllViewsDialog* m_dialog;

};

#endif
