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

#ifndef UMLVIEWIMAGEEXPORTER_H
#define UMLVIEWIMAGEEXPORTER_H

#include <qstring.h>
#include <kurl.h>

class UMLView;
class KFileDialog;

/**
 * Exports the view as an image.
 * This class takes care of asking the user the needed parameters and
 * then exports the view.
 */
class UMLViewImageExporter {
public:

    /**
     * Constructor for UMLViewImageExporter
     */
    UMLViewImageExporter(UMLView* view);

    /**
     * Destructor for UMLViewImageExporter
     */
    virtual ~UMLViewImageExporter() {
    }

    /**
     * Shows a save dialog to the user to get the needed parameters and then exports
     * the view.
     * If the selected file already exists, an overwrite confirmation
     * dialog is shown. If the user doesn't want to overwrite the file,
     * the save dialog is shown again.
     * The dialog remembers values between calls (in the same application instance,
     * although it's not persistent between Umbrello executions).
     *
     * The status bar shows an information message until the export finishes.
     *
     * If something went wrong while exporting, an error dialog is shown to the
     * user with the error message explaining the problem that happened.
     */
    void exportView();

    /**
     * Returns the URL used to save the image.
     *
     * @return The URL used to save the image.
     */
    KURL getImageURL() const {
        return m_imageURL;
    }

    /**
     * Returns the mime type used to save the image.
     *
     * @return The mime type used to save the image.
     */
    QString getImageMimeType() const {
        return m_imageMimeType;
    }

private:

    /**
     * The view to export.
     */
    UMLView* m_view;

    /**
     * The URL used to save the image.
     */
    KURL m_imageURL;

    /**
     * The mime type used to save the image.
     */
    QString m_imageMimeType;

    /**
     * Shows a save file dialog to the user to get the parameters used
     * to export the view.
     * If the selected file already exists, an overwrite confirmation
     * dialog is shown. If the user doesn't want to overwrite the file,
     * the save dialog is shown again.
     *
     * @return True if the user wants to save the image,
     *         false if the operation is cancelled.
     */
    bool prepareExportView();

    /**
     * Shows a save file dialog to the user to get the parameters used
     * to export the view and updates the attributes with the parameters got.
     *
     * @return True if the user wants to save the image,
     *         false if the operation is cancelled.
     */
    bool getParametersFromUser();

    /**
     * Prepares the save file dialog.
     * Sets the mime type filter, sensible default values...
     *
     * @param fileDialog The dialog to prepare.
     */
    void prepareFileDialog(KFileDialog &fileDialog);

};

#endif
