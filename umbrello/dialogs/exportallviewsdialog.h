/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
#ifndef EXPORTALLVIEWSDIALOG_H
#define EXPORTALLVIEWSDIALOG_H

// app includes
#include "ui_exportallviewsdialogbase.h"
#include "imagetypewidget.h"
#include "resolutionwidget.h"
#include "singlepagedialogbase.h"

// KDE forward declarations
#if QT_VERSION >= 0x050000
class KComboBox;
#else
class KFileFilterCombo;
#endif

/**
 * Dialog for collecting the "Export all views" params.
 * Inherits ExportAllViewsDialogBase and adds a KFileFilterCombo that uses
 * only the mime types of the supported images types.
 *
 * The KFileFilterCombo is declared here instead of in the .ui file because QT
 * Designer and uic don't recognize it.
 */
class ExportAllViewsDialog : public SinglePageDialogBase, private Ui::ExportAllViewsDialogBase
{
  Q_OBJECT

public:

    explicit ExportAllViewsDialog(QWidget* parent = 0, const char* name = 0);

    ~ExportAllViewsDialog();

private:
    ImageTypeWidget* m_imageType;  ///< The image type selected.
    ResolutionWidget* m_imageResolution;  ///< The image resolution selected.

protected slots:

    friend class UMLViewImageExporterAll;

    virtual void languageChange();
    void slotImageTypeChanged(QString imageType);
};

#endif
