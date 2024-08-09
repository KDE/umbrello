/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef EXPORTALLVIEWSDIALOG_H
#define EXPORTALLVIEWSDIALOG_H

// app includes
#include "ui_exportallviewsdialogbase.h"
#include "imagetypewidget.h"
#include "resolutionwidget.h"
#include "singlepagedialogbase.h"

// KDE forward declarations
class KComboBox;

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

    explicit ExportAllViewsDialog(QWidget  *parent = nullptr, const char *name = nullptr);

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
