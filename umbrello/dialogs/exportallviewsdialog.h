/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef EXPORTALLVIEWSDIALOG_H
#define EXPORTALLVIEWSDIALOG_H

// application specific includes
#include "ui_exportallviewsdialogbase.h"

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
class ExportAllViewsDialog : public KDialog, private Ui::ExportAllViewsDialogBase
{
  Q_OBJECT

public:

    explicit ExportAllViewsDialog(QWidget* parent = 0, const char* name = 0,
                         bool modal = false, Qt::WindowFlags fl = 0,
                         const QString& defaultMimeType = "image/png");

    ~ExportAllViewsDialog();

private:

    KFileFilterCombo* m_imageType;  ///< The image type selected.

protected slots:

    friend class UMLViewImageExporterAll;

    virtual void languageChange();

};

#endif
