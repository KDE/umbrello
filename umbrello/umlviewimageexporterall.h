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

    void exportAllViews();

private:

    /**
     * The dialog to get the needed parameters from the user.
     */
    ExportAllViewsDialog* m_dialog;

};

#endif
