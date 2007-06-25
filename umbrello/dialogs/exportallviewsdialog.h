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

#ifndef EXPORTALLVIEWSDIALOG_H
#define EXPORTALLVIEWSDIALOG_H

// application specific includes
#include "exportallviewsdialogbase.h"

// KDE forward declarations
class KFileFilterCombo;

/**
 * Dialog for collecting the "Export all views" params.
 * Inherits ExportAllViewsDialogBase and adds a KFileFilterCombo that uses
 * only the mime types of the supported images types.
 *
 * The KFileFilterCombo is declared here instead of in the .ui file because QT
 * Designer and uic don't recognize it.
 */
class ExportAllViewsDialog : public ExportAllViewsDialogBase {
  Q_OBJECT

public:

    /**
     * Constructor for UMLViewImageExporterModel.
     *
     * @param parent The parent of the dialog.
     * @param name The internal name.
     * @param modal If modal is true the dialog will block input to other the windows
     *              in the application until it's closed.
     * @param fl Window flags.
     * @param defaultMimeType The default mime type that appears in the mime types list.
     *
     * @see QDialog::QDialog
     */
    explicit ExportAllViewsDialog(QWidget* parent = 0, const char* name = 0,
                         bool modal = false, WFlags fl = 0,
                         const QString& defaultMimeType = "image/png");

    /**
     * Destructor for UMLViewImageExporterModel.
     */
    ~ExportAllViewsDialog() {
    }

    /**
     * The image type selected.
     */
    KFileFilterCombo* m_imageType;

protected slots:

    /**
     *  Sets the strings of the subwidgets using the current
     *  language.
     */
    virtual void languageChange();

};

#endif

